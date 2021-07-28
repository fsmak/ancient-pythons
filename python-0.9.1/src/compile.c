/***********************************************************
Copyright 1991 by Stichting Mathematisch Centrum, Amsterdam, The
Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.

STICHTING MATHEMATISCH CENTRUM DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH CENTRUM BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/* Compile an expression node to intermediate code */

/* XXX TO DO:
   XXX Compute maximum needed stack sizes while compiling
   XXX Generate simple jump for break/return outside 'try...finally'
   XXX Include function name in code (and module names?)
*/

#include "allobjects.h"

#include "node.h"
#include "token.h"
#include "graminit.h"
#include "compile.h"
#include "opcode.h"
#include "structmember.h"

#include <ctype.h>

#define OFF(x) offsetof(codeobject, x)

static struct memberlist code_memberlist[] = {
       {"co_code",   T_OBJECT,       OFF(co_code)},
       {"co_consts", T_OBJECT,       OFF(co_consts)},
       {"co_names",  T_OBJECT,       OFF(co_names)},
       {"co_filename",       T_OBJECT,       OFF(co_filename)},
       {NULL}  /* Sentinel */
};

static object *
code_getattr(co, name)
       codeobject *co;
       char *name;
{
       return getmember((char *)co, code_memberlist, name);
}

static void
code_dealloc(co)
       codeobject *co;
{
       XDECREF(co->co_code);
       XDECREF(co->co_consts);
       XDECREF(co->co_names);
       XDECREF(co->co_filename);
       DEL(co);
}

typeobject Codetype = {
       OB_HEAD_INIT(&Typetype)
       0,
       "code",
       sizeof(codeobject),
       0,
       code_dealloc,   /*tp_dealloc*/
       0,              /*tp_print*/
       code_getattr,   /*tp_getattr*/
       0,              /*tp_setattr*/
       0,              /*tp_compare*/
       0,              /*tp_repr*/
       0,              /*tp_as_number*/
       0,              /*tp_as_sequence*/
       0,              /*tp_as_mapping*/
};

static codeobject *newcodeobject PROTO((object *, object *, object *, char *));

static codeobject *
newcodeobject(code, consts, names, filename)
       object *code;
       object *consts;
       object *names;
       char *filename;
{
       codeobject *co;
       int i;
       /* Check argument types */
       if (code == NULL || !is_stringobject(code) ||
               consts == NULL || !is_listobject(consts) ||
               names == NULL || !is_listobject(names)) {
               err_badcall();
               return NULL;
       }
       /* Make sure the list of names contains only strings */
       for (i = getlistsize(names); --i >= 0; ) {
               object *v = getlistitem(names, i);
               if (v == NULL || !is_stringobject(v)) {
                       err_badcall();
                       return NULL;
               }
       }
       co = NEWOBJ(codeobject, &Codetype);
       if (co != NULL) {
               INCREF(code);
               co->co_code = (stringobject *)code;
               INCREF(consts);
               co->co_consts = consts;
               INCREF(names);
               co->co_names = names;
               if ((co->co_filename = newstringobject(filename)) == NULL) {
                       DECREF(co);
                       co = NULL;
               }
       }
       return co;
}


/* Data structure used internally */
struct compiling {
       object *c_code;         /* string */
       object *c_consts;       /* list of objects */
       object *c_names;        /* list of strings (names) */
       int c_nexti;            /* index into c_code */
       int c_errors;           /* counts errors occurred */
       int c_infunction;       /* set when compiling a function */
       int c_loops;            /* counts nested loops */
       char *c_filename;       /* filename of current node */
};

/* Prototypes */
static int com_init PROTO((struct compiling *, char *));
static void com_free PROTO((struct compiling *));
static void com_done PROTO((struct compiling *));
static void com_node PROTO((struct compiling *, struct _node *));
static void com_addbyte PROTO((struct compiling *, int));
static void com_addint PROTO((struct compiling *, int));
static void com_addoparg PROTO((struct compiling *, int, int));
static void com_addfwref PROTO((struct compiling *, int, int *));
static void com_backpatch PROTO((struct compiling *, int));
static int com_add PROTO((struct compiling *, object *, object *));
static int com_addconst PROTO((struct compiling *, object *));
static int com_addname PROTO((struct compiling *, object *));
static void com_addopname PROTO((struct compiling *, int, node *));

static int
com_init(c, filename)
       struct compiling *c;
       char *filename;
{
       if ((c->c_code = newsizedstringobject((char *)NULL, 0)) == NULL)
               goto fail_3;
       if ((c->c_consts = newlistobject(0)) == NULL)
               goto fail_2;
       if ((c->c_names = newlistobject(0)) == NULL)
               goto fail_1;
       c->c_nexti = 0;
       c->c_errors = 0;
       c->c_infunction = 0;
       c->c_loops = 0;
       c->c_filename = filename;
       return 1;

  fail_1:
       DECREF(c->c_consts);
  fail_2:
       DECREF(c->c_code);
  fail_3:
       return 0;
}

static void
com_free(c)
       struct compiling *c;
{
       XDECREF(c->c_code);
       XDECREF(c->c_consts);
       XDECREF(c->c_names);
}

static void
com_done(c)
       struct compiling *c;
{
       if (c->c_code != NULL)
               resizestring(&c->c_code, c->c_nexti);
}

static void
com_addbyte(c, byte)
       struct compiling *c;
       int byte;
{
       int len;
       if (byte < 0 || byte > 255) {
               fprintf(stderr, "XXX compiling bad byte: %d\n", byte);
               abort();
               err_setstr(SystemError, "com_addbyte: byte out of range");
               c->c_errors++;
       }
       if (c->c_code == NULL)
               return;
       len = getstringsize(c->c_code);
       if (c->c_nexti >= len) {
               if (resizestring(&c->c_code, len+1000) != 0) {
                       c->c_errors++;
                       return;
               }
       }
       getstringvalue(c->c_code)[c->c_nexti++] = byte;
}

static void
com_addint(c, x)
       struct compiling *c;
       int x;
{
       com_addbyte(c, x & 0xff);
       com_addbyte(c, x >> 8); /* XXX x should be positive */
}

static void
com_addoparg(c, op, arg)
       struct compiling *c;
       int op;
       int arg;
{
       com_addbyte(c, op);
       com_addint(c, arg);
}

static void
com_addfwref(c, op, p_anchor)
       struct compiling *c;
       int op;
       int *p_anchor;
{
       /* Compile a forward reference for backpatching */
       /* p_anchor 是一个链条关系，比如 
       try:
         suit --> 这里执行完了要去执行 finally ，因此会有一个 JUMP_FORWARD FINALLY地址 
       except: 
         suit --> 这里执行完了要去执行 finally ，因此会有一个 JUMP_FORWARD FINALLY地址 
       finally：
         suit --> 这里执行完了要去执行 finally ，因此会有一个 JUMP_FORWARD FINALLY地址 
       可以看到有若干个block代码执行完都需要jump到同一个地方，因此这里需要一个链条的关系把回补同一个地址的anchor偏移串起来
       第一个回填地址初始值为0，第二个同样的回填地址则是当前here-第一个的差（即为距离第一个回填地址的偏移here - anchor），
       如此类推，对于要回填同一个地址的回填地址初始化的值都为对上一个回填地址的偏移，这样回填的时候就可以一个个串起来，一直
       找到第一个，如何判断为第一个？那就是初始化值为0
       [#0 初始值=0]......[#1 初始值=距离#0的偏移]......[#2 初始值=距离#1的偏移]
            
       */
       int here;
       int anchor;
       com_addbyte(c, op);
       here = c->c_nexti;
       anchor = *p_anchor;
       *p_anchor = here;
       com_addint(c, anchor == 0 ? 0 : here - anchor);
}

static void
com_backpatch(c, anchor)
       struct compiling *c;
       int anchor; /* Must be nonzero */
{
       unsigned char *code = (unsigned char *) getstringvalue(c->c_code);
       int target = c->c_nexti;
       int lastanchor = 0;
       int dist;
       int prev;
       /*
       这个循环是对同一条链填充同一个回填地址，prev是上一个回填地址的初始值，
       [#0 初始值=0]......[#1 初始值=距离#0的偏移]......[#2 初始值=距离#1的偏移]
       比如填充#2的时候，先提取出#2记录的初始值，这个初始值就是#1的位置，有了上一个位置之后，
       就填充#2后
       [#0 初始值=0]......[#1 初始值=距离#0的偏移]......[#2 dist]
       接着判断prev是否为0？如果是则代表没有上一个回填地址了，跳出循环，通常while里面没有break语句等就一次回填即可
       假如prev不为0，则这个prev就是上一个回填偏移（#1），所以 anchor -= prev
       这个时候，还是先提取#1的初始值到prev，然后填充#1
       [#0 初始值=0]......[#1 dist]......[#2 dist]
       继续判断prev是否为0？ 如此类推一直到 prev初始值为0，则是找到链头了，没有父的anchor了，退出循环
       */
       for (;;) {
               /* Make the JUMP instruction at anchor point to target */
               prev = code[anchor] + (code[anchor+1] << 8);
               dist = target - (anchor+2);
               code[anchor] = dist & 0xff;
               code[anchor+1] = dist >> 8;
               if (!prev)
                       break;
               lastanchor = anchor;
               anchor -= prev;
       }
}

/* Handle constants and names uniformly */

static int
com_add(c, list, v)
       struct compiling *c;
       object *list;
       object *v;
{
       int n = getlistsize(list);
       int i;
       for (i = n; --i >= 0; ) {
               object *w = getlistitem(list, i);
               if (cmpobject(v, w) == 0)
                       return i;
       }
       if (addlistitem(list, v) != 0)
               c->c_errors++;
       return n;
}

static int
com_addconst(c, v)
       struct compiling *c;
       object *v;
{
       return com_add(c, c->c_consts, v);
}

static int
com_addname(c, v)
       struct compiling *c;
       object *v;
{
       return com_add(c, c->c_names, v);
}

static void
com_addopname(c, op, n)
       struct compiling *c;
       int op;
       node *n;
{
       object *v;
       int i;
       char *name;
       if (TYPE(n) == STAR)
               name = "*";
       else {
               REQ(n, NAME);
               name = STR(n);
       }
       if ((v = newstringobject(name)) == NULL) {
               c->c_errors++;
               i = 255;
       }
       else {
               i = com_addname(c, v);
               DECREF(v);
       }
       com_addoparg(c, op, i);
}

static object *
parsenumber(s)
       char *s;
{
       extern long strtol();
       extern double atof();
       char *end = s;
       long x;
       x = strtol(s, &end, 0);
       if (*end == '\0')
               return newintobject(x);
       if (*end == '.' || *end == 'e' || *end == 'E')
               return newfloatobject(atof(s));
       err_setstr(RuntimeError, "bad number syntax");
       return NULL;
}

static object *
parsestr(s)
       char *s;
{
       object *v;
       int len;
       char *buf;
       char *p;
       int c;
       if (*s != '\'') {
               err_badcall();
               return NULL;
       }
       s++;
       len = strlen(s);
       if (s[--len] != '\'') {
               err_badcall();
               return NULL;
       }
       if (strchr(s, '\\') == NULL)
               return newsizedstringobject(s, len);
       v = newsizedstringobject((char *)NULL, len);
       p = buf = getstringvalue(v);
       while (*s != '\0' && *s != '\'') {
               if (*s != '\\') {
                       *p++ = *s++;
                       continue;
               }
               s++;
               switch (*s++) {
               /* XXX This assumes ASCII! */
               case '\\': *p++ = '\\'; break;
               case '\'': *p++ = '\''; break;
               case 'b': *p++ = '\b'; break;
               case 'f': *p++ = '\014'; break; /* FF */
               case 't': *p++ = '\t'; break;
               case 'n': *p++ = '\n'; break;
               case 'r': *p++ = '\r'; break;
               case 'v': *p++ = '\013'; break; /* VT */
               case 'E': *p++ = '\033'; break; /* ESC, not C */
               case 'a': *p++ = '\007'; break; /* BEL, not classic C */
               case '0': case '1': case '2': case '3':
               case '4': case '5': case '6': case '7':
                       c = s[-1] - '0';
                       if ('0' <= *s && *s <= '7') {
                               c = (c<<3) + *s++ - '0';
                               if ('0' <= *s && *s <= '7')
                                       c = (c<<3) + *s++ - '0';
                       }
                       *p++ = c;
                       break;
               case 'x':
                       if (isxdigit(*s)) {
                               sscanf(s, "%x", &c);
                               *p++ = c;
                               do {
                                       s++;
                               } while (isxdigit(*s));
                               break;
                       }
               /* FALLTHROUGH */
               default: *p++ = '\\'; *p++ = s[-1]; break;
               }
       }
       resizestring(&v, (int)(p - buf));
       return v;
}

static void
com_list_constructor(c, n)
       struct compiling *c;
       node *n;
{
       int len;
       int i;
       object *v, *w;
       if (TYPE(n) != testlist)
               REQ(n, exprlist);
       /* exprlist: expr (',' expr)* [',']; likewise for testlist */
       len = (NCH(n) + 1) / 2;
       for (i = 0; i < NCH(n); i += 2)
               com_node(c, CHILD(n, i));
       com_addoparg(c, BUILD_LIST, len);
}

static void
com_atom(c, n)
       struct compiling *c;
       node *n;
{
       node *ch;
       object *v;
       int i;
       REQ(n, atom);
       ch = CHILD(n, 0);
       switch (TYPE(ch)) {
       case LPAR:
               if (TYPE(CHILD(n, 1)) == RPAR)
                       com_addoparg(c, BUILD_TUPLE, 0);
               else
                       com_node(c, CHILD(n, 1));
               break;
       case LSQB:
               if (TYPE(CHILD(n, 1)) == RSQB)
                       com_addoparg(c, BUILD_LIST, 0);
               else
                       com_list_constructor(c, CHILD(n, 1));
               break;
       case LBRACE:
               com_addoparg(c, BUILD_MAP, 0);
               break;
       case BACKQUOTE:
               com_node(c, CHILD(n, 1));
               com_addbyte(c, UNARY_CONVERT);
               break;
       case NUMBER:
               if ((v = parsenumber(STR(ch))) == NULL) {
                       c->c_errors++;
                       i = 255;
               }
               else {
                       i = com_addconst(c, v);
                       DECREF(v);
               }
               com_addoparg(c, LOAD_CONST, i);
               break;
       case STRING:
               if ((v = parsestr(STR(ch))) == NULL) {
                       c->c_errors++;
                       i = 255;
               }
               else {
                       i = com_addconst(c, v);
                       DECREF(v);
               }
               com_addoparg(c, LOAD_CONST, i);
               break;
       case NAME:
               com_addopname(c, LOAD_NAME, ch);
               break;
       default:
               fprintf(stderr, "node type %d\n", TYPE(ch));
               err_setstr(SystemError, "com_atom: unexpected node type");
               c->c_errors++;
       }
}

static void
com_slice(c, n, op)
       struct compiling *c;
       node *n;
       int op;
{
       if (NCH(n) == 1) {
               com_addbyte(c, op);
       }
       else if (NCH(n) == 2) {
               if (TYPE(CHILD(n, 0)) != COLON) {
                       com_node(c, CHILD(n, 0));
                       com_addbyte(c, op+1);
               }
               else {
                       com_node(c, CHILD(n, 1));
                       com_addbyte(c, op+2);
               }
       }
       else {
               com_node(c, CHILD(n, 0));
               com_node(c, CHILD(n, 2));
               com_addbyte(c, op+3);
       }
}

static void
com_apply_subscript(c, n)
       struct compiling *c;
       node *n;
{
       REQ(n, subscript);
       if (NCH(n) == 1 && TYPE(CHILD(n, 0)) != COLON) {
               /* It's a single subscript */
               com_node(c, CHILD(n, 0));
               com_addbyte(c, BINARY_SUBSCR);
       }
       else {
               /* It's a slice: [expr] ':' [expr] */
               com_slice(c, n, SLICE);
       }
}

static void
com_call_function(c, n)
       struct compiling *c;
       node *n; /* EITHER testlist OR ')' */
{
       if (TYPE(n) == RPAR) {
               //无参数调用
               com_addbyte(c, UNARY_CALL);
       }
       else {
               //有参数调用
               com_node(c, n);
               com_addbyte(c, BINARY_CALL);
       }
}

static void
com_select_member(c, n)
       struct compiling *c;
       node *n;
{
       com_addopname(c, LOAD_ATTR, n);
}

static void
com_apply_trailer(c, n)
       struct compiling *c;
       node *n;
{
       REQ(n, trailer);
       switch (TYPE(CHILD(n, 0))) {
       case LPAR:
               com_call_function(c, CHILD(n, 1));
               break;
       case DOT:
               com_select_member(c, CHILD(n, 1));
               break;
       case LSQB:
               com_apply_subscript(c, CHILD(n, 1));
               break;
       default:
               err_setstr(SystemError,
                       "com_apply_trailer: unknown trailer type");
               c->c_errors++;
       }
}

static void
com_factor(c, n)
       struct compiling *c;
       node *n;
{
       int i;
       REQ(n, factor);
       if (TYPE(CHILD(n, 0)) == PLUS) {
               com_factor(c, CHILD(n, 1));
               com_addbyte(c, UNARY_POSITIVE);
       }
       else if (TYPE(CHILD(n, 0)) == MINUS) {
               com_factor(c, CHILD(n, 1));
               com_addbyte(c, UNARY_NEGATIVE);
       }
       else {
               com_atom(c, CHILD(n, 0));
               for (i = 1; i < NCH(n); i++)
                       com_apply_trailer(c, CHILD(n, i));
       }
}

static void
com_term(c, n)
       struct compiling *c;
       node *n;
{
       int i;
       int op;
       REQ(n, term);
       com_factor(c, CHILD(n, 0));
       for (i = 2; i < NCH(n); i += 2) {
               com_factor(c, CHILD(n, i));
               switch (TYPE(CHILD(n, i-1))) {
               case STAR:
                       op = BINARY_MULTIPLY;
                       break;
               case SLASH:
                       op = BINARY_DIVIDE;
                       break;
               case PERCENT:
                       op = BINARY_MODULO;
                       break;
               default:
                       err_setstr(SystemError,
                               "com_term: term operator not *, / or %");
                       c->c_errors++;
                       op = 255;
               }
               com_addbyte(c, op);
       }
}

static void
com_expr(c, n)
       struct compiling *c;
       node *n;
{
       int i;
       int op;
       REQ(n, expr);
       com_term(c, CHILD(n, 0));
       for (i = 2; i < NCH(n); i += 2) {
               com_term(c, CHILD(n, i));
               switch (TYPE(CHILD(n, i-1))) {
               case PLUS:
                       op = BINARY_ADD;
                       break;
               case MINUS:
                       op = BINARY_SUBTRACT;
                       break;
               default:
                       err_setstr(SystemError,
                               "com_expr: expr operator not + or -");
                       c->c_errors++;
                       op = 255;
               }
               com_addbyte(c, op);
       }
}

static enum cmp_op
cmp_type(n)
       node *n;
{
       REQ(n, comp_op);
       /* comp_op: '<' | '>' | '=' | '>' '=' | '<' '=' | '<' '>'
                 | 'in' | 'not' 'in' | 'is' | 'is' not' */
       if (NCH(n) == 1) {
               n = CHILD(n, 0);
               switch (TYPE(n)) {
               case LESS:      return LT;
               case GREATER:   return GT;
               case EQUAL:     return EQ;
               case NAME:      if (strcmp(STR(n), "in") == 0) return IN;
                               if (strcmp(STR(n), "is") == 0) return IS;
               }
       }
       else if (NCH(n) == 2) {
               int t2 = TYPE(CHILD(n, 1));
               switch (TYPE(CHILD(n, 0))) {
               case LESS:      if (t2 == EQUAL)        return LE;
                               if (t2 == GREATER)      return NE;
                               break;
               case GREATER:   if (t2 == EQUAL)        return GE;
                               break;
               case NAME:      if (strcmp(STR(CHILD(n, 1)), "in") == 0)
                                       return NOT_IN;
                               if (strcmp(STR(CHILD(n, 0)), "is") == 0)
                                       return IS_NOT;
               }
       }
       return BAD;
}

static void
com_comparison(c, n)
       struct compiling *c;
       node *n;
{
       int i;
       enum cmp_op op;
       int anchor;
       REQ(n, comparison); /* comparison: expr (comp_op expr)* */
       com_expr(c, CHILD(n, 0));
       if (NCH(n) == 1)
               return;

       /****************************************************************
          The following code is generated for all but the last
          comparison in a chain:

          label:       on stack:       opcode:         jump to:

                       a               <code to load b>
                       a, b            DUP_TOP
                       a, b, b         ROT_THREE
                       b, a, b         COMPARE_OP
                       b, 0-or-1       JUMP_IF_FALSE   L1
                       b, 1            POP_TOP
                       b

          We are now ready to repeat this sequence for the next
          comparison in the chain.

          For the last we generate:

                       b               <code to load c>
                       b, c            COMPARE_OP
                       0-or-1

          If there were any jumps to L1 (i.e., there was more than one
          comparison), we generate:

                       0-or-1          JUMP_FORWARD    L2
          L1:          b, 0            ROT_TWO
                       0, b            POP_TOP
                       0
          L2:
       ****************************************************************/

       anchor = 0;

       for (i = 2; i < NCH(n); i += 2) {
               com_expr(c, CHILD(n, i));
               if (i+2 < NCH(n)) {
                       com_addbyte(c, DUP_TOP);
                       com_addbyte(c, ROT_THREE);
               }
               op = cmp_type(CHILD(n, i-1));
               if (op == BAD) {
                       err_setstr(SystemError,
                               "com_comparison: unknown comparison op");
                       c->c_errors++;
               }
               com_addoparg(c, COMPARE_OP, op);
               if (i+2 < NCH(n)) {
                       com_addfwref(c, JUMP_IF_FALSE, &anchor);
                       com_addbyte(c, POP_TOP);
               }
       }

       if (anchor) {
               int anchor2 = 0;
               com_addfwref(c, JUMP_FORWARD, &anchor2);
               com_backpatch(c, anchor);
               com_addbyte(c, ROT_TWO);
               com_addbyte(c, POP_TOP);
               com_backpatch(c, anchor2);
       }
}

static void
com_not_test(c, n)
       struct compiling *c;
       node *n;
{
       REQ(n, not_test); /* 'not' not_test | comparison */
       if (NCH(n) == 1) {
               com_comparison(c, CHILD(n, 0));
       }
       else {
               com_not_test(c, CHILD(n, 1));
               com_addbyte(c, UNARY_NOT);
       }
}

static void
com_and_test(c, n)
       struct compiling *c;
       node *n;
{
       int i;
       int anchor;
       REQ(n, and_test); /* not_test ('and' not_test)* */
       anchor = 0;
       i = 0;
       for (;;) {
               com_not_test(c, CHILD(n, i));
               if ((i += 2) >= NCH(n))
                       break;
               com_addfwref(c, JUMP_IF_FALSE, &anchor);
               com_addbyte(c, POP_TOP);
       }
       if (anchor)
               com_backpatch(c, anchor);
}

static void
com_test(c, n)
       struct compiling *c;
       node *n;
{
       int i;
       int anchor;
       REQ(n, test); /* and_test ('and' and_test)* */
       anchor = 0;
       i = 0;
       for (;;) {
               com_and_test(c, CHILD(n, i));
               if ((i += 2) >= NCH(n))
                       break;
               com_addfwref(c, JUMP_IF_TRUE, &anchor);
               com_addbyte(c, POP_TOP);
       }
       if (anchor)
               com_backpatch(c, anchor);
}

static void
com_list(c, n)
       struct compiling *c;
       node *n;
{
       /* exprlist: expr (',' expr)* [',']; likewise for testlist */
       if (NCH(n) == 1) {
               com_node(c, CHILD(n, 0));
       }
       else {
               int i;
               int len;
               len = (NCH(n) + 1) / 2;
               for (i = 0; i < NCH(n); i += 2)
                       com_node(c, CHILD(n, i));
               com_addoparg(c, BUILD_TUPLE, len);
       }
}


/* Begin of assignment compilation */

static void com_assign_name PROTO((struct compiling *, node *, int));
static void com_assign PROTO((struct compiling *, node *, int));

static void
com_assign_attr(c, n, assigning)
       struct compiling *c;
       node *n;
       int assigning;
{
       com_addopname(c, assigning ? STORE_ATTR : DELETE_ATTR, n);
}

static void
com_assign_slice(c, n, assigning)
       struct compiling *c;
       node *n;
       int assigning;
{
       com_slice(c, n, assigning ? STORE_SLICE : DELETE_SLICE);
}

static void
com_assign_subscript(c, n, assigning)
       struct compiling *c;
       node *n;
       int assigning;
{
       com_node(c, n);
       com_addbyte(c, assigning ? STORE_SUBSCR : DELETE_SUBSCR);
}

static void
com_assign_trailer(c, n, assigning)
       struct compiling *c;
       node *n;
       int assigning;
{
       char *name;
       REQ(n, trailer);
       switch (TYPE(CHILD(n, 0))) {
       case LPAR: /* '(' [exprlist] ')' */
               err_setstr(TypeError, "can't assign to function call");
               c->c_errors++;
               break;
       case DOT: /* '.' NAME */
               com_assign_attr(c, CHILD(n, 1), assigning);
               break;
       case LSQB: /* '[' subscript ']' */
               n = CHILD(n, 1);
               REQ(n, subscript); /* subscript: expr | [expr] ':' [expr] */
               if (NCH(n) > 1 || TYPE(CHILD(n, 0)) == COLON)
                       com_assign_slice(c, n, assigning);
               else
                       com_assign_subscript(c, CHILD(n, 0), assigning);
               break;
       default:
               err_setstr(TypeError, "unknown trailer type");
               c->c_errors++;
       }
}

static void
com_assign_tuple(c, n, assigning)
       struct compiling *c;
       node *n;
       int assigning;
{
       int i;
       if (TYPE(n) != testlist)
               REQ(n, exprlist);
       if (assigning)
               com_addoparg(c, UNPACK_TUPLE, (NCH(n)+1)/2);
       for (i = 0; i < NCH(n); i += 2)
               com_assign(c, CHILD(n, i), assigning);
}

static void
com_assign_list(c, n, assigning)
       struct compiling *c;
       node *n;
       int assigning;
{
       int i;
       if (assigning)
               com_addoparg(c, UNPACK_LIST, (NCH(n)+1)/2);
       for (i = 0; i < NCH(n); i += 2)
               com_assign(c, CHILD(n, i), assigning);
}

static void
com_assign_name(c, n, assigning)
       struct compiling *c;
       node *n;
       int assigning;
{
       REQ(n, NAME);
       com_addopname(c, assigning ? STORE_NAME : DELETE_NAME, n);
}

static void
com_assign(c, n, assigning)
       struct compiling *c;
       node *n;
       int assigning;
{
       /* Loop to avoid trivial recursion */
       for (;;) {
               switch (TYPE(n)) {

               case exprlist:
               case testlist:
                       if (NCH(n) > 1) {
                               com_assign_tuple(c, n, assigning);
                               return;
                       }
                       n = CHILD(n, 0);
                       break;

               case test:
               case and_test:
               case not_test:
                       if (NCH(n) > 1) {
                               err_setstr(TypeError,
                                       "can't assign to operator");
                               c->c_errors++;
                               return;
                       }
                       n = CHILD(n, 0);
                       break;

               case comparison:
                       if (NCH(n) > 1) {
                               err_setstr(TypeError,
                                       "can't assign to operator");
                               c->c_errors++;
                               return;
                       }
                       n = CHILD(n, 0);
                       break;

               case expr:
                       if (NCH(n) > 1) {
                               err_setstr(TypeError,
                                       "can't assign to operator");
                               c->c_errors++;
                               return;
                       }
                       n = CHILD(n, 0);
                       break;

               case term:
                       if (NCH(n) > 1) {
                               err_setstr(TypeError,
                                       "can't assign to operator");
                               c->c_errors++;
                               return;
                       }
                       n = CHILD(n, 0);
                       break;

               case factor: /* ('+'|'-') factor | atom trailer* */
                       if (TYPE(CHILD(n, 0)) != atom) { /* '+' | '-' */
                               err_setstr(TypeError,
                                       "can't assign to operator");
                               c->c_errors++;
                               return;
                       }
                       if (NCH(n) > 1) { /* trailer present */
                               int i;
                               com_node(c, CHILD(n, 0));
                               for (i = 1; i+1 < NCH(n); i++) {
                                       com_apply_trailer(c, CHILD(n, i));
                               } /* NB i is still alive */
                               com_assign_trailer(c,
                                               CHILD(n, i), assigning);
                               return;
                       }
                       n = CHILD(n, 0);
                       break;

               case atom:
                       switch (TYPE(CHILD(n, 0))) {
                       case LPAR:
                               n = CHILD(n, 1);
                               if (TYPE(n) == RPAR) {
                                       /* XXX Should allow () = () ??? */
                                       err_setstr(TypeError,
                                               "can't assign to ()");
                                       c->c_errors++;
                                       return;
                               }
                               break;
                       case LSQB:
                               n = CHILD(n, 1);
                               if (TYPE(n) == RSQB) {
                                       err_setstr(TypeError,
                                               "can't assign to []");
                                       c->c_errors++;
                                       return;
                               }
                               com_assign_list(c, n, assigning);
                               return;
                       case NAME:
                               com_assign_name(c, CHILD(n, 0), assigning);
                               return;
                       default:
                               err_setstr(TypeError,
                                               "can't assign to constant");
                               c->c_errors++;
                               return;
                       }
                       break;

               default:
                       fprintf(stderr, "node type %d\n", TYPE(n));
                       err_setstr(SystemError, "com_assign: bad node");
                       c->c_errors++;
                       return;

               }
       }
}

static void
com_expr_stmt(c, n)
       struct compiling *c;
       node *n;
{
       REQ(n, expr_stmt); /* exprlist ('=' exprlist)* NEWLINE */
       /*
       // x = 1 为例
       node *n0 = CHILD(n, 0);// exprlist (a)
       node *n1 = CHILD(n, 1);// '='
       node *n2 = CHILD(n, 2);// exprlist (1)
       node *n3 = CHILD(n, 3);// NEWLINE
       */
       //先编译 = 后面的 exrlist，然后把值push到栈，
       //这里最终会产生一个 LOAD_CONST 0 的指令，把初始化值0 push入栈
       com_node(c, CHILD(n, NCH(n)-2));
       if (NCH(n) == 2) {
               com_addbyte(c, PRINT_EXPR);
       }
       else {
               int i;
               for (i = 0; i < NCH(n)-3; i+=2) {
                       if (i+2 < NCH(n)-3) {
                               // x=y=1 的情况，DUP_TOP 是两个TOP，复制一份
                               com_addbyte(c, DUP_TOP);
                       }
                       //再编译 = 前面的 exprlist，这个时候栈里面已经有了 = 后面exrlist的值了，
                       //再赋予给 = 前面的 exrlist
                       //这里最终会产生一个 STORE_NAME a 的指令，把已经入栈的 0 赋值到 a 变量
                       com_assign(c, CHILD(n, i), 1/*assign*/);
               }
       }
}

static void
com_print_stmt(c, n)
       struct compiling *c;
       node *n;
{
       int i;
       REQ(n, print_stmt); /* 'print' (test ',')* [test] NEWLINE */
       for (i = 1; i+1 < NCH(n); i += 2) {
               com_node(c, CHILD(n, i));
               com_addbyte(c, PRINT_ITEM);
       }
       if (TYPE(CHILD(n, NCH(n)-2)) != COMMA)
               com_addbyte(c, PRINT_NEWLINE);
               /* XXX Alternatively, LOAD_CONST '\n' and then PRINT_ITEM */
}

static void
com_return_stmt(c, n)
       struct compiling *c;
       node *n;
{
       REQ(n, return_stmt); /* 'return' [testlist] NEWLINE */
       if (!c->c_infunction) {
               err_setstr(TypeError, "'return' outside function");
               c->c_errors++;
       }
       if (NCH(n) == 2)
               com_addoparg(c, LOAD_CONST, com_addconst(c, None));
       else
               com_node(c, CHILD(n, 1));
       com_addbyte(c, RETURN_VALUE);
}

static void
com_raise_stmt(c, n)
       struct compiling *c;
       node *n;
{
       REQ(n, raise_stmt); /* 'raise' expr [',' expr] NEWLINE */
       com_node(c, CHILD(n, 1));
       if (NCH(n) > 3)
               com_node(c, CHILD(n, 3));
       else
               com_addoparg(c, LOAD_CONST, com_addconst(c, None));
       com_addbyte(c, RAISE_EXCEPTION);
}

static void
com_import_stmt(c, n)
       struct compiling *c;
       node *n;
{
       int i;
       REQ(n, import_stmt);
       /* 'import' NAME (',' NAME)* NEWLINE |
          'from' NAME 'import' ('*' | NAME (',' NAME)*) NEWLINE */
       if (STR(CHILD(n, 0))[0] == 'f') {
               /* 'from' NAME 'import' ... */
               REQ(CHILD(n, 1), NAME);
               com_addopname(c, IMPORT_NAME, CHILD(n, 1));
               for (i = 3; i < NCH(n); i += 2)
                       com_addopname(c, IMPORT_FROM, CHILD(n, i));
               com_addbyte(c, POP_TOP);
       }
       else {
               /* 'import' ... */
               for (i = 1; i < NCH(n); i += 2) {
                       com_addopname(c, IMPORT_NAME, CHILD(n, i));
                       com_addopname(c, STORE_NAME, CHILD(n, i));
               }
       }
}

static void
com_if_stmt(c, n)
       struct compiling *c;
       node *n;
{
       /*
       对于for、while、try-catch-finally等语句块都有对应的 setup block，但是为什么 if 没有呢？
       在if-elif-else的suite里面，创建的临时变量不需要清理掉吗？写了以下一段python测试代码
       ```
         if 1:
           a = 1
         print a
       ```
       程序可以运行起来，外面的print a输出1，跟程序的逻辑一致，因为对于if语句来讲，语句里面创建的
       不是临时变量，而是在当前frame的变量，就算离开if语句了一样生效，不知道为什么python要这样设计
       如果测试代码是下面
       ```
         if 0:
           a = 1
         print a
       ```
       因为python是动态语言，因为没有执行if里面的逻辑，所以变量a没有被创建，所以外层print a是找不到
       变量a的。在python编译代码里面要时刻记住一个东西，就是实例是执行是通过各种对应的构造指令生成的
       而不是说编译的时候知道有变量a了就做初始化，哪怕if 0里面的赋值语句没执行，这点很关键。
       */
       int i;
       //anchor记录的是所有if语句里面所有会跳转到整个if语句结束位置的回填地址链
       int anchor = 0;
       REQ(n, if_stmt);
       /*'if' test ':' suite ('elif' test ':' suite)* ['else' ':' suite] */
       for (i = 0; i+3 < NCH(n); i+=4) {
               /*
               这个循环每次分析的是4个节点，刚好对应 if test ':' suite ，所以这个循环
               就是编译 if elif 的
               */
               // a 记录的是当前 if/elif 块的结束位置，就是说这个 if/elif 不满足了就跳去接着的下一个 elif
               // 所有每次循环 a = 0；
               int a = 0;
               node *ch = CHILD(n, i+1);
               if (i > 0) {
                       com_addoparg(c, SET_LINENO, ch->n_lineno);
               }
               // 编译 test
               com_node(c, CHILD(n, i+1));
               // 插入回填地址，如果 test 语句条件不满足则跳转到下一个 elif
               com_addfwref(c, JUMP_IF_FALSE, &a);
               // 清理 test 判断的值，结果放stack了，来到这里就是条件满足了，把结果 pop 掉
               com_addbyte(c, POP_TOP);
               // 解析 suite
               com_node(c, CHILD(n, i+3));
               // 插入 整个 if 语句的结束回填地址，因为既然这个 if/elif 处理完了就可以直接结束了
               com_addfwref(c, JUMP_FORWARD, &anchor);
               // 回填 当前 if/elif 语句块的地址，其实也就是下一个 elif或者else的地址
               com_backpatch(c, a);
               com_addbyte(c, POP_TOP);
       }
       if (i+2 < NCH(n)) {
               //这个是编译有 else 语句的
               com_node(c, CHILD(n, i+2));
       }
       // 回填整个 if 语句的结束地址，回填所有 anchor 链节点
       com_backpatch(c, anchor);
}

static void
com_while_stmt(c, n)
       struct compiling *c;
       node *n;
{
       int break_anchor = 0; //回填地址偏移
       int anchor = 0; //回填地址偏移
       int begin; //suit指令偏移
       REQ(n, while_stmt); /* 'while' test ':' suite ['else' ':' suite] */
       /*
       node *n0 = CHILD(n, 0); // 'while'
       node *n1 = CHILD(n, 1); // test
       node *n2 = CHILD(n, 2); // ':'
       node *n3 = CHILD(n, 3); // suite

       [xxx]
       这个 SETUP_LOOP 指令是设置一个 setup_block 函数调用，对应应该是后续的 POP_BLOCK 指令 
       [SETUP_LOOP break_anchor] 这个 break_anchor 是一个绝对值，指向的是suite第一条语句---------------->>
       while                                                                                         |
           test :  ----> begin                                                                       |
         [JUMP_IF_FALSE anchor] 这个anchor 是相对值指向循环结束接跟着的清理语句，条件是test运算为FALSE    |
         suite...                   |    <<----------------------------------------------------------<<
         suite...                   |
         suite...                   |
         [JUMP_ABSOLUTE begin]      |  跳转到 test begin 重新下一轮循环                    
       [POP_TOP] <------------------<
       [POP_BLOCK]
       */
       //break_anchor对应的是SETUP_LOOP指令参数（跳转位置）的偏移，用于后面回填，这个值记录的是
       //整个循环的范围（包含了清理语句 POP_TOP 和 POP_BLOCk
       com_addfwref(c, SETUP_LOOP, &break_anchor); 
       begin = c->c_nexti;
       com_addoparg(c, SET_LINENO, n->n_lineno);
       //解释test代码，解释完这部分代码后，下面的com_addfwref就可以获取suit第一条代码的位置了，
       //用于后续回填地址
       com_node(c, CHILD(n, 1));
       //当前anchor对应的是JUMP_IF_FALSE指令后面的参数位置，用于编译完整个循环后得出循环结束地址后，
       //再回填JUMP_IF_FALSE这个参数真正的地址，这个回填工作是后面com_backpatch(c, anchor)做的事情
       //这里要做的除了插入[JUMP_IF_FALSE 0]之外（0是暂时填上去占个坑，对于while循环，这个anchor是0），
       //还记录了参数0对应的指令偏移，有了这个偏移后面回填
       //就可以填上真正的 [JUMP_IF_FALSE 循环结束清理地址] 了
       //根据JUMP_IF_FALSE指令，执行的是相对跳转，因此在执行时会用当前指令偏移+anchor实现跳转到循环
       //结束的清理语句位置执行，具体可以看ceval.c解释执行部分代码
       com_addfwref(c, JUMP_IF_FALSE, &anchor); 
       com_addbyte(c, POP_TOP);
       c->c_loops++;
       com_node(c, CHILD(n, 3));
       c->c_loops--;
       com_addoparg(c, JUMP_ABSOLUTE, begin); //跳转到test准备第二轮循环
       //编译完所有suite指令后，可以得出当前地址了，然后再去回填anchor
       com_backpatch(c, anchor);
       com_addbyte(c, POP_TOP);
       com_addbyte(c, POP_BLOCK);
       //如果是有 else 的则继续编译 else 对应的 suit 代码
       if (NCH(n) > 4) {
               com_node(c, CHILD(n, 6));
       }
       //全部都编译完成了，那么就可以得出整个循环的结束位置范围（包括清理代码 POP_TOP POP_BLOCK），可以回填了
       //跟anchor的原理一样，这个break_anchor也是 SETUP_LOOP 指令参数的一个位置，
       com_backpatch(c, break_anchor);
}

static void
com_for_stmt(c, n)
       struct compiling *c;
       node *n;
{
       object *v;
       int break_anchor = 0;
       int anchor = 0;
       int begin;
       REQ(n, for_stmt);
       /* 'for' exprlist 'in' exprlist ':' suite ['else' ':' suite] */
       com_addfwref(c, SETUP_LOOP, &break_anchor);
       com_node(c, CHILD(n, 3));
       v = newintobject(0L);
       if (v == NULL)
               c->c_errors++;
       com_addoparg(c, LOAD_CONST, com_addconst(c, v));
       XDECREF(v);
       begin = c->c_nexti;
       com_addoparg(c, SET_LINENO, n->n_lineno);
       com_addfwref(c, FOR_LOOP, &anchor);
       com_assign(c, CHILD(n, 1), 1/*assigning*/);
       c->c_loops++;
       com_node(c, CHILD(n, 5));
       c->c_loops--;
       com_addoparg(c, JUMP_ABSOLUTE, begin);
       com_backpatch(c, anchor);
       com_addbyte(c, POP_BLOCK);
       if (NCH(n) > 8)
               com_node(c, CHILD(n, 8));
       com_backpatch(c, break_anchor);
}

/* Although 'execpt' and 'finally' clauses can be combined
   syntactically, they are compiled separately.  In fact,
       try: S
       except E1: S1
       except E2: S2
       ...
       finally: Sf
   is equivalent to
       try:
           try: S
           except E1: S1
           except E2: S2
           ...
       finally: Sf
   meaning that the 'finally' clause is entered even if things
   go wrong again in an exception handler.  Note that this is
   not the case for exception handlers: at most one is entered.
   
   上面这段话大概讲述了，finally是独立自己一个block的，防止在except的block里面又发生错误异常。

   Code generated for "try: S finally: Sf" is as follows:

               SETUP_FINALLY   L
               <code for S>
               POP_BLOCK
               LOAD_CONST      <nil>
       L:      <code for Sf>
               END_FINALLY

   The special instructions use the block stack.  Each block
   stack entry contains the instruction that created it (here
   SETUP_FINALLY), the level of the value stack at the time the
   block stack entry was created, and a label (here L).

   上面这段话大概介绍了block的标记情况，比如对finally有一个专门的SETUP_FINALLY指令，
   这个指令会记录当前stack信息，这个L就是finally block的偏移位置，用于跳转到finally
   执行代码的。每一个block都记录两个重要的关键信息，一个是stack用于恢复清理block里面
   代码产生的临时元素；一个是handler用于指定block完结后应该跳转到那里去继续执行，比如
   try的block完了会去执行finally的代码。

   SETUP_FINALLY:
       Pushes the current value stack level and the label
       onto the block stack.
       这个指令是把当前stack指针偏移和对应finally的位置保存到block数据结构然后push到
       frame的block stack。
   POP_BLOCK:
       Pops en entry from the block stack, and pops the value
       stack until its level is the same as indicated on the
       block stack.  (The label is ignored.)
       这个指令是从frame的blockstack取一个block元素出来，把当前stack恢复成block元素
       记录的stack状态，就是清理stack在上一个代码块block所产生的临时变量数据
   END_FINALLY:
       Pops a variable number of entries from the *value* stack
       and re-raises the exception they specify.  The number of
       entries popped depends on the (pseudo) exception type.
       END_FINALLY有点难以理解，看编译代码，只要有except就会有一个END_FINALLY，有finally
       也有一个END_FINALLY，所以如果代码里面有except和finally那就会生成两个END_FINALLY，
       至于END_FINALLY的作用看注释意思是说清理stack元素然后再重新抛出一个异常。经过实战调试
       后确认，假如except E1，except E2，等等，只要有异常类型匹配了，END_FINALLY 里面 v=pop()
       v是NoneObject，但是假如没有一个异常类型匹配那么END_FINALLY的作用就是重新抛出一个异常
       re-raises让外面的去捕获去处理，所以个人理解END_FINALLY的作用就是看所有except语句结束后
       看看stack里面是否有未能识别处理的异常，有的话就重新抛到外面去处理。

   The block stack is unwound when an exception is raised:
   when a SETUP_FINALLY entry is found, the exception is pushed
   onto the value stack (and the exception condition is cleared),
   and the interpreter jumps to the label gotten from the block
   stack.

   Code generated for "try: S except E1, V1: S1 except E2, V2: S2 ...":
   (The contents of the value stack is shown in [], with the top
   at the right; 'tb' is trace-back info, 'val' the exception's
   associated value, and 'exc' the exception.)

   下面以
   try:
     S
   except E1, V1:
     S1
   except E2, V2:
     S2
   为例子，记录stack情况，其中 ：
   tb 为trace-back info；
   val 为except关联数据；
   exc 为except信息
   下面这个图非常关键，可以理解stack元素push、pop情况，也知道为什么代码有POP的操作

   Value stack         Label   Instruction     Argument
   []                          SETUP_EXCEPT    L1                              #SETUP_EXCEPT指令会记录当前stack及L1（L1为except入口偏移）信息到frame的block stack
   []                          <code for S>
   []                          POP_BLOCK
   []                          JUMP_FORWARD    L0                              #正常执行完毕直接跳转到L0继续执行其他语句

   [tb, val, exc]      L1:     DUP                             )               #except E1，V1的逻辑及stack情况
   [tb, val, exc, exc]         <evaluate E1>                     )
   [tb, val, exc, exc, E1]     COMPARE_OP      EXC_MATCH       ) only if E1    #判断是否是E1异常
   [tb, val, exc, 1-or-0]      JUMP_IF_FALSE   L2              )               #如果不是E1异常则去L2，判断是否为E2异常
   [tb, val, exc, 1]           POP                             )               #E1异常逻辑
   [tb, val, exc]              POP
   [tb, val]                   <assign to V1>    (or POP if no V1)
   [tb]                                POP
   []                          <code for S1>
                               JUMP_FORWARD    L0                              #执行完E1异常逻辑跳去L0

   [tb, val, exc, 0]   L2:     POP                                             #E2异常处理逻辑
   [tb, val, exc]              DUP
   .............................etc.......................

   [tb, val, exc, 0]   Ln+1:   POP
   [tb, val, exc]              END_FINALLY     # re-raise exception

   []                  L0:     <next statement>

   Of course, parts are not generated if Vi or Ei is not present.
*/

static void
com_try_stmt(c, n)
       struct compiling *c;
       node *n;
{
       // finally回填地址偏移
       // 这个回填地址就是finally入口地址，这个要把try except 这部分语句都生成完才知道finally的地址才回填
       // 对finally来讲，只有一个回填节点
       int finally_anchor = 0; 
       // except回填地址链节点头，默认值为0，当前值为链头，链头初值为0，每增加一个回填节点记录的是距离上一个节点的偏移
       int except_anchor = 0;  
       REQ(n, try_stmt);
       /* 'try' ':' suite (except_clause ':' suite)* ['finally' ':' suite] */
       /*
       node *n0 = CHILD(n, 0); // 'try'
       node *n1 = CHILD(n, 1); // ':'
       node *n2 = CHILD(n, 2); // suite
       node *n3 = CHILD(n, 3); // except_clause
       node *n4 = CHILD(n, 4); // ':'
       node *n5 = CHILD(n, 5); // suite
       node *n6 = CHILD(n, 6); // finally
       node *n7 = CHILD(n, 7); // ':'
       node *n8 = CHILD(n, 8); // suite
       */
       if (NCH(n) > 3 && TYPE(CHILD(n, NCH(n)-3)) != except_clause) {
               /* Have a 'finally' clause */
               //有finally的先生成 SETUP_FINALLY 指令及设置第一个finally的回填链头
               //这里会调用setup_block, push一个block到frame
               com_addfwref(c, SETUP_FINALLY, &finally_anchor);
       }
       if (NCH(n) > 3 && TYPE(CHILD(n, 3)) == except_clause) {
               /* Have an 'except' clause */
               //有except的先生成 SETUP_EXCEPT 指令及设置第一个except的回填链头
               //这里会调用setup_block, push一个block到frame
               com_addfwref(c, SETUP_EXCEPT, &except_anchor);
       }
       //编译 try 包含的语句
       com_node(c, CHILD(n, 2));
       if (except_anchor) {
               //如果有 except 语句，那么需要对 except 进行编译
               
               //这个end_anchor对应的是except结束位置，注意跟 except_anchor区分好
               //except_anchor是单个except的结束位置，而end_anchor是最后一个except
               //的结束位置
               int end_anchor = 0;
               int i;
               node *ch;
               //通常代码执行到这里就是没有产生异常了，如果产生异常会直接跳到except位置except_anchor
               com_addbyte(c, POP_BLOCK); //这行代码是提取出SETUP_EXCEPT时保存的stack信息恢复
               com_addfwref(c, JUMP_FORWARD, &end_anchor); //跳转到except结束位置
               //上面这两条语句是没有异常时执行的，恢复stack然后跳转到except后面的语句继续执行
               //这里才是except的入口，有异常发生时会从这里开始执行，也就是回填except_anchor地址
               com_backpatch(c, except_anchor);
               for (i = 3;
                       i < NCH(n) && TYPE(ch = CHILD(n, i)) == except_clause;
                                                               i += 3) {
                       //这个循环是针对有多个except的，如果只有一个except就轮一次
                       /* except_clause: 'except' [expr [',' expr]] */
                       if (except_anchor == 0) {
                               err_setstr(TypeError,
                                       "default 'except:' must be last");
                               c->c_errors++;
                               break;
                       }
                       //except_anchor这个是这次except的结束位置也就是下一个except的入口
                       //注意跟上面end_anchor的区别，那个是最后一个except也就是所有except
                       //解释完后的结束位置，所以每次循环都要重新把except_anchor置零，因为
                       //他没有链条关系，单独的一个except而已
                       except_anchor = 0;
                       com_addoparg(c, SET_LINENO, ch->n_lineno);
                       if (NCH(ch) > 1) {
                               //这里是编译有匹配异常类型的逻辑，这里有个DUP_TOP的操作，这个操作就是把当前stack
                               //top的元素复制一份，就是把异常信息元素复制一份，用来下面做EXC_MATCH异常匹配比较
                               //因为这个操作会pop一个异常作为参数，所以这里DUP_TOP一份就是这个意思，防止匹配不
                               //符合后还继续保留一个异常信息元素留待下一个匹配或者抛出外面处理
                               com_addbyte(c, DUP_TOP);
                               com_node(c, CHILD(ch, 1));
                               //比较是否匹配异常类型
                               com_addoparg(c, COMPARE_OP, EXC_MATCH);
                               //如果不匹配，那么就跳转到下一个except异常判断逻辑，这里需要注意except_anchor是每个
                               //循环都会重置为0，因为有多个except的话，每个except入口都是不一样的，所以这个for循环
                               //处理每个except都不同。
                               //类型匹配只能而且也只会有一次TRUE的情况，所以不匹配的就继续判断下一个except
                               com_addfwref(c, JUMP_IF_FALSE, &except_anchor);
                               //假如异常匹配了没有跳转就会来到这里，我们需要把TOP POP掉，因为COMPARE_OP会把比较结果
                               //PUSH到stack，所以这里需要POP掉COMPARE_OP的结果
                               com_addbyte(c, POP_TOP);
                       }
                       /*
                       下面是编译except逻辑，对于有多个except（就是有指定匹配类型的）或者一个except都会生成对应的
                       代码，但是真正执行的时候只会执行一个一次，对于多个except只会有一次匹配，对于没有指定类型的那么
                       也只有一个except，所以下面的代码虽然循环生成一个或者多个except逻辑，但是实际上只会执行一次，
                       上面JUMP_IF_FALSE也有解释，所以我们要带着执行时的逻辑去看代码，而不仅仅是编译的逻辑去看代码
                       */
                       //这个POP是POP掉异常元素[exc]，来到这里已经是可以处理异常了，具体可以看上面官方自带的注释stack元素情况
                       com_addbyte(c, POP_TOP);
                       if (NCH(ch) > 3)
                               com_assign(c, CHILD(ch, 3), 1/*assigning*/);//有赋值异常信息的需要再解释
                       else
                               com_addbyte(c, POP_TOP);//这里根据注释是pop掉[val]元素，因为代码没有对异常信息赋值，所以也没用了可以清理
                       //根据官方注释，这里是pop掉[tb] trace-backinfo 元素
                       com_addbyte(c, POP_TOP);
                       //编译except具体逻辑
                       com_node(c, CHILD(n, i+2));
                       //except完了就跳转到except的结束位置
                       com_addfwref(c, JUMP_FORWARD, &end_anchor);
                       if (except_anchor) {
                               com_backpatch(c, except_anchor);
                               com_addbyte(c, POP_TOP);
                       }
               }
               //加入 END_FINALLY 指令跟进是否还有未处理或者未识别的 异常，具体看指令执行逻辑
               com_addbyte(c, END_FINALLY);
               //编译完except了，回填 except 结束地址 end_anchor 了
               com_backpatch(c, end_anchor);
       }
       if (finally_anchor) {
               //假如有 finally 则解释 finally 部分
               node *ch;
               //POP_BLOCK清理stack临时生成的元素
               com_addbyte(c, POP_BLOCK);
               com_addoparg(c, LOAD_CONST, com_addconst(c, None));
               //回填finally入口地址
               com_backpatch(c, finally_anchor);
               ch = CHILD(n, NCH(n)-1);
               com_addoparg(c, SET_LINENO, ch->n_lineno);
               com_node(c, ch);
               com_addbyte(c, END_FINALLY);
       }
}

static void
com_suite(c, n)
       struct compiling *c;
       node *n;
{
       REQ(n, suite);
       /* simple_stmt | NEWLINE INDENT NEWLINE* (stmt NEWLINE*)+ DEDENT */
       if (NCH(n) == 1) {
               com_node(c, CHILD(n, 0));
       }
       else {
               int i;
               for (i = 0; i < NCH(n); i++) {
                       node *ch = CHILD(n, i);
                       if (TYPE(ch) == stmt)
                               com_node(c, ch);
               }
       }
}

static void
com_funcdef(c, n)
       struct compiling *c;
       node *n;
{
       /*
       这里可能会有疑问，按照正常理解，进入函数有保存stack的逻辑，类似steup block这样的操作，在函数离开退出的时候
       恢复stack信息，跟以前c、c++生成的汇编语言 push all 和 pop all 差不多，但是为什么python没有呢，不像while、for、try...except...finalyy
       那样弄个setup block，是因为在python编译机制里面，针对类、函数都会单独调用 compile 函数，生成一个独立的 *v 结构，BUILD_FUNCTION 和 
       BUILD_CLASS 会生成对应的独立实例保存 locals 等，所以就不需要额外的stack保存恢复了
       */
       object *v;
       REQ(n, funcdef); /* funcdef: 'def' NAME parameters ':' suite */
       v = (object *)compile(n, c->c_filename);
       if (v == NULL)
               c->c_errors++;
       else {
               // *v是编译好的函数指令代码，把他保存到const
               int i = com_addconst(c, v);
               // 生成一个 LOAD_CONST 指令加载 const 数据到 push stack，作为下面的 BUILD 指令参数
               com_addoparg(c, LOAD_CONST, i);
               // 生成 BUILD 构造指令，参数上面已经push 到 stack了
               com_addbyte(c, BUILD_FUNCTION);
               //每一个函数编译=》构造完后，都会用 STORE_NAME 指令保存在 frame 的 f_locals 里面
               //这个信息很关键，因为后续比如编译类成员函数的时候，需要获取这个类的所有函数构造成
               //一个字典，具体看类编译部分
               com_addopname(c, STORE_NAME, CHILD(n, 1));
               DECREF(v);
       }
}

static void
com_bases(c, n)
       struct compiling *c;
       node *n;
{
       int i, nbases;
       REQ(n, baselist);
       /*
       baselist: atom arguments (',' atom arguments)*
       arguments: '(' [testlist] ')'
       */
       for (i = 0; i < NCH(n); i += 3)
               com_node(c, CHILD(n, i));
       com_addoparg(c, BUILD_TUPLE, (NCH(n)+1) / 3);
}

static void
com_classdef(c, n)
       struct compiling *c;
       node *n;
{
       object *v;
       REQ(n, classdef);
       /*
       classdef: 'class' NAME parameters ['=' baselist] ':' suite
       baselist: atom arguments (',' atom arguments)*
       arguments: '(' [testlist] ')'
       */
       if (NCH(n) == 7) {
           //有父类，则调用com_bases编译
           com_bases(c, CHILD(n, 4));
       } else {
           //没有父类则弄个None，push一个Node到stack
           com_addoparg(c, LOAD_CONST, com_addconst(c, None));
       }
       //编译类，这里的类有可能是单独一个文件的，返回的是类编译代码 *v
       //类和函数一样，是单独 compile 编译一个 *v 实例的
       v = (object *)compile(n, c->c_filename);
       if (v == NULL)
               c->c_errors++;
       else {
               //把类编译代码添加到const，并获取对应索引 i
               int i = com_addconst(c, v);
               //生成一个const加载到stack指令，具体看LOAD_CONST指令运行机制，这里就是push类编译代码到stack
               com_addoparg(c, LOAD_CONST, i);
               //生成一个BUILD_FUNCTION代码，这个BUILD_FUNCTION是为了执行类里面的函数构造指令而设定的
               //python里面编译函数指令是一部分，但是在运行时还需要专门的构造指令BUILD_FUNCTION去为函数创建分配对应的数据结构
               //因此，类里面的函数是编译好了，但是还需要去运行对应 *v 里面的BUILD_FUNCTION指令（多少个函数里面就有多少个指令，这里说的是 *v里面，而不是下面那个）
               //所以，一个类编译好了之后，就要生成一个BUILD_FUNCTION去运行 *v 类里面的BUILD_FUNCTION指令进行成员函数的构造
               com_addbyte(c, BUILD_FUNCTION);
               //上面生成了BUILD_FUNCTION指令了，接下来就需要调用，这里是无参数调用因此就生成一个UNARY_CALL，这个指令就会去触发 *v 里面的成员
               //函数进行逐个构造BUILD_FUNCTION
               com_addbyte(c, UNARY_CALL);
               //生成一个类构造指令BUILD_CLASS，这里会跟距 父类和类 的编译代码来构造一个类信息 newclassobject 对象push回stack
               //父类base信息比较好获取，上面已经有com_base了，那么类里面的函数信息如何获取？需要看 compile_node函数里面 case classdef 逻辑
               //成员里面函数编译完后保存在locals，然后作为返回值push到stack，因此 BUILD_CLASS 指令执行的时候，可以pop stack来获取函数列表信息
               //具体看BUILD_CLASS的执行逻辑
               com_addbyte(c, BUILD_CLASS);
               //生成一个STORE_NAME指令，把类代码信息保存到 frame 的 local 变量字典
               com_addopname(c, STORE_NAME, CHILD(n, 1));
               DECREF(v);
       }
}

static void
com_node(c, n)
       struct compiling *c;
       node *n;
{
       switch (TYPE(n)) {

       /* Definition nodes */

       case funcdef:
               com_funcdef(c, n);
               break;
       case classdef:
               com_classdef(c, n);
               break;

       /* Trivial parse tree nodes */

       case stmt:
       case flow_stmt:
               com_node(c, CHILD(n, 0));
               break;

       case simple_stmt:
       case compound_stmt:
               com_addoparg(c, SET_LINENO, n->n_lineno);
               com_node(c, CHILD(n, 0));
               break;

       /* Statement nodes */

       case expr_stmt:
               com_expr_stmt(c, n);
               break;
       case print_stmt:
               com_print_stmt(c, n);
               break;
       case del_stmt: /* 'del' exprlist NEWLINE */
               com_assign(c, CHILD(n, 1), 0/*delete*/);
               break;
       case pass_stmt:
               break;
       case break_stmt:
               if (c->c_loops == 0) {
                       err_setstr(TypeError, "'break' outside loop");
                       c->c_errors++;
               }
               com_addbyte(c, BREAK_LOOP);
               break;
       case return_stmt:
               com_return_stmt(c, n);
               break;
       case raise_stmt:
               com_raise_stmt(c, n);
               break;
       case import_stmt:
               com_import_stmt(c, n);
               break;
       case if_stmt:
               com_if_stmt(c, n);
               break;
       case while_stmt:
               com_while_stmt(c, n);
               break;
       case for_stmt:
               com_for_stmt(c, n);
               break;
       case try_stmt:
               com_try_stmt(c, n);
               break;
       case suite:
               com_suite(c, n);
               break;

       /* Expression nodes */

       case testlist:
               com_list(c, n);
               break;
       case test:
               com_test(c, n);
               break;
       case and_test:
               com_and_test(c, n);
               break;
       case not_test:
               com_not_test(c, n);
               break;
       case comparison:
               com_comparison(c, n);
               break;
       case exprlist:
               com_list(c, n);
               break;
       case expr:
               com_expr(c, n);
               break;
       case term:
               com_term(c, n);
               break;
       case factor:
               com_factor(c, n);
               break;
       case atom:
               com_atom(c, n);
               break;

       default:
               fprintf(stderr, "node type %d\n", TYPE(n));
               err_setstr(SystemError, "com_node: unexpected node type");
               c->c_errors++;
       }
}

static void com_fplist PROTO((struct compiling *, node *));

static void
com_fpdef(c, n)
       struct compiling *c;
       node *n;
{
       REQ(n, fpdef); /* fpdef: NAME | '(' fplist ')' */
       if (TYPE(CHILD(n, 0)) == LPAR)
               com_fplist(c, CHILD(n, 1));
       else
               com_addopname(c, STORE_NAME, CHILD(n, 0));
}

static void
com_fplist(c, n)
       struct compiling *c;
       node *n;
{
       REQ(n, fplist); /* fplist: fpdef (',' fpdef)* */
       if (NCH(n) == 1) {
               com_fpdef(c, CHILD(n, 0));
       }
       else {
               int i;
               com_addoparg(c, UNPACK_TUPLE, (NCH(n)+1)/2);
               for (i = 0; i < NCH(n); i += 2)
                       com_fpdef(c, CHILD(n, i));
       }
}

static void
com_file_input(c, n)
       struct compiling *c;
       node *n;
{
       int i;
       REQ(n, file_input); /* (NEWLINE | stmt)* ENDMARKER */
       for (i = 0; i < NCH(n); i++) {
               node *ch = CHILD(n, i);
               if (TYPE(ch) != ENDMARKER && TYPE(ch) != NEWLINE)
                       com_node(c, ch);
       }
}

/* Top-level compile-node interface */

static void
compile_funcdef(c, n)
       struct compiling *c;
       node *n;
{
       node *ch;
       REQ(n, funcdef); /* funcdef: 'def' NAME parameters ':' suite */
       ch = CHILD(n, 2); /* parameters: '(' [fplist] ')' */
       ch = CHILD(ch, 1); /* ')' | fplist */
       if (TYPE(ch) == RPAR)
               com_addbyte(c, REFUSE_ARGS);
       else {
               com_addbyte(c, REQUIRE_ARGS);
               com_fplist(c, ch);
       }
       c->c_infunction = 1;
       com_node(c, CHILD(n, 4));
       c->c_infunction = 0;
       com_addoparg(c, LOAD_CONST, com_addconst(c, None));
       com_addbyte(c, RETURN_VALUE);
}

static void
compile_node(c, n)
       struct compiling *c;
       node *n;
{
       com_addoparg(c, SET_LINENO, n->n_lineno);

       switch (TYPE(n)) {

       case single_input: /* One interactive command */
               /* NEWLINE | simple_stmt | compound_stmt NEWLINE */
               com_addbyte(c, REFUSE_ARGS);
               n = CHILD(n, 0);
               if (TYPE(n) != NEWLINE)
                       com_node(c, n);
               com_addoparg(c, LOAD_CONST, com_addconst(c, None));
               com_addbyte(c, RETURN_VALUE);
               break;

       case file_input: /* A whole file, or built-in function exec() */
               com_addbyte(c, REFUSE_ARGS);
               com_file_input(c, n);
               com_addoparg(c, LOAD_CONST, com_addconst(c, None));
               com_addbyte(c, RETURN_VALUE);
               break;

       case expr_input: /* Built-in function eval() */
               com_addbyte(c, REFUSE_ARGS);
               com_node(c, CHILD(n, 0));
               com_addbyte(c, RETURN_VALUE);
               break;

       case eval_input: /* Built-in function input() */
               com_addbyte(c, REFUSE_ARGS);
               com_node(c, CHILD(n, 0));
               com_addbyte(c, RETURN_VALUE);
               break;

       case funcdef: /* A function definition */
               compile_funcdef(c, n);
               break;

       case classdef: /* A class definition */
               /* 'class' NAME parameters ['=' baselist] ':' suite */
               com_addbyte(c, REFUSE_ARGS);
               com_node(c, CHILD(n, NCH(n)-1));
               //先看com_funcdef函数说明，每一个函数编译完都会借助STORE_NAME指令把函数信息保存到 frame 的 locals
               //当一个类里面的函数编译完之后，locals里面就记录了所有类里面的函数信息了，这里需要作为参数返回到外面
               //因为外层的com_classdef函数需要做构造类的指令，其中需要类里面的函数信息
               com_addbyte(c, LOAD_LOCALS);
               com_addbyte(c, RETURN_VALUE);
               break;

       default:
               fprintf(stderr, "node type %d\n", TYPE(n));
               err_setstr(SystemError, "compile_node: unexpected node type");
               c->c_errors++;
       }
}

codeobject *
compile(n, filename)
       node *n;
       char *filename;
{
       struct compiling sc;
       codeobject *co;
       if (!com_init(&sc, filename))
               return NULL;
       compile_node(&sc, n);
       com_done(&sc);
       if (sc.c_errors == 0)
               co = newcodeobject(sc.c_code, sc.c_consts, sc.c_names, filename);
       else
               co = NULL;
        /**/
       printf("==================%d %s===================\n", n->n_type, filename);
       for( int i = 0; i < sc.c_nexti; i++ ) {
               int byte = getstringvalue(sc.c_code)[i];
               char *szbyte = opcode_str[byte];
               if( HAS_ARG(byte) ) {
                       int arg = getstringvalue(sc.c_code)[i+1];
                       printf("%02d: [%d][%s][%d]\n", i, byte, szbyte, arg);
                       i+=2;
               } else {
                       printf("%02d: [%d][%s]\n", i, byte, szbyte);
               }
       }

       com_free(&sc);
       return co;
}

