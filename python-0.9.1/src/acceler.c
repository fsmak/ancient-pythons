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

/* Parser accelerator module */

/* The parser as originally conceived had disappointing performance.
   This module does some precomputation that speeds up the selection
   of a DFA based upon a token, turning a search through an array
   into a simple indexing operation.  The parser now cannot work
   without the accelerators installed.  Note that the accelerators
   are installed dynamically when the parser is initialized, they
   are not part of the static data structure written on graminit.[ch]
   by the parser generator. */

#include "pgenheaders.h"
#include "grammar.h"
#include "token.h"
#include "parser.h"

/* Forward references */
static void fixdfa PROTO((grammar *, dfa *));
static void fixstate PROTO((grammar *, dfa *, state *));

void
addaccelerators(g)
       grammar *g;
{
       dfa *d;
       int i;
#ifdef DEBUG
       printf("Adding parser accellerators ...\n");
#endif
       d = g->g_dfa;
       // graminit.c
       // 有42个 DFA，理解应该就是有 42 个状态机
       for (i = g->g_ndfas; --i >= 0; d++) {
               // 对每个状态进行fix
               fixdfa(g, d);
       }
       g->g_accel = 1;
#ifdef DEBUG
       printf("Done.\n");
#endif
}

static void
fixdfa(g, d)
       grammar *g;
       dfa *d;
{
       state *s;
       int j;
       s = d->d_state;
       // 对每个 DFA 状态里面的 子状态进行 fix
       for (j = 0; j < d->d_nstates; j++, s++) {
               fixstate(g, d, s);
       }
}

static void
fixstate(g, d, s)
       grammar *g;
       dfa *d;
       state *s;
{
       // 先去看 graminit.c 的数据结构描述 
       /*
       fix每一个子状态，我理解就是加速，每一个子状态 state包含了基础的（arc信息）有点像类集成，其中arc{lbl下标，arrow指向下一个状态}
       是必须的，然后 Optional accelerators 是可选的，加速后才用到的成员，具体可看 grammar.h 头文件描述，
       accel应该是一个临时变量，记录了所有lables的下一个状态的信息，计算出所有信息之后，再对子状态 state 里面的 Optional accelerators
       成员进行赋值，记录加速信息，这样解析起来就快很多，暂时是这样理解的
       #以下面为例子:
       static arc arcs_0_0[3] = {
       {2, 1},  => 对应 type=4
       {3, 1},  => 对应 type=265 非终结符
       {4, 2},  => 对应 type=275 非终结符
       };
       s_narcs = 3
       */
       arc *a;
       int k;
       int *accel;
       int nl = g->g_ll.ll_nlabels;
       s->s_accept = 0;
       // 这里 根据 grammar 的 lables（91） 数分配一个 int* 是数组用于加速？
       accel = NEW(int, nl);
       for (k = 0; k < nl; k++)
               accel[k] = -1;
       a = s->s_arc;
       for (k = s->s_narcs; --k >= 0; a++) {
               int lbl = a->a_lbl;
               label *l = &g->g_ll.ll_label[lbl];
               int type = l->lb_type;
               if (a->a_arrow >= (1 << 7)) {
                       printf("XXX too many states!\n");
                       continue;
               }
               /*
               对于不同DFA的状态流转，状态节点有3种情况（类型）分别对应下面3个if-else-elseif分支
               1.是非终结符还需要继续展开的 ISNONTERMINAL
               2.是结束符EMPTY，accept
               3.是普通独立状态节点，不需要展开的（比如 关键字 之类等）可以直接流转到下一个状态节点的
               */

               // 这个是判断是否为非终结符，所有非终结符都是 >= 256 的，
               // 参考 gramminit.h #define single_input 256 开始
               // #define NT_OFFSET              256
               // #define ISNONTERMINAL(x)       ((x) >= NT_OFFSET) 
               if (ISNONTERMINAL(type)) {
                       // 如果是非终结符，那么需要找到它对应的 DFA 状态机
                       // 对于非终结符 >= 256 参考 gramminit.h 宏定义，这类型type都是需要深入展开的
                       // 所以这里的逻辑比较复杂，记录 accel 的 value 信息也比较多
                       dfa *d1 = finddfa(g, type);
                       int ibit;
                       if (type - NT_OFFSET >= (1 << 7)) {
                               printf("XXX too high nonterminal number!\n");
                               continue;
                       }
                       // ibit 记录的是位数，循环++， ibit数量跟labels数量一样（91个），也跟accel数组长度一致
                       for (ibit = 0; ibit < g->g_ll.ll_nlabels; ibit++) {
                               // #define BITSPERBYTE    (8*sizeof(BYTE)) 计算1个byte是多少个bit，8
                               // #define BIT2BYTE(ibit) ((ibit) / BITSPERBYTE) 用当前ibit计算出是第几个字节，所以用 ibit / 8
                               // #define BIT2SHIFT(ibit)        ((ibit) % BITSPERBYTE) 计算当前ibit取整字节后的位数，为先mask辅助
                               // #define BIT2MASK(ibit) (1 << BIT2SHIFT(ibit)) 计算当前ibit的掩码mask，简单来讲就是二进制1每循环一次往左移动1位 <<，所以依次是 1 2 4 8 16 32 64
                               // #define testbit(ss, ibit) (((ss)[BIT2BYTE(ibit)] & BIT2MASK(ibit)) != 0)
                               // 需要注意的是 d_first 初始化的值是八进制
                               printf("[%d] %d %d 0x%X(%u)(\\0%o)\n", ibit, BIT2BYTE(ibit), BIT2MASK(ibit), 
                               d1->d_first[BIT2BYTE(ibit)], d1->d_first[BIT2BYTE(ibit)], d1->d_first[BIT2BYTE(ibit)]);
                               //
                               // 看了grammar.h 数据结构描述后，这里的逻辑也就比较清晰了，通过分析每一个子状态state里面的 d_first 开始状态数据
                               // 然后把这个这些开始状态的信息及下一个状态的指向信息（arrow）记录到一个局部数组accel里面，到了函数结束部分整理
                               // 后回写进这个子状态state的 Optional accelerators 成员里面
                               //
                               if (testbit(d1->d_first, ibit)) {
                                       if (accel[ibit] != -1) {
                                               printf("XXX ambiguity!\n"); // ambiguity是歧义的意思
                                       }
                                       // 加速信息由key=>value组成
                                       // key 是ibit对应的是labels 91个元素下标
                                       // value 是对应加速信息，由2个byte字节组成
                                       //   高8位字节记录的是type信息，这里的type是要先减去 NT_OFFSET的
                                       //   低8位字节记录的是下一个状态信息保存在低7位，然后第一位标记为1（代表非终结符）
                                       accel[ibit] = a->a_arrow | (1 << 7) |
                                               ((type - NT_OFFSET) << 8);

                                       //这里针对匹配的才输出labels下标方便查看
                                       printf("lables_idx=[%d][%d] arrow=%d type=%d(%d) => value[%d][%X]\n", 
                                       ibit-1, ibit, a->a_arrow, type, type - NT_OFFSET, accel[ibit], accel[ibit]);

                               }
                       }
               }
               else if (lbl == EMPTY) {
                       // 标记结束状态，个人理解就是一个DFA状态流转正常的情况下，会有一个正常的结束
                       // 而这个 labels下标=0 EMPTY 则是代表整个状态流转的结束标记
                       s->s_accept = 1;
               }
               else if (lbl >= 0 && lbl < nl) {
                       // 这个 else if 逻辑是处理 type < 256 的 token.h 
                       // 参考 graminit.c 关于 static label labels[91] 的注释
                       // 这里理解就是 lbl 元素是独立的，不用像非终结符那样需要再慢慢深入展开分析
                       // 所以这里就直接保存一下个状态指向即可，不需额外记录其他信息了
                       accel[lbl] = a->a_arrow;
               }
       }
       // 因为 nl 是labels的个数，如果剩下的都没加速的（accel[nl-1] == -1），那么就缩减 nl，意思就是nl只计算到有加速的部分
       while (nl > 0 && accel[nl-1] == -1) {
               nl--;
       }
       // k 记录的是第一个有加速的 labels 下标， nl 则是记录了最后一个有加速的 labels下标 
       for (k = 0; k < nl && accel[k] == -1;) {
               k++;
       }
       // [k,nl] 记录的就是有加速的 labels 最小、最大 下标了
       // 把整理好的状态迁移位图信息记录到这个子状态的 Optional accelerators 成员去
       // {s_lower\s_upper\s_accel\s_accept}
       // 这些信息是后面parser分析token的时候用于快速生成语法树的
       if (k < nl) {
               int i;
               s->s_accel = NEW(int, nl-k);
               if (s->s_accel == NULL) {
                       fprintf(stderr, "no mem to add parser accelerators\n");
                       exit(1);
               }
               s->s_lower = k;  // 记录了这个子状态对应最小下标
               s->s_upper = nl; // 记录了这个子状态对应最大下标
               for (i = 0; k < nl; i++, k++) {
                       // 把状态走向位图bit信息记录到这个子状态state的s_accel bit数组里面
                       s->s_accel[i] = accel[k];
               }
       }
       DEL(accel);
       /*
         这个加速逻辑需要结合 parser.c 里面的 addtoken 函数使用才能看懂
         个人理解，就是把状态节点对应迁移的状态节点流转信息用位图的方式记录在该状态的数据结构
         _state 的 Optional accelerators 成员里，当 addtoken分析语法的时候，用语法树的方式
         不断 push、pop 来匹配语法
       */
}
