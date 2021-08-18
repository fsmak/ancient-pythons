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

#include "pgenheaders.h"
#include "grammar.h"
static arc arcs_0_0[3] = {
       {2, 1},
       {3, 1},
       {4, 2},
};
static arc arcs_0_1[1] = {
       {0, 1},
};
static arc arcs_0_2[1] = {
       {2, 1},
};
static state states_0[3] = {
       {3, arcs_0_0},
       {1, arcs_0_1},
       {1, arcs_0_2},
};
static arc arcs_1_0[3] = {
       {2, 0},
       {6, 0},
       {7, 1},
};
static arc arcs_1_1[1] = {
       {0, 1},
};
static state states_1[2] = {
       {3, arcs_1_0},
       {1, arcs_1_1},
};
static arc arcs_2_0[1] = {
       {9, 1},
};
static arc arcs_2_1[1] = {
       {2, 2},
};
static arc arcs_2_2[1] = {
       {0, 2},
};
static state states_2[3] = {
       {1, arcs_2_0},
       {1, arcs_2_1},
       {1, arcs_2_2},
};
static arc arcs_3_0[1] = {
       {9, 1},
};
static arc arcs_3_1[1] = {
       {7, 2},
};
static arc arcs_3_2[1] = {
       {0, 2},
};
static state states_3[3] = {
       {1, arcs_3_0},
       {1, arcs_3_1},
       {1, arcs_3_2},
};
static arc arcs_4_0[1] = {
       {12, 1},
};
static arc arcs_4_1[1] = {
       {13, 2},
};
static arc arcs_4_2[1] = {
       {14, 3},
};
static arc arcs_4_3[1] = {
       {15, 4},
};
static arc arcs_4_4[1] = {
       {16, 5},
};
static arc arcs_4_5[1] = {
       {0, 5},
};
static state states_4[6] = {
       {1, arcs_4_0},
       {1, arcs_4_1},
       {1, arcs_4_2},
       {1, arcs_4_3},
       {1, arcs_4_4},
       {1, arcs_4_5},
};
static arc arcs_5_0[1] = {
       {17, 1},
};
static arc arcs_5_1[2] = {
       {18, 2},
       {19, 3},
};
static arc arcs_5_2[1] = {
       {19, 3},
};
static arc arcs_5_3[1] = {
       {0, 3},
};
static state states_5[4] = {
       {1, arcs_5_0},
       {2, arcs_5_1},
       {1, arcs_5_2},
       {1, arcs_5_3},
};
static arc arcs_6_0[1] = {
       {20, 1},
};
static arc arcs_6_1[2] = {
       {21, 0},
       {0, 1},
};
static state states_6[2] = {
       {1, arcs_6_0},
       {2, arcs_6_1},
};
static arc arcs_7_0[2] = {
       {13, 1},
       {17, 2},
};
static arc arcs_7_1[1] = {
       {0, 1},
};
static arc arcs_7_2[1] = {
       {18, 3},
};
static arc arcs_7_3[1] = {
       {19, 1},
};
static state states_7[4] = {
       {2, arcs_7_0},
       {1, arcs_7_1},
       {1, arcs_7_2},
       {1, arcs_7_3},
};
static arc arcs_8_0[2] = {
       {3, 1},
       {4, 1},
};
static arc arcs_8_1[1] = {
       {0, 1},
};
static state states_8[2] = {
       {2, arcs_8_0},
       {1, arcs_8_1},
};
static arc arcs_9_0[6] = {
       {22, 1},
       {23, 1},
       {24, 1},
       {25, 1},
       {26, 1},
       {27, 1},
};
static arc arcs_9_1[1] = {
       {0, 1},
};
static state states_9[2] = {
       {6, arcs_9_0},
       {1, arcs_9_1},
};
static arc arcs_10_0[1] = {
       {28, 1},
};
static arc arcs_10_1[2] = {
       {29, 0},
       {2, 2},
};
static arc arcs_10_2[1] = {
       {0, 2},
};
static state states_10[3] = {
       {1, arcs_10_0},
       {2, arcs_10_1},
       {1, arcs_10_2},
};
static arc arcs_11_0[1] = {
       {30, 1},
};
static arc arcs_11_1[2] = {
       {31, 2},
       {2, 3},
};
static arc arcs_11_2[2] = {
       {21, 1},
       {2, 3},
};
static arc arcs_11_3[1] = {
       {0, 3},
};
static state states_11[4] = {
       {1, arcs_11_0},
       {2, arcs_11_1},
       {2, arcs_11_2},
       {1, arcs_11_3},
};
static arc arcs_12_0[1] = {
       {32, 1},
};
static arc arcs_12_1[1] = {
       {28, 2},
};
static arc arcs_12_2[1] = {
       {2, 3},
};
static arc arcs_12_3[1] = {
       {0, 3},
};
static state states_12[4] = {
       {1, arcs_12_0},
       {1, arcs_12_1},
       {1, arcs_12_2},
       {1, arcs_12_3},
};
static arc arcs_13_0[1] = {
       {33, 1},
};
static arc arcs_13_1[1] = {
       {2, 2},
};
static arc arcs_13_2[1] = {
       {0, 2},
};
static state states_13[3] = {
       {1, arcs_13_0},
       {1, arcs_13_1},
       {1, arcs_13_2},
};
static arc arcs_14_0[3] = {
       {34, 1},
       {35, 1},
       {36, 1},
};
static arc arcs_14_1[1] = {
       {0, 1},
};
static state states_14[2] = {
       {3, arcs_14_0},
       {1, arcs_14_1},
};
static arc arcs_15_0[1] = {
       {37, 1},
};
static arc arcs_15_1[1] = {
       {2, 2},
};
static arc arcs_15_2[1] = {
       {0, 2},
};
static state states_15[3] = {
       {1, arcs_15_0},
       {1, arcs_15_1},
       {1, arcs_15_2},
};
static arc arcs_16_0[1] = {
       {38, 1},
};
static arc arcs_16_1[2] = {
       {9, 2},
       {2, 3},
};
static arc arcs_16_2[1] = {
       {2, 3},
};
static arc arcs_16_3[1] = {
       {0, 3},
};
static state states_16[4] = {
       {1, arcs_16_0},
       {2, arcs_16_1},
       {1, arcs_16_2},
       {1, arcs_16_3},
};
static arc arcs_17_0[1] = {
       {39, 1},
};
static arc arcs_17_1[1] = {
       {40, 2},
};
static arc arcs_17_2[2] = {
       {21, 3},
       {2, 4},
};
static arc arcs_17_3[1] = {
       {40, 5},
};
static arc arcs_17_4[1] = {
       {0, 4},
};
static arc arcs_17_5[1] = {
       {2, 4},
};
static state states_17[6] = {
       {1, arcs_17_0},
       {1, arcs_17_1},
       {2, arcs_17_2},
       {1, arcs_17_3},
       {1, arcs_17_4},
       {1, arcs_17_5},
};
static arc arcs_18_0[2] = {
       {41, 1},
       {42, 2},
};
static arc arcs_18_1[1] = {
       {13, 3},
};
static arc arcs_18_2[1] = {
       {13, 4},
};
static arc arcs_18_3[2] = {
       {21, 1},
       {2, 5},
};
static arc arcs_18_4[1] = {
       {41, 6},
};
static arc arcs_18_5[1] = {
       {0, 5},
};
static arc arcs_18_6[2] = {
       {43, 7},
       {13, 8},
};
static arc arcs_18_7[1] = {
       {2, 5},
};
static arc arcs_18_8[2] = {
       {21, 9},
       {2, 5},
};
static arc arcs_18_9[1] = {
       {13, 8},
};
static state states_18[10] = {
       {2, arcs_18_0},
       {1, arcs_18_1},
       {1, arcs_18_2},
       {2, arcs_18_3},
       {1, arcs_18_4},
       {1, arcs_18_5},
       {2, arcs_18_6},
       {1, arcs_18_7},
       {2, arcs_18_8},
       {1, arcs_18_9},
};
static arc arcs_19_0[6] = {
       {44, 1},
       {45, 1},
       {46, 1},
       {47, 1},
       {11, 1},
       {48, 1},
};
static arc arcs_19_1[1] = {
       {0, 1},
};
static state states_19[2] = {
       {6, arcs_19_0},
       {1, arcs_19_1},
};
static arc arcs_20_0[1] = {
       {
              49, // Label of this arc 记录所对应 labels 数组的下标， 49对应的是 if
              1   // State where this arc goes to 理解应该是指向下一个状态，对应的是应该看 states_20 第2个， arcs_20_1 对应的是 test
       },
};
static arc arcs_20_1[1] = {
       // if 语法是 if test
       // 所以这个状态就是 test
       {31, 2},  // labels对应下标#31的是 {282, 0}，这个 282 对应的是type， #define test 282， 下一个状态是 arcs_20_2
};
static arc arcs_20_2[1] = {
       // 'if' test ':'
       // 所以这个状态就是 ':'， 下一个状态是 arcs_20_3
       {15, 3},  // lables对应下标#15的是 {11, 0}, 这个 11对应的是type， #define COLON 11 冒号':'
};
static arc arcs_20_3[1] = {
       // 'if' test ':' suite
       // 所以这个状态对应的是 suite， 下一个状态是 arcs_20_4
       {16, 4},  // lables对应下标#16的是 {281, 0}, 这个 281对应的是type， #define suite 281
};
static arc arcs_20_4[3] = {
       // 对于 if 语句 elif、else 是可选的
       {50, 1},  // lables对应下标#50的是 {1, "elif"},  如果是 elif 下一个状态是 arcs_20_1 继续 test
       {51, 5},  // lables对应下标#50的是 {1, "else"},  如果是 else 下一个状态是 arcs_20_5 冒号 ':'
       {0, 4},   // lables对应下标#0的是  {0, "EMPTY"},
};
static arc arcs_20_5[1] = {
       {15, 6},  // lables对应下标#15的是 {11, 0}, 这个 11对应的是type， #define COLON 11 冒号':'
};
static arc arcs_20_6[1] = {
       {16, 7},  // lables对应下标#16的是 {281, 0}, 这个 281对应的是type， #define suite 281
};
static arc arcs_20_7[1] = {
       {0, 7},  // 结束 {0, "EMPTY"},
};
/*
以 if 语句的状态机为例子
{
  276, // Non-terminal this represents 
  "if_stmt", // For printing
  0, // Initial state
  8, // d_nstates 标记下面 DFA 状态数组 个数的数量
  states_20, // Array of states DFA 状态数组（8个元素）
  "\000\000\000\000\000\000\002\000\000\000\000\000" //
},  
// 语法
// 'if' test ':' suite ('elif' test ':' suite)* ['else' ':' suite] 
*/
static state states_20[8] = {
       {
              1,         // int s_narcs 标记下面的 s_acr 数组有多少个元素
              arcs_20_0  // arc *s_acr  acr数组，定义在上面
       },
       {1, arcs_20_1},
       {1, arcs_20_2},
       {1, arcs_20_3},
       {3, arcs_20_4},
       {1, arcs_20_5},
       {1, arcs_20_6},
       {1, arcs_20_7},
};
static arc arcs_21_0[1] = {
       {52, 1},
};
static arc arcs_21_1[1] = {
       {31, 2},
};
static arc arcs_21_2[1] = {
       {15, 3},
};
static arc arcs_21_3[1] = {
       {16, 4},
};
static arc arcs_21_4[2] = {
       {51, 5},
       {0, 4},
};
static arc arcs_21_5[1] = {
       {15, 6},
};
static arc arcs_21_6[1] = {
       {16, 7},
};
static arc arcs_21_7[1] = {
       {0, 7},
};
static state states_21[8] = {
       {1, arcs_21_0},
       {1, arcs_21_1},
       {1, arcs_21_2},
       {1, arcs_21_3},
       {2, arcs_21_4},
       {1, arcs_21_5},
       {1, arcs_21_6},
       {1, arcs_21_7},
};
static arc arcs_22_0[1] = {
       {53, 1},
};
static arc arcs_22_1[1] = {
       {28, 2},
};
static arc arcs_22_2[1] = {
       {54, 3},
};
static arc arcs_22_3[1] = {
       {28, 4},
};
static arc arcs_22_4[1] = {
       {15, 5},
};
static arc arcs_22_5[1] = {
       {16, 6},
};
static arc arcs_22_6[2] = {
       {51, 7},
       {0, 6},
};
static arc arcs_22_7[1] = {
       {15, 8},
};
static arc arcs_22_8[1] = {
       {16, 9},
};
static arc arcs_22_9[1] = {
       {0, 9},
};
static state states_22[10] = {
       {1, arcs_22_0},
       {1, arcs_22_1},
       {1, arcs_22_2},
       {1, arcs_22_3},
       {1, arcs_22_4},
       {1, arcs_22_5},
       {2, arcs_22_6},
       {1, arcs_22_7},
       {1, arcs_22_8},
       {1, arcs_22_9},
};
static arc arcs_23_0[1] = {
       {55, 1},
};
static arc arcs_23_1[1] = {
       {15, 2},
};
static arc arcs_23_2[1] = {
       {16, 3},
};
static arc arcs_23_3[3] = {
       {56, 1},
       {57, 4},
       {0, 3},
};
static arc arcs_23_4[1] = {
       {15, 5},
};
static arc arcs_23_5[1] = {
       {16, 6},
};
static arc arcs_23_6[1] = {
       {0, 6},
};
static state states_23[7] = {
       {1, arcs_23_0},
       {1, arcs_23_1},
       {1, arcs_23_2},
       {3, arcs_23_3},
       {1, arcs_23_4},
       {1, arcs_23_5},
       {1, arcs_23_6},
};
static arc arcs_24_0[1] = {
       {58, 1},
};
static arc arcs_24_1[2] = {
       {40, 2},
       {0, 1},
};
static arc arcs_24_2[2] = {
       {21, 3},
       {0, 2},
};
static arc arcs_24_3[1] = {
       {40, 4},
};
static arc arcs_24_4[1] = {
       {0, 4},
};
static state states_24[5] = {
       {1, arcs_24_0},
       {2, arcs_24_1},
       {2, arcs_24_2},
       {1, arcs_24_3},
       {1, arcs_24_4},
};
static arc arcs_25_0[2] = {
       {3, 1},
       {2, 2},
};
static arc arcs_25_1[1] = {
       {0, 1},
};
static arc arcs_25_2[1] = {
       {59, 3},
};
static arc arcs_25_3[2] = {
       {2, 3},
       {6, 4},
};
static arc arcs_25_4[3] = {
       {6, 4},
       {2, 4},
       {60, 1},
};
static state states_25[5] = {
       {2, arcs_25_0},
       {1, arcs_25_1},
       {1, arcs_25_2},
       {2, arcs_25_3},
       {3, arcs_25_4},
};
static arc arcs_26_0[1] = {
       {61, 1},
};
static arc arcs_26_1[2] = {
       {62, 0},
       {0, 1},
};
static state states_26[2] = {
       {1, arcs_26_0},
       {2, arcs_26_1},
};
static arc arcs_27_0[1] = {
       {63, 1},
};
static arc arcs_27_1[2] = {
       {64, 0},
       {0, 1},
};
static state states_27[2] = {
       {1, arcs_27_0},
       {2, arcs_27_1},
};
static arc arcs_28_0[2] = {
       {65, 1},
       {66, 2},
};
static arc arcs_28_1[1] = {
       {63, 2},
};
static arc arcs_28_2[1] = {
       {0, 2},
};
static state states_28[3] = {
       {2, arcs_28_0},
       {1, arcs_28_1},
       {1, arcs_28_2},
};
static arc arcs_29_0[1] = {
       {40, 1},
};
static arc arcs_29_1[2] = {
       {67, 0},
       {0, 1},
};
static state states_29[2] = {
       {1, arcs_29_0},
       {2, arcs_29_1},
};
static arc arcs_30_0[6] = {
       {68, 1},
       {69, 2},
       {29, 3},
       {54, 3},
       {65, 4},
       {70, 5},
};
static arc arcs_30_1[3] = {
       {29, 3},
       {69, 3},
       {0, 1},
};
static arc arcs_30_2[2] = {
       {29, 3},
       {0, 2},
};
static arc arcs_30_3[1] = {
       {0, 3},
};
static arc arcs_30_4[1] = {
       {54, 3},
};
static arc arcs_30_5[2] = {
       {65, 3},
       {0, 5},
};
static state states_30[6] = {
       {6, arcs_30_0},
       {3, arcs_30_1},
       {2, arcs_30_2},
       {1, arcs_30_3},
       {1, arcs_30_4},
       {2, arcs_30_5},
};
static arc arcs_31_0[1] = {
       {71, 1},
};
static arc arcs_31_1[3] = {
       {72, 0},
       {73, 0},
       {0, 1},
};
static state states_31[2] = {
       {1, arcs_31_0},
       {3, arcs_31_1},
};
static arc arcs_32_0[1] = {
       {74, 1},
};
static arc arcs_32_1[4] = {
       {43, 0},
       {75, 0},
       {76, 0},
       {0, 1},
};
static state states_32[2] = {
       {1, arcs_32_0},
       {4, arcs_32_1},
};
static arc arcs_33_0[3] = {
       {72, 1},
       {73, 1},
       {77, 2},
};
static arc arcs_33_1[1] = {
       {74, 3},
};
static arc arcs_33_2[2] = {
       {78, 2},
       {0, 2},
};
static arc arcs_33_3[1] = {
       {0, 3},
};
static state states_33[4] = {
       {3, arcs_33_0},
       {1, arcs_33_1},
       {2, arcs_33_2},
       {1, arcs_33_3},
};
static arc arcs_34_0[7] = {
       {17, 1},
       {79, 2},
       {81, 3},
       {83, 4},
       {13, 5},
       {84, 5},
       {85, 5},
};
static arc arcs_34_1[2] = {
       {9, 6},
       {19, 5},
};
static arc arcs_34_2[2] = {
       {9, 7},
       {80, 5},
};
static arc arcs_34_3[1] = {
       {82, 5},
};
static arc arcs_34_4[1] = {
       {9, 8},
};
static arc arcs_34_5[1] = {
       {0, 5},
};
static arc arcs_34_6[1] = {
       {19, 5},
};
static arc arcs_34_7[1] = {
       {80, 5},
};
static arc arcs_34_8[1] = {
       {83, 5},
};
static state states_34[9] = {
       {7, arcs_34_0},
       {2, arcs_34_1},
       {2, arcs_34_2},
       {1, arcs_34_3},
       {1, arcs_34_4},
       {1, arcs_34_5},
       {1, arcs_34_6},
       {1, arcs_34_7},
       {1, arcs_34_8},
};
static arc arcs_35_0[3] = {
       {17, 1},
       {79, 2},
       {87, 3},
};
static arc arcs_35_1[2] = {
       {9, 4},
       {19, 5},
};
static arc arcs_35_2[1] = {
       {86, 6},
};
static arc arcs_35_3[1] = {
       {13, 5},
};
static arc arcs_35_4[1] = {
       {19, 5},
};
static arc arcs_35_5[1] = {
       {0, 5},
};
static arc arcs_35_6[1] = {
       {80, 5},
};
static state states_35[7] = {
       {3, arcs_35_0},
       {2, arcs_35_1},
       {1, arcs_35_2},
       {1, arcs_35_3},
       {1, arcs_35_4},
       {1, arcs_35_5},
       {1, arcs_35_6},
};
static arc arcs_36_0[2] = {
       {40, 1},
       {15, 2},
};
static arc arcs_36_1[2] = {
       {15, 2},
       {0, 1},
};
static arc arcs_36_2[2] = {
       {40, 3},
       {0, 2},
};
static arc arcs_36_3[1] = {
       {0, 3},
};
static state states_36[4] = {
       {2, arcs_36_0},
       {2, arcs_36_1},
       {2, arcs_36_2},
       {1, arcs_36_3},
};
static arc arcs_37_0[1] = {
       {40, 1},
};
static arc arcs_37_1[2] = {
       {21, 2},
       {0, 1},
};
static arc arcs_37_2[2] = {
       {40, 1},
       {0, 2},
};
static state states_37[3] = {
       {1, arcs_37_0},
       {2, arcs_37_1},
       {2, arcs_37_2},
};
static arc arcs_38_0[1] = {
       {31, 1},
};
static arc arcs_38_1[2] = {
       {21, 2},
       {0, 1},
};
static arc arcs_38_2[2] = {
       {31, 1},
       {0, 2},
};
static state states_38[3] = {
       {1, arcs_38_0},
       {2, arcs_38_1},
       {2, arcs_38_2},
};
static arc arcs_39_0[1] = {
       {88, 1},
};
static arc arcs_39_1[1] = {
       {13, 2},
};
static arc arcs_39_2[1] = {
       {14, 3},
};
static arc arcs_39_3[2] = {
       {29, 4},
       {15, 5},
};
static arc arcs_39_4[1] = {
       {89, 6},
};
static arc arcs_39_5[1] = {
       {16, 7},
};
static arc arcs_39_6[1] = {
       {15, 5},
};
static arc arcs_39_7[1] = {
       {0, 7},
};
static state states_39[8] = {
       {1, arcs_39_0},
       {1, arcs_39_1},
       {1, arcs_39_2},
       {2, arcs_39_3},
       {1, arcs_39_4},
       {1, arcs_39_5},
       {1, arcs_39_6},
       {1, arcs_39_7},
};
static arc arcs_40_0[1] = {
       {77, 1},
};
static arc arcs_40_1[1] = {
       {90, 2},
};
static arc arcs_40_2[2] = {
       {21, 0},
       {0, 2},
};
static state states_40[3] = {
       {1, arcs_40_0},
       {1, arcs_40_1},
       {2, arcs_40_2},
};
static arc arcs_41_0[1] = {
       {17, 1},
};
static arc arcs_41_1[2] = {
       {9, 2},
       {19, 3},
};
static arc arcs_41_2[1] = {
       {19, 3},
};
static arc arcs_41_3[1] = {
       {0, 3},
};
static state states_41[4] = {
       {1, arcs_41_0},
       {2, arcs_41_1},
       {1, arcs_41_2},
       {1, arcs_41_3},
};
static dfa dfas[42] = {
       {256, "single_input", 0, 3, states_0,
        "\004\060\002\100\343\006\262\000\000\203\072\001"},
       {257, "file_input", 0, 2, states_1,
        "\204\060\002\100\343\006\262\000\000\203\072\001"},
       {258, "expr_input", 0, 3, states_2,
        "\000\040\002\000\000\000\000\000\002\203\072\000"},
       {259, "eval_input", 0, 3, states_3,
        "\000\040\002\000\000\000\000\000\002\203\072\000"},
       {260, "funcdef", 0, 6, states_4,
        "\000\020\000\000\000\000\000\000\000\000\000\000"},
       {261, "parameters", 0, 4, states_5,
        "\000\000\002\000\000\000\000\000\000\000\000\000"},
       {262, "fplist", 0, 2, states_6,
        "\000\040\002\000\000\000\000\000\000\000\000\000"},
       {263, "fpdef", 0, 4, states_7,
        "\000\040\002\000\000\000\000\000\000\000\000\000"},
       {264, "stmt", 0, 2, states_8,
        "\000\060\002\100\343\006\262\000\000\203\072\001"},
       {
              265, 
              "simple_stmt", 
              0, 
              2, 
              states_9,
              //下面这串数字是八进制
              //  0  32   2  64 227   6   0   0   0 131  58   0 (十进制)
              "\000\040\002\100\343\006\000\000\000\203\072\000"
        },
       {266, "expr_stmt", 0, 3, states_10,
        "\000\040\002\000\000\000\000\000\000\203\072\000"},
       {267, "print_stmt", 0, 4, states_11,
        "\000\000\000\100\000\000\000\000\000\000\000\000"},
       {268, "del_stmt", 0, 4, states_12,
        "\000\000\000\000\001\000\000\000\000\000\000\000"},
       {269, "pass_stmt", 0, 3, states_13,
        "\000\000\000\000\002\000\000\000\000\000\000\000"},
       {270, "flow_stmt", 0, 2, states_14,
        "\000\000\000\000\340\000\000\000\000\000\000\000"},
       {271, "break_stmt", 0, 3, states_15,
        "\000\000\000\000\040\000\000\000\000\000\000\000"},
       {272, "return_stmt", 0, 4, states_16,
        "\000\000\000\000\100\000\000\000\000\000\000\000"},
       {273, "raise_stmt", 0, 6, states_17,
        "\000\000\000\000\200\000\000\000\000\000\000\000"},
       {274, "import_stmt", 0, 10, states_18,
        "\000\000\000\000\000\006\000\000\000\000\000\000"},
       {275, "compound_stmt", 0, 2, states_19,
        "\000\020\000\000\000\000\262\000\000\000\000\001"},
       {
              276, 
              "if_stmt", 
              0, 
              8, 
              states_20,
              //下面这串数字是八进制
              // #0  #1  #2  #3  #4  #5  #6  #7  #8  #9 #10 #11
              "\000\000\000\000\000\000\002\000\000\000\000\000"
              // 可以看到唯一一个非0的是 #6 字节的第二位，那么对应 labels下标是
              // 6 * 8 + 2 = 50 第50个元素也就是刚好对应下标是#49，正是if
              //
              //重点：
              // 这串八进制数字一共有12个字节，12 * 8 = 96 bit
              // 每一个 bit 对应的是 lables 的下标，lables一共有 91 个，所以就需要12个字节
              // 这个成员名称叫 d_first，我理解就是对应的这个 DFA 开始的状态，假如开始状态
              // 有多种情况的，那么对应会有多个非0的bit，假如只有一个开始状态的，比如 if 语句
              // 肯定是 if 关键字开头，有且只能有一个开始状态，所以就只有一个非0的bit
              // 对比起其他如 suite 是逻辑的，那么对应的开始状态就有多种情况了
        },
       {277, "while_stmt", 0, 8, states_21,
        "\000\000\000\000\000\000\020\000\000\000\000\000"},
       {278, "for_stmt", 0, 10, states_22,
        "\000\000\000\000\000\000\040\000\000\000\000\000"},
       {279, "try_stmt", 0, 7, states_23,
        "\000\000\000\000\000\000\200\000\000\000\000\000"},
       {280, "except_clause", 0, 5, states_24,
        "\000\000\000\000\000\000\000\004\000\000\000\000"},
       {281, "suite", 0, 5, states_25,
        "\004\040\002\100\343\006\000\000\000\203\072\000"},
       {282, "test", 0, 2, states_26,
        "\000\040\002\000\000\000\000\000\002\203\072\000"},
       {283, "and_test", 0, 2, states_27,
        "\000\040\002\000\000\000\000\000\002\203\072\000"},
       {284, "not_test", 0, 3, states_28,
        "\000\040\002\000\000\000\000\000\002\203\072\000"},
       {285, "comparison", 0, 2, states_29,
        "\000\040\002\000\000\000\000\000\000\203\072\000"},
       {286, "comp_op", 0, 6, states_30,
        "\000\000\000\040\000\000\100\000\162\000\000\000"},
       {287, "expr", 0, 2, states_31,
        "\000\040\002\000\000\000\000\000\000\203\072\000"},
       {288, "term", 0, 2, states_32,
        "\000\040\002\000\000\000\000\000\000\203\072\000"},
       {289, "factor", 0, 4, states_33,
        "\000\040\002\000\000\000\000\000\000\203\072\000"},
       {290, "atom", 0, 9, states_34,
        "\000\040\002\000\000\000\000\000\000\200\072\000"},
       {291, "trailer", 0, 7, states_35,
        "\000\000\002\000\000\000\000\000\000\200\200\000"},
       {292, "subscript", 0, 4, states_36,
        "\000\240\002\000\000\000\000\000\000\203\072\000"},
       {293, "exprlist", 0, 3, states_37,
        "\000\040\002\000\000\000\000\000\000\203\072\000"},
       {294, "testlist", 0, 3, states_38,
        "\000\040\002\000\000\000\000\000\002\203\072\000"},
       {295, "classdef", 0, 8, states_39,
        "\000\000\000\000\000\000\000\000\000\000\000\001"},
       {296, "baselist", 0, 3, states_40,
        "\000\040\002\000\000\000\000\000\000\200\072\000"},
       {297, "arguments", 0, 4, states_41,
        "\000\000\002\000\000\000\000\000\000\000\000\000"},
};
/*
typedef struct _label {
       // 对应的是type类型，type分两类：
       1类是小于 NT_OFFSET 256 的，参考 token.h 各个宏定义，这类是单独一个元素的，解释语法树的时候不需要再深入，直接走下一个状态即可
       1类是大于等于NT_OFFSET 256 的，参考 graminit.h 各个宏定义，这类是非终结符，解释语法树的时候需要深入进去
       对于type的影响逻辑，可以参考acceler.c fixstate 函数
       int     lb_type;  
       char    *lb_str;
} label;
*/
static label labels[91] = {
       {0, "EMPTY"},  // 下标 #0  标记结束
       {256, 0},
       {4, 0},
       {265, 0},
       {275, 0},
       {257, 0},
       {264, 0},
       {0, 0},
       {258, 0},
       {294, 0},
       {259, 0},     //  下标 #10
       {260, 0},
       {1, "def"},
       {1, 0},
       {261, 0},
       {11, 0},
       {281, 0},
       {7, 0},
       {262, 0},
       {8, 0},
       {263, 0},     //  下标 #20
       {12, 0},
       {266, 0},
       {267, 0},
       {269, 0},
       {268, 0},
       {270, 0},
       {274, 0},
       {293, 0},
       {22, 0},
       {1, "print"},  // 下标#30
       {282, 0},      // 下标#31 #define test 282 对应的是 test，没有可显示的字符串
       {1, "del"},
       {1, "pass"},
       {271, 0},
       {272, 0},
       {273, 0},
       {1, "break"},
       {1, "return"},
       {1, "raise"},
       {287, 0},
       {1, "import"},
       {1, "from"},
       {16, 0},
       {276, 0},
       {277, 0},
       {278, 0},
       {279, 0},
       {295, 0},   // 下标48
       {1, "if"},  // 下标#49 以if语句为例 { int lb_type; 类型 | char *lb_str; 字符串if } */
       {1, "elif"},
       {1, "else"},
       {1, "while"},
       {1, "for"},
       {1, "in"},
       {1, "try"},
       {280, 0},
       {1, "finally"},
       {1, "except"},
       {5, 0},
       {6, 0},
       {283, 0},
       {1, "or"},
       {284, 0},
       {1, "and"},
       {1, "not"},
       {285, 0},
       {286, 0},
       {20, 0},
       {21, 0},
       {1, "is"},
       {288, 0},
       {14, 0},
       {15, 0},
       {289, 0},
       {17, 0},
       {24, 0},
       {290, 0},
       {291, 0},
       {9, 0},
       {10, 0},
       {26, 0},
       {27, 0},
       {25, 0},
       {2, 0},
       {3, 0},
       {292, 0},
       {23, 0},
       {1, "class"},
       {296, 0},
       {297, 0},
};
grammar gram = {
       42,
       dfas,
       {91, labels},
       256
};
