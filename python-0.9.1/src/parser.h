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

/* Parser interface */

#define MAXSTACK 100

//为stack专门设计的元素数据结构，里面存放的就是DFA和state
typedef struct _stackentry {
       int              s_state;       /* State in current DFA */
       dfa             *s_dfa;         /* Current DFA */
       // push到p_stack的元素记录了上层父节点的信息       
       struct _node    *s_parent;      /* Where to add next node */
} stackentry;

/*
parser的stack结构是专门为dfa和state而设置的，采用的是push状态入stack的方式来分析
*/
typedef struct _stack {
       stackentry      *s_top;         /* Top entry */
       stackentry       s_base[MAXSTACK];/* Array of stack entries */
                                       /* NB The stack grows down */
} stack;

//语法分析的主数据结构
typedef struct {
       //语法树stack，这里采用的是把语法对应的各个状态不断push stack，然后再从末端 pop回来的方式分析语法
       struct _stack    p_stack;       /* Stack of parser states */
       
       //语法DFA
       struct _grammar *p_grammar;     /* Grammar to use */
       
       // node 记录的是分析树里面包含对应语法树的各个节点，是作为分析的返回结果
       // 这里需要注意了，这个 p_tree 是语法树的顶层父节点，作为push到p_stack元素的s_parent
       // 
       struct _node    *p_tree;        /* Top of parse tree */
} parser_state;

parser_state *newparser PROTO((struct _grammar *g, int start));
void delparser PROTO((parser_state *ps));
int addtoken PROTO((parser_state *ps, int type, char *str, int lineno));
void addaccelerators PROTO((grammar *g));
