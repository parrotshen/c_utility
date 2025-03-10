#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "generate.h"
#include "encode.h"
#include "reedsolomon.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

int g_L[40][6] = {
/* Version and EC level L:
*      Total data codewords
*      |    EC codewords per block
*      |    |    Blocks in group 1
*      |    |    |    Data codewords in group 1
*      |    |    |    |    Blocks in group 2
*      |    |    |    |    |   Data codewords in group 2
*      |    |    |    |    |   |   */
    { 19,   7,   1,  19,   0,  0   },  // (19*1) = 19
    { 34,   10,  1,  34,   0,  0   },  // (34*1) = 34
    { 55,   15,  1,  55,   0,  0   },  // (55*1) = 55
    { 80,   20,  1,  80,   0,  0   },  // (80*1) = 80
    { 108,  26,  1,  108,  0,  0   },  // (108*1) = 108
    { 136,  18,  2,  68,   0,  0   },  // (68*2) = 136
    { 156,  20,  2,  78,   0,  0   },  // (78*2) = 156
    { 194,  24,  2,  97,   0,  0   },  // (97*2) = 194
    { 232,  30,  2,  116,  0,  0   },  // (116*2) = 232
    { 274,  18,  2,  68,   2,  69  },  // (68*2) + (69*2) = 274
    { 324,  20,  4,  81,   0,  0   },  // (81*4) = 324
    { 370,  24,  2,  92,   2,  93  },  // (92*2) + (93*2) = 370
    { 428,  26,  4,  107,  0,  0   },  // (107*4) = 428
    { 461,  30,  3,  115,  1,  116 },  // (115*3) + (116*1) = 461
    { 523,  22,  5,  87,   1,  88  },  // (87*5) + (88*1) = 523
    { 589,  24,  5,  98,   1,  99  },  // (98*5) + (99*1) = 589
    { 647,  28,  1,  107,  5,  108 },  // (107*1) + (108*5) = 647
    { 721,  30,  5,  120,  1,  121 },  // (120*5) + (121*1) = 721
    { 795,  28,  3,  113,  4,  114 },  // (113*3) + (114*4) = 795
    { 861,  28,  3,  107,  5,  108 },  // (107*3) + (108*5) = 861
    { 932,  28,  4,  116,  4,  117 },  // (116*4) + (117*4) = 932
    { 1006, 28,  2,  111,  7,  112 },  // (111*2) + (112*7) = 1006
    { 1094, 30,  4,  121,  5,  122 },  // (121*4) + (122*5) = 1094
    { 1174, 30,  6,  117,  4,  118 },  // (117*6) + (118*4) = 1174
    { 1276, 26,  8,  106,  4,  107 },  // (106*8) + (107*4) = 1276
    { 1370, 28,  10, 114,  2,  115 },  // (114*10) + (115*2) = 1370
    { 1468, 30,  8,  122,  4,  123 },  // (122*8) + (123*4) = 1468
    { 1531, 30,  3,  117,  10, 118 },  // (117*3) + (118*10) = 1531
    { 1631, 30,  7,  116,  7,  117 },  // (116*7) + (117*7) = 1631
    { 1735, 30,  5,  115,  10, 116 },  // (115*5) + (116*10) = 1735
    { 1843, 30,  13, 115,  3,  116 },  // (115*13) + (116*3) = 1843
    { 1955, 30,  17, 115,  0,  0   },  // (115*17) = 1955
    { 2071, 30,  17, 115,  1,  116 },  // (115*17) + (116*1) = 2071
    { 2191, 30,  13, 115,  6,  116 },  // (115*13) + (116*6) = 2191
    { 2306, 30,  12, 121,  7,  122 },  // (121*12) + (122*7) = 2306
    { 2434, 30,  6,  121,  14, 122 },  // (121*6) + (122*14) = 2434
    { 2566, 30,  17, 122,  4,  123 },  // (122*17) + (123*4) = 2566
    { 2702, 30,  4,  122,  18, 123 },  // (122*4) + (123*18) = 2702
    { 2812, 30,  20, 117,  4,  118 },  // (117*20) + (118*4) = 2812
    { 2956, 30,  19, 118,  6,  119 }   // (118*19) + (119*6) = 2956
};

int g_M[40][6] = {
/* Version and EC level M:
*      Total data codewords
*      |     EC codewords per block
*      |     |   Blocks in group 1
*      |     |   |    Data codewords in group 1
*      |     |   |    |   Blocks in group 2
*      |     |   |    |   |    Data codewords in group 2
*      |     |   |    |   |    |  */
    { 16,   10,  1,  16,  0,   0  },  // (16*1) = 16
    { 28,   16,  1,  28,  0,   0  },  // (28*1) = 28
    { 44,   26,  1,  44,  0,   0  },  // (44*1) = 44
    { 64,   18,  2,  32,  0,   0  },  // (32*2) = 64
    { 86,   24,  2,  43,  0,   0  },  // (43*2) = 86
    { 108,  16,  4,  27,  0,   0  },  // (27*4) = 108
    { 124,  18,  4,  31,  0,   0  },  // (31*4) = 124
    { 154,  22,  2,  38,  2,   39 },  // (38*2) + (39*2) = 154
    { 182,  22,  3,  36,  2,   37 },  // (36*3) + (37*2) = 182
    { 216,  26,  4,  43,  1,   44 },  // (43*4) + (44*1) = 216
    { 254,  30,  1,  50,  4,   51 },  // (50*1) + (51*4) = 254
    { 290,  22,  6,  36,  2,   37 },  // (36*6) + (37*2) = 290
    { 334,  22,  8,  37,  1,   38 },  // (37*8) + (38*1) = 334
    { 365,  24,  4,  40,  5,   41 },  // (40*4) + (41*5) = 365
    { 415,  24,  5,  41,  5,   42 },  // (41*5) + (42*5) = 415
    { 453,  28,  7,  45,  3,   46 },  // (45*7) + (46*3) = 453
    { 507,  28,  10, 46,  1,   47 },  // (46*10) + (47*1) = 507
    { 563,  26,  9,  43,  4,   44 },  // (43*9) + (44*4) = 563
    { 627,  26,  3,  44,  11,  45 },  // (44*3) + (45*11) = 627
    { 669,  26,  3,  41,  13,  42 },  // (41*3) + (42*13) = 669
    { 714,  26,  17, 42,  0,   0  },  // (42*17) = 714
    { 782,  28,  17, 46,  0,   0  },  // (46*17) = 782
    { 860,  28,  4,  47,  14,  48 },  // (47*4) + (48*14) = 860
    { 914,  28,  6,  45,  14,  46 },  // (45*6) + (46*14) = 914
    { 1000, 28,  8,  47,  13,  48 },  // (47*8) + (48*13) = 1000
    { 1062, 28,  19, 46,  4,   47 },  // (46*19) + (47*4) = 1062
    { 1128, 28,  22, 45,  3,   46 },  // (45*22) + (46*3) = 1128
    { 1193, 28,  3,  45,  23,  46 },  // (45*3) + (46*23) = 1193
    { 1267, 28,  21, 45,  7,   46 },  // (45*21) + (46*7) = 1267
    { 1373, 28,  19, 47,  10,  48 },  // (47*19) + (48*10) = 1373
    { 1455, 28,  2,  46,  29,  47 },  // (46*2) + (47*29) = 1455
    { 1541, 28,  10, 46,  23,  47 },  // (46*10) + (47*23) = 1541
    { 1631, 28,  14, 46,  21,  47 },  // (46*14) + (47*21) = 1631
    { 1725, 28,  14, 46,  23,  47 },  // (46*14) + (47*23) = 1725
    { 1812, 28,  12, 47,  26,  48 },  // (47*12) + (48*26) = 1812
    { 1914, 28,  6,  47,  34,  48 },  // (47*6) + (48*34) = 1914
    { 1992, 28,  29, 46,  14,  47 },  // (46*29) + (47*14) = 1992
    { 2102, 28,  13, 46,  32,  47 },  // (46*13) + (47*32) = 2102
    { 2216, 28,  40, 47,  7,   48 },  // (47*40) + (48*7) = 2216
    { 2334, 28,  18, 47,  31,  48 }   // (47*18) + (48*31) = 2334
};

int g_Q[40][6] = {
/* Version and EC level Q:
*      Total data codewords
*      |     EC codewords per block
*      |     |   Blocks in group 1
*      |     |   |     Data codewords in group 1
*      |     |   |     |   Blocks in group 2
*      |     |   |     |   |    Data codewords in group 2
*      |     |   |     |   |    |  */
    { 13,   13,  1,   13,  0,   0  },  // (13*1) = 13
    { 22,   22,  1,   22,  0,   0  },  // (22*1) = 22
    { 34,   18,  2,   17,  0,   0  },  // (17*2) = 34
    { 48,   26,  2,   24,  0,   0  },  // (24*2) = 48
    { 62,   18,  2,   15,  2,   16 },  // (15*2) + (16*2) = 62
    { 76,   24,  4,   19,  0,   0  },  // (19*4) = 76
    { 88,   18,  2,   14,  4,   15 },  // (14*2) + (15*4) = 88
    { 110,  22,  4,   18,  2,   19 },  // (18*4) + (19*2) = 110
    { 132,  20,  4,   16,  4,   17 },  // (16*4) + (17*4) = 132
    { 154,  24,  6,   19,  2,   20 },  // (19*6) + (20*2) = 154
    { 180,  28,  4,   22,  4,   23 },  // (22*4) + (23*4) = 180
    { 206,  26,  4,   20,  6,   21 },  // (20*4) + (21*6) = 206
    { 244,  24,  8,   20,  4,   21 },  // (20*8) + (21*4) = 244
    { 261,  20,  11,  16,  5,   17 },  // (16*11) + (17*5) = 261
    { 295,  30,  5,   24,  7,   25 },  // (24*5) + (25*7) = 295
    { 325,  24,  15,  19,  2,   20 },  // (19*15) + (20*2) = 325
    { 367,  28,  1,   22,  15,  23 },  // (22*1) + (23*15) = 367
    { 397,  28,  17,  22,  1,   23 },  // (22*17) + (23*1) = 397
    { 445,  26,  17,  21,  4,   22 },  // (21*17) + (22*4) = 445
    { 485,  30,  15,  24,  5,   25 },  // (24*15) + (25*5) = 485
    { 512,  28,  17,  22,  6,   23 },  // (22*17) + (23*6) = 512
    { 568,  30,  7,   24,  16,  25 },  // (24*7) + (25*16) = 568
    { 614,  30,  11,  24,  14,  25 },  // (24*11) + (25*14) = 614
    { 664,  30,  11,  24,  16,  25 },  // (24*11) + (25*16) = 664
    { 718,  30,  7,   24,  22,  25 },  // (24*7) + (25*22) = 718
    { 754,  28,  28,  22,  6,   23 },  // (22*28) + (23*6) = 754
    { 808,  30,  8,   23,  26,  24 },  // (23*8) + (24*26) = 808
    { 871,  30,  4,   24,  31,  25 },  // (24*4) + (25*31) = 871
    { 911,  30,  1,   23,  37,  24 },  // (23*1) + (24*37) = 911
    { 985,  30,  15,  24,  25,  25 },  // (24*15) + (25*25) = 985
    { 1033, 30,  42,  24,  1,   25 },  // (24*42) + (25*1) = 1033
    { 1115, 30,  10,  24,  35,  25 },  // (24*10) + (25*35) = 1115
    { 1171, 30,  29,  24,  19,  25 },  // (24*29) + (25*19) = 1171
    { 1231, 30,  44,  24,  7,   25 },  // (24*44) + (25*7) = 1231
    { 1286, 30,  39,  24,  14,  25 },  // (24*39) + (25*14) = 1286
    { 1354, 30,  46,  24,  10,  25 },  // (24*46) + (25*10) = 1354
    { 1426, 30,  49,  24,  10,  25 },  // (24*49) + (25*10) = 1426
    { 1502, 30,  48,  24,  14,  25 },  // (24*48) + (25*14) = 1502
    { 1582, 30,  43,  24,  22,  25 },  // (24*43) + (25*22) = 1582
    { 1666, 30,  34,  24,  34,  25 }   // (24*34) + (25*34) = 1666
};

int g_H[40][6] = {
/* Version and EC level H:
*     Total data codewords
*     |     EC codewords per block
*     |     |    Blocks in group 1
*     |     |    |    Data codewords in group 1
*     |     |    |    |    Blocks in group 2
*     |     |    |    |    |    Data codewords in group 2
*     |     |    |    |    |    |  */
    { 9,    17,  1,   9,   0,   0  },  // (9*1) = 9
    { 16,   28,  1,   16,  0,   0  },  // (16*1) = 16
    { 26,   22,  2,   13,  0,   0  },  // (13*2) = 26
    { 36,   16,  4,   9,   0,   0  },  // (9*4) = 36
    { 46,   22,  2,   11,  2,   12 },  // (11*2) + (12*2) = 46
    { 60,   28,  4,   15,  0,   0  },  // (15*4) = 60
    { 66,   26,  4,   13,  1,   14 },  // (13*4) + (14*1) = 66
    { 86,   26,  4,   14,  2,   15 },  // (14*4) + (15*2) = 86
    { 100,  24,  4,   12,  4,   13 },  // (12*4) + (13*4) = 100
    { 122,  28,  6,   15,  2,   16 },  // (15*6) + (16*2) = 122
    { 140,  24,  3,   12,  8,   13 },  // (12*3) + (13*8) = 140
    { 158,  28,  7,   14,  4,   15 },  // (14*7) + (15*4) = 158
    { 180,  22,  12,  11,  4,   12 },  // (11*12) + (12*4) = 180
    { 197,  24,  11,  12,  5,   13 },  // (12*11) + (13*5) = 197
    { 223,  24,  11,  12,  7,   13 },  // (12*11) + (13*7) = 223
    { 253,  30,  3,   15,  13,  16 },  // (15*3) + (16*13) = 253
    { 283,  28,  2,   14,  17,  15 },  // (14*2) + (15*17) = 283
    { 313,  28,  2,   14,  19,  15 },  // (14*2) + (15*19) = 313
    { 341,  26,  9,   13,  16,  14 },  // (13*9) + (14*16) = 341
    { 385,  28,  15,  15,  10,  16 },  // (15*15) + (16*10) = 385
    { 406,  30,  19,  16,  6,   17 },  // (16*19) + (17*6) = 406
    { 442,  24,  34,  13,  0,   0  },  // (13*34) = 442
    { 464,  30,  16,  15,  14,  16 },  // (15*16) + (16*14) = 464
    { 514,  30,  30,  16,  2,   17 },  // (16*30) + (17*2) = 514
    { 538,  30,  22,  15,  13,  16 },  // (15*22) + (16*13) = 538
    { 596,  30,  33,  16,  4,   17 },  // (16*33) + (17*4) = 596
    { 628,  30,  12,  15,  28,  16 },  // (15*12) + (16*28) = 628
    { 661,  30,  11,  15,  31,  16 },  // (15*11) + (16*31) = 661
    { 701,  30,  19,  15,  26,  16 },  // (15*19) + (16*26) = 701
    { 745,  30,  23,  15,  25,  16 },  // (15*23) + (16*25) = 745
    { 793,  30,  23,  15,  28,  16 },  // (15*23) + (16*28) = 793
    { 845,  30,  19,  15,  35,  16 },  // (15*19) + (16*35) = 845
    { 901,  30,  11,  15,  46,  16 },  // (15*11) + (16*46) = 901
    { 961,  30,  59,  16,  1,   17 },  // (16*59) + (17*1) = 961
    { 986,  30,  22,  15,  41,  16 },  // (15*22) + (16*41) = 986
    { 1054, 30,  2,   15,  64,  16 },  // (15*2) + (16*64) = 1054
    { 1096, 30,  24,  15,  46,  16 },  // (15*24) + (16*46) = 1096
    { 1142, 30,  42,  15,  32,  16 },  // (15*42) + (16*32) = 1142
    { 1222, 30,  10,  15,  67,  16 },  // (15*10) + (16*67) = 1222
    { 1276, 30,  20,  15,  61,  16 }   // (15*20) + (16*61) = 1276
};

int g_rem[40] = {
    0, 7, 7, 7, 7, 7, 0, 0, 0, 0,
    0, 0, 0, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 3, 3, 3,
    3, 3, 3, 3, 0, 0, 0, 0, 0, 0
};

int g_bits[29656]; /* max. bits 29656 = (3706 * 8) + 8 */
int g_totLen = 0;
int g_datLen = 0;
int g_eccLen = 0;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void qr_show_codewords(void)
{
    int *onebit = g_bits;
    int remainder;
    int i, j;

    printf("Data codewords: %d (%d bits)\n", (g_datLen / 8), g_datLen);
    for (i=0, j=0; i<g_datLen; i+=8, j++)
    {
        if ((j != 0) && ((j % 4) == 0)) printf("\n");
        printf(
            "%d%d%d%d%d%d%d%d ",
            onebit[0],
            onebit[1],
            onebit[2],
            onebit[3],
            onebit[4],
            onebit[5],
            onebit[6],
            onebit[7]
        );
        onebit += 8;
    }
    printf("\n\n");

    printf("EC codewords: %d (%d bits)\n", (g_eccLen / 8), g_eccLen);
    for (i=0, j=0; i<g_eccLen; i+=8, j++)
    {
        if ((j != 0) && ((j % 4) == 0)) printf("\n");
        printf(
            "%d%d%d%d%d%d%d%d ",
            onebit[0],
            onebit[1],
            onebit[2],
            onebit[3],
            onebit[4],
            onebit[5],
            onebit[6],
            onebit[7]
        );
        onebit += 8;
    }
    printf("\n\n");

    remainder = (g_totLen - g_datLen - g_eccLen);
    printf("Required remainder bits: %d\n", remainder);
    if (remainder > 0)
    {
        for (i=0; i<remainder; i++)
        {
            printf("%d", onebit[i]);
        }
        printf("\n");
    }
    printf("\n");

    printf("Total codewords: %d (%d bits)\n\n", (g_totLen / 8), g_totLen);
}

void qr_data_ecc_encode(char *string)
{
    unsigned char padding[2] = { 0xEC, 0x11 };
    unsigned char codeword1[2956]; /* Data codewords */
    unsigned char codeword2[3706]; /* Data and EC codewords */
    int codewordLen1 = 0;
    int codewordLen2 = 0;
    int count = 0; /* codeword bits */
    int infoLen;
    int ver = g_version - 1;
    int i;
    int j;
    int k;

    strncpy(g_string, string, 4095);
    infoLen = strlen( string );
    memset(codeword1, 0, 2956);
    memset(codeword2, 0, 3706);

    /* [1] Mode Indicator (4 bits) */
    codeword1[0] = 0x40;
    count += 4;

    /* [2] Character Count Indicator (8 or 16 bits) */
    if (g_version > 9)
    {
        codeword1[0] |= ((infoLen >> 15) & 0x1) << 3;
        codeword1[0] |= ((infoLen >> 14) & 0x1) << 2;
        codeword1[0] |= ((infoLen >> 13) & 0x1) << 1;
        codeword1[0] |= ((infoLen >> 12) & 0x1);
        for (i=7; i>=0; i++)
        {
            codeword1[1] |= ((infoLen >> (i + 4)) & 0x1) << i;
        }
        codeword1[2] |= ((infoLen >> 3) & 0x1) << 7;
        codeword1[2] |= ((infoLen >> 2) & 0x1) << 6;
        codeword1[2] |= ((infoLen >> 1) & 0x1) << 5;
        codeword1[2] |= ((infoLen     ) & 0x1) << 4;
        count += 16;
    }
    else
    {
        codeword1[0] |= ((infoLen >> 7) & 0x1) << 3;
        codeword1[0] |= ((infoLen >> 6) & 0x1) << 2;
        codeword1[0] |= ((infoLen >> 5) & 0x1) << 1;
        codeword1[0] |= ((infoLen >> 4) & 0x1);
        codeword1[1] |= ((infoLen >> 3) & 0x1) << 7;
        codeword1[1] |= ((infoLen >> 2) & 0x1) << 6;
        codeword1[1] |= ((infoLen >> 1) & 0x1) << 5;
        codeword1[1] |= ((infoLen     ) & 0x1) << 4;
        count += 8;
    }

    /* [3] Data (56 to 23624 bits) */
    for (i=0, j=(count/8); i<infoLen; i++, j++)
    {
        codeword1[j  ] |= ((string[i] >> 4) & 0xF);
        codeword1[j+1]  = ((string[i] & 0xF) << 4);
    }
    count += (infoLen * 8);

    /* [4] Terminate Indicator (0 to 4 bits) */
    for (i=0; i<4; i++)
    {
        if (count >= g_datLen) break;
        count++;
    }

    /* [5] Padding */
    for (i=0; i<(count%8); i++)
    {
        count++;
    }
    for (i=0, j=(count/8); count<g_datLen; count+=8)
    {
        codeword1[j++] = padding[i];
        i ^= 0x1;
    }

    if (count != g_datLen)
    {
        printf(
            "ERR: wrong data coding bits (%d <--> %d)\n",
            count,
            g_datLen
        );
    }

    /* [6] Error Correction Level (56 to 19440 bits) */
    {
        int (*ecTable)[6];
        unsigned char generator[30]; // max. EC codewords per block
        int blockNum;
        unsigned char *datBlock = codeword1;
        int datBlockLen;
        unsigned char eccBlock[30];
        int eccBlockLen;

        ecTable = (1 == g_level) ? g_L :
                  (2 == g_level) ? g_M :
                  (3 == g_level) ? g_Q :
                                   g_H;
        blockNum = (ecTable[ver][2] + ecTable[ver][4]);
        eccBlockLen = ecTable[ver][1];
        codewordLen1 = g_datLen / 8;
        codewordLen2 = codewordLen1 + (eccBlockLen * blockNum);

        reedSolomonPoly(generator, eccBlockLen);

        for (i=0; i<blockNum; i++)
        {
            datBlockLen
             = ecTable[ver][3] + (i < ecTable[ver][2] ? 0 : 1);
            reedSolomonCalc(
                generator,
                datBlock,
                datBlockLen,
                eccBlock,
                eccBlockLen
            );
            for (j=0, k=i; j<datBlockLen; j++, k+=blockNum)
            {
                if (j == ecTable[ver][3]) k -= ecTable[ver][2];
                codeword2[k] = datBlock[j];
            }
            for (j=0, k=codewordLen1+i; j<eccBlockLen; j++, k+=blockNum)
            {
                codeword2[k] = eccBlock[j];
            }
            datBlock += datBlockLen;
        }
    }

    /* Convert codeword from byte to bits */
    count = 0;
    for (j=0; j<codewordLen2; j++)
    {
        for (i=7; i>=0; i--)
        {
            g_bits[count++] = (codeword2[j] >> i) & 0x1;
        }
    }

    if (count != g_totLen)
    {
        printf(
            "ERR: wrong total coding bits (%d <--> %d)\n",
            count,
            g_totLen
        );
    }

    /* [7] Required Remainder Bits (0 to 7 bits) */
    g_totLen += g_rem[ver];
}

