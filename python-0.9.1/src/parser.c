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

/* Parser implementation */

/* For a description, see the comments at end of this file */

/* XXX To do: error recovery */

#include "pgenheaders.h"
#include "assert.h"
#include "token.h"
#include "grammar.h"
#include "node.h"
#include "parser.h"
#include "errcode.h"


#ifdef DEBUG
extern int debugging;
#define D(x) if (!debugging); else x
#else
#define D(x)
#endif


/* STACK DATA TYPE */

static void s_reset PROTO((stack *));

static void
s_reset(s)
       stack *s;
{
       s->s_top = &s->s_base[MAXSTACK];
}

#define s_empty(s) ((s)->s_top == &(s)->s_base[MAXSTACK])

static int s_push PROTO((stack *, dfa *, node *));

static int
s_push(s, d, parent)
       register stack *s;
       dfa *d;
       node *parent;
{
       register stackentry *top;
       if (s->s_top == s->s_base) {
               fprintf(stderr, "s_push: parser stack overflow\n");
               return -1;
       }
       top = --s->s_top;
       top->s_dfa = d;
       top->s_parent = parent;
       top->s_state = 0;
       return 0;
}

#ifdef DEBUG

static void s_pop PROTO((stack *));

static void
s_pop(s)
       register stack *s;
{
       if (s_empty(s)) {
               fprintf(stderr, "s_pop: parser stack underflow -- FATAL\n");
               abort();
       }
       s->s_top++;
}

#else /* !DEBUG */

#define s_pop(s) (s)->s_top++

#endif


/* PARSER CREATION */

parser_state *
newparser(g, start)
       grammar *g;
       int start;
{
       parser_state *ps;

       if (!g->g_accel)
               addaccelerators(g);
       ps = NEW(parser_state, 1);
       if (ps == NULL)
               return NULL;
       ps->p_grammar = g;
       ps->p_tree = newtree(start);
       if (ps->p_tree == NULL) {
               DEL(ps);
               return NULL;
       }
       s_reset(&ps->p_stack);
       /*
       初始化stack，首先push进去的是指定 start 的 DFA
       #define file_input 257  start=257 代表的是以执行文件的方式运行起python
       #define single_input 256  start=256 代表的是以交换方式也就是直接运行python命令行运行起来
       不同的方式导致 start 值是不一样的，根据 start 值找到对应的 DFA 然后去初始化 stack

       注意：ps->p_tree作为父节点被传递到初始化push的第一个元素，整个p_stack的DFA、state的信息都是一颗树
       */
       (void) s_push(&ps->p_stack, finddfa(g, start), ps->p_tree);
       return ps;
}

void
delparser(ps)
       parser_state *ps;
{
       /* NB If you want to save the parse tree,
          you must set p_tree to NULL before calling delparser! */
       freetree(ps->p_tree);
       DEL(ps);
}


/* PARSER STACK OPERATIONS */

static int shift PROTO((stack *, int, char *, int, int));

static int
shift(s, type, str, newstate, lineno)
       register stack *s;
       int type;
       char *str;
       int newstate;
       int lineno;
{
       assert(!s_empty(s));
       if (addchild(s->s_top->s_parent, type, str, lineno) == NULL) {
               fprintf(stderr, "shift: no mem in addchild\n");
               return -1;
       }
       s->s_top->s_state = newstate;
       return 0;
}

static int push PROTO((stack *, int, dfa *, int, int));

static int
push(s, type, d, newstate, lineno)
       register stack *s;
       int type;
       dfa *d;
       int newstate;
       int lineno;
{
       register node *n;
       n = s->s_top->s_parent;
       assert(!s_empty(s));
       if (addchild(n, type, (char *)NULL, lineno) == NULL) {
               fprintf(stderr, "push: no mem in addchild\n");
               return -1;
       }
       s->s_top->s_state = newstate;
       return s_push(s, d, CHILD(n, NCH(n)-1));
}


/* PARSER PROPER */

static int classify PROTO((grammar *, int, char *));

static int
classify(g, type, str)
       grammar *g;
       register int type;
       char *str;
{
       register int n = g->g_ll.ll_nlabels;

       if (type == NAME) {
               register char *s = str;
               register label *l = g->g_ll.ll_label;
               register int i;
               for (i = n; i > 0; i--, l++) {
                       if (l->lb_type == NAME && l->lb_str != NULL &&
                                       l->lb_str[0] == s[0] &&
                                       strcmp(l->lb_str, s) == 0) {
                               D(printf("It's a keyword [label=%d] \n", n - i));
                               return n - i;
                       }
               }
       }

       {
               register label *l = g->g_ll.ll_label;
               register int i;
               for (i = n; i > 0; i--, l++) {
                       if (l->lb_type == type && l->lb_str == NULL) {
                               D(printf("It's a token we know [label=%d] \n", n - i));
                               return n - i;
                       }
               }
       }

       D(printf("Illegal token\n"));
       return -1;
}

int
addtoken(ps, type, str, lineno)
       register parser_state *ps;
       register int type;
       char *str;
       int lineno;
{
       register int ilabel;

       /*
       个人觉得语法树的生成有点类似 深度优先 算法，每一条分支去匹配，不行再回退再尝试另外一条分支，满足了就结束
       我觉得这里可以用递归的，不过它没有，用了堆栈的方式（堆栈消除递归），后面的解释运行反而是用了递归的算法，具体看 ceval.c

       以 a = 1 + 1 只有一条语句的为例，最后生成的 p_tree 属性图是如下所示：

       file_input => stmt => simple_stmt => expr_stmt => exprlist => expr => term => factor => atom => NAME(a)
                                                      => EQUAL(=)
                                                      => exprlist => expr => term => factor => atom => NAME(a)
                                                                          => PLUS(+)
                                                                          => term => factor => atom => NAME(a)
       */
       /*
       注意：这里push有两个操作，一个是针对解释语法树DFA、state的 p_stack 结构的push对应是 s_push
            另外一个是对语法树node的 p_tree 结构的push对应的是push，注意push里面会同时调用s_push
       */

       D(printf("Token %s/'%s' ... ", tok_name[type], str));

       /* Find out which label this token is */
       //这里根据type和str两个token属性，去匹配对应graminit.c的labels然后得出labels的下标ilabel
       ilabel = classify(ps->p_grammar, type, str);
       if (ilabel < 0)
               return E_SYNTAX;

       /* Loop until the token is shifted or an error occurred */
       for (;;) {
               /* Fetch the current dfa and state */
               //获取当前stack里面的 DFA和state信息，在初始化parser的时候stack就有了起始的DFA、state元素了
               //以 file_input 为例，在newparser初始化的时候，已经将 file_input 的DFA和state push入stack了
               //所以第一次循环的时候，取出来的是 file_input 的 DFA和state
               //那么在第二次循环的时候，再取一次DFA、state就不再是 file_input 的了，是file_input的下一个状态 stmt
               //那么这个 stmt 的DFA、state信息是什么是push stack的呢，就看下面代码循环的 push(xx,xx,xx,xx,xx)部分
               register dfa *d = ps->p_stack.s_top->s_dfa;
               register state *s = &d->d_state[ps->p_stack.s_top->s_state];

               //当前在处理的 DFA、state
               //那对应是 push 还是 pop 操作，具体看下面的逻辑
               //正常情况下都是从外到内先push一轮到stack到终结符
               //然后再从底层内层去适配慢慢pop完就完成整个解释
               //这里打印的是当前p_stack第一个元素，这条语句还没结束，不是一个完整的日志输出，没有\n换行，后面
               //具体是 Push、shift、Pop、ACDEPT 加上才是真正完整一条
               D(printf(" DFA '%s', state %d:", d->d_name, ps->p_stack.s_top->s_state));

               /* Check accelerator */
               // ilabel 在这个 state 的 [lower,upper] 有效状态转换区间
               // 个人理解就是不在这个区间内的，都是不正常的状态转换
               if (s->s_lower <= ilabel && ilabel < s->s_upper) {
                       //这里的 x 是当前 state 的下一个状态信息，这个在 acceler.c fixstate函数已经有描述
                       //把x信息（也就是下一个状态）提出出来，低7位，高1位是type类型信息（fixstate函数已经有描述）
                       register int x = s->s_accel[ilabel - s->s_lower];
                       if (x != -1) {
                               if (x & (1<<7)) {
                                       /* Push non-terminal */
                                       //如果是非终结符，那么就继续找下一个状态并push stack
                                       //
                                       int nt = (x >> 8) + NT_OFFSET;  // 类型 type
                                       int arrow = x & ((1<<7)-1);     // 下一个状态指向
                                       // 根据 nt、arrow 信息查找对应的 DFA，然后把它 push 到 stack
                                       // 这样下一次循环提取的就是下一个状态的 DFA、state信息了，如此类推一直到语法状态结束为止
                                       dfa *d1 = finddfa(ps->p_grammar, nt);
                                       //
                                       //push是增加 p_tree node节点的同时，又 push p_stack 下一个状态，一共有2个职能
                                       //
                                       if (push(&ps->p_stack, nt, d1,
                                               arrow, lineno) < 0) {
                                               D(printf(" MemError: push.\n"));
                                               return E_NOMEM;
                                       }
                                       //这个push日志对应会显示push了什么dfa、state入stack，看上一个D(printf)日志
                                       D(printf(" Push ...\n"));
                                       //状态走向没完继续 continue
                                       continue;
                               }

                               /* Shift the token */
                               // shift 增加的是父节点下面的子节点
                               // 而这个父节点信息就是 shift 日志里面有显示
                               if (shift(&ps->p_stack, type, str,
                                               x, lineno) < 0) {
                                       D(printf(" MemError: shift.\n"));
                                       return E_NOMEM;
                               }
                               //但遇到了终结符之后，就开始一个个pop来适配对应的DFA、state，一直到处理完为止
                               D(printf(" Shift.\n"));
                               /* Pop while we are in an accept-only state */
                               while ( s = &d->d_state[ps->p_stack.s_top->s_state], 
                                       s->s_accept && s->s_narcs == 1) {
                                       D(printf("  Direct pop.\n"));
                                       s_pop(&ps->p_stack);
                                       if (s_empty(&ps->p_stack)) {
                                               D(printf("  ACCEPT.\n"));
                                               /* #define E_DONE 16 Parsing complete */
                                               // 全部分析完成就输出 ACCEPT 全部代码分析完成而不是某个代码
                                               return E_DONE;
                                       }
                                       d = ps->p_stack.s_top->s_dfa;
                               }
                               // addtoken 这个 token 分析完毕
                               D(printf("E_OK\n"));
                               return E_OK;
                       }
               }

               // pop一个dfa然后尝试下一个
               if (s->s_accept) {
                       /* Pop this dfa and try again */
                       // 根据 acceler.c accpet就是该状态流转到达结束了，既然是到达结束了
                       // 那么就有2种情况，1种是匹配符合语法树；1种是不匹配那就是语法有问题了
                       s_pop(&ps->p_stack);
                       D(printf(" Pop ...\n"));
                       //如果发现pop完之后，p_stack为空了，那就代表没有正常分析完一个状态图，
                       //应该是语法有错误了直接返回
                       if (s_empty(&ps->p_stack)) {
                               D(printf(" Error: bottom of stack.\n"));
                               return E_SYNTAX;
                       }
                       continue;
               }

               /* Stuck, report syntax error */
               D(printf(" Error.\n"));
               return E_SYNTAX;
       }
}


#ifdef DEBUG

/* DEBUG OUTPUT */

void
dumptree(g, n)
       grammar *g;
       node *n;
{
       int i;

       if (n == NULL)
               printf("NIL");
       else {
               label l;
               l.lb_type = TYPE(n);
               l.lb_str = STR(n);
               printf("%s", labelrepr(&l));
               if (ISNONTERMINAL(TYPE(n))) {
                       printf("(");
                       for (i = 0; i < NCH(n); i++) {
                               if (i > 0)
                                       printf(",");
                               dumptree(g, CHILD(n, i));
                       }
                       printf(")");
               }
       }
}

void
showtree(g, n)
       grammar *g;
       node *n;
{
       int i;

       if (n == NULL)
               return;
       if (ISNONTERMINAL(TYPE(n))) {
               for (i = 0; i < NCH(n); i++)
                       showtree(g, CHILD(n, i));
       }
       else if (ISTERMINAL(TYPE(n))) {
               printf("%s", tok_name[TYPE(n)]);
               if (TYPE(n) == NUMBER || TYPE(n) == NAME)
                       printf("(%s)", STR(n));
               printf(" ");
       }
       else
               printf("? ");
}

void
printtree(ps)
       parser_state *ps;
{
       if (debugging) {
               printf("Parse tree:\n");
               dumptree(ps->p_grammar, ps->p_tree);
               printf("\n");
               printf("Tokens:\n");
               showtree(ps->p_grammar, ps->p_tree);
               printf("\n");
       }
       printf("Listing:\n");
       listtree(ps->p_tree);
       printf("\n");
}

#endif /* DEBUG */

/*

Description
-----------

The parser's interface is different than usual: the function addtoken()
must be called for each token in the input.  This makes it possible to
turn it into an incremental parsing system later.  The parsing system
constructs a parse tree as it goes.

A parsing rule is represented as a Deterministic Finite-state Automaton
(DFA).  A node in a DFA represents a state of the parser; an arc represents
a transition.  Transitions are either labeled with terminal symbols or
with non-terminals.  When the parser decides to follow an arc labeled
with a non-terminal, it is invoked recursively with the DFA representing
the parsing rule for that as its initial state; when that DFA accepts,
the parser that invoked it continues.  The parse tree constructed by the
recursively called parser is inserted as a child in the current parse tree.

The DFA's can be constructed automatically from a more conventional
language description.  An extended LL(1) grammar (ELL(1)) is suitable.
Certain restrictions make the parser's life easier: rules that can produce
the empty string should be outlawed (there are other ways to put loops
or optional parts in the language).  To avoid the need to construct
FIRST sets, we can require that all but the last alternative of a rule
(really: arc going out of a DFA's state) must begin with a terminal
symbol.

As an example, consider this grammar:

expr:  term (OP term)*
term:  CONSTANT | '(' expr ')'

The DFA corresponding to the rule for expr is:

------->.---term-->.------->
       ^          |
       |          |
       \----OP----/

The parse tree generated for the input a+b is:

(expr: (term: (NAME: a)), (OP: +), (term: (NAME: b)))

*/
