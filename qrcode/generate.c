#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "generate.h"
#include "pattern.h"
#include "encode.h"
#include "masking.h"
#include "format.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

int g_codewords[40][5] = {
/* Byte mode character capacities:
*        Data and EC codewords (Total codewords)
*        |     L     M     Q     H (Data codewords)
*        |     |     |     |     | */
    {   26,   17,   14,   11,    7 },
    {   44,   32,   26,   20,   14 },
    {   70,   53,   42,   32,   24 },
    {  100,   78,   62,   46,   34 },
    {  134,  106,   84,   60,   44 },
    {  172,  134,  106,   74,   58 },
    {  196,  154,  122,   86,   64 },
    {  242,  192,  152,  108,   84 },
    {  292,  230,  180,  130,   98 },
    {  346,  271,  213,  151,  119 },
    {  404,  321,  251,  177,  137 },
    {  466,  367,  287,  203,  155 },
    {  532,  425,  331,  241,  177 },
    {  581,  458,  362,  258,  194 },
    {  655,  520,  412,  292,  220 },
    {  733,  586,  450,  322,  250 },
    {  815,  644,  504,  364,  280 },
    {  901,  718,  560,  394,  310 },
    {  991,  792,  624,  442,  338 },
    { 1085,  858,  666,  482,  382 },
    { 1156,  929,  711,  509,  403 },
    { 1258, 1003,  779,  565,  439 },
    { 1364, 1091,  857,  611,  461 },
    { 1474, 1171,  911,  661,  511 },
    { 1588, 1273,  997,  715,  535 },
    { 1706, 1367, 1059,  751,  593 },
    { 1828, 1465, 1125,  805,  625 },
    { 1921, 1528, 1190,  868,  658 },
    { 2051, 1628, 1264,  908,  698 },
    { 2185, 1732, 1370,  982,  742 },
    { 2323, 1840, 1452, 1030,  790 },
    { 2465, 1952, 1538, 1112,  842 },
    { 2611, 2068, 1628, 1168,  898 },
    { 2761, 2188, 1722, 1228,  958 },
    { 2876, 2303, 1809, 1283,  983 },
    { 3034, 2431, 1911, 1351, 1051 },
    { 3196, 2563, 1989, 1423, 1093 },
    { 3362, 2699, 2099, 1499, 1399 },
    { 3532, 2809, 2213, 1579, 1219 },
    { 3706, 2953, 2331, 1663, 1273 }
};

/*
* -1: null
*  0: white pixel
*  1: black pixel
*  2: reserved area
*/
int  g_matrixA[MAX_SIZE][MAX_SIZE]; /* Store modules before placement */
int  g_matrixB[MAX_SIZE][MAX_SIZE]; /* Store modules before masking */
int  g_matrixSize = 0;
int  g_version = 0;
int  g_level = 0;
char g_string[4096];


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

int qr_check_capacity(int bytes, int version, int level)
{
    int smallest = 0;
    int i;

    /* Version 1 ~ 40 */
    for (i=0; i<40; i++)
    {
        if (g_codewords[i][level] >= bytes)
        {
            smallest = (i + 1);
            if (version < smallest) version = smallest;
            if (version > 40) version = 40;
            return version;
        }
    }

    return 0;
}

void qr_matrix_placement(void)
{
#define UPWARD_R 1
#define UPWARD_L 2
#define DOWNWARD_R 3
#define DOWNWARD_L 4
    int state = UPWARD_R;
    int modules;
    int x, y;
    int i, j;

    modules = (g_matrixSize * g_matrixSize);
    x = y = g_matrixSize - 1;
    for (i=0, j=0; i<modules; i++)
    {
        switch ( state )
        {
            case UPWARD_R:
                if (-1 == g_matrixB[x][y])
                {
                    g_matrixB[x][y] = g_bits[j];
                    j++;
                }
                x--;
                state = UPWARD_L;
                break;
            case UPWARD_L:
                if (-1 == g_matrixB[x][y])
                {
                    g_matrixB[x][y] = g_bits[j];
                    j++;
                }
                y--;
                if (y >= 0)
                {
                    x++;
                    state = UPWARD_R;
                }
                else
                {
                    y = 0;
                    x--;
                    if (x == 6) x--;
                    state = DOWNWARD_R;
                }
                break;
            case DOWNWARD_R:
                if (-1 == g_matrixB[x][y])
                {
                    g_matrixB[x][y] = g_bits[j];
                    j++;
                }
                x--;
                state = DOWNWARD_L;
                break;
            case DOWNWARD_L:
                if (-1 == g_matrixB[x][y])
                {
                    g_matrixB[x][y] = g_bits[j];
                    j++;
                }
                y++;
                if (y < g_matrixSize)
                {
                    x++;
                    state = DOWNWARD_R;
                }
                else
                {
                    y = (g_matrixSize - 1);
                    x--;
                    if (x == 6) x--;
                    state = UPWARD_R;
                }
                break;
            default:
                printf("ERR: wrong state(%d) in %s\n", state, __func__);
                return;
        }

        if (j == g_totLen) return; /* placement finished */
        if ((x < 0) || (x >= g_matrixSize))
        {
            printf("ERR: wrong x(%d) in %s\n", x, __func__);
            return;
        }
    }

    printf("ERR: cannot place all bits %d/%d in %s\n", j, g_totLen, __func__);
}

void qr_matrix_output(void)
{
    char EC[] = { '?', 'L', 'M', 'Q', 'H' };
    #if (DEBUG_PATTERN)
    int (*matrix)[MAX_SIZE] = g_matrixA;
    #else
    int (*matrix)[MAX_SIZE] = g_masking[g_maskId];
    #endif
    int x, y;

    printf(
        "%d x %d (version %d%c)\n",
        g_matrixSize,
        g_matrixSize,
        g_version,
        EC[g_level]
    );
    printf("\n\n"); // Quiet zone
    for (y=0; y<g_matrixSize; y++)
    {
        printf("    "); // Quiet zone
        for (x=0; x<g_matrixSize; x++)
        {
            if (0 == matrix[x][y]) printf("  ");
            else if (1 == matrix[x][y]) printf("██");
            else if (2 == matrix[x][y]) printf("++");
            else printf("--");
        }
        printf("\n");
    }
    printf("\n\n"); // Quiet zone
    printf("%s\n", g_string);
}

int qr_code_generate(char *string, int version, int level)
{
    /* https://www.thonky.com/qr-code-tutorial */
    int bytes;

    bytes = strlen( string );
    version = qr_check_capacity(bytes, version, level);
    if (version == 0)
    {
        printf("ERR: insufficient data capacity (%d bytes)\n", bytes);
        return -1;
    }

    g_version = version;
    version = version - 1;
    g_level = level;
    memset(g_bits, 0, (29656 * sizeof(int)));
    g_totLen = (g_codewords[version][0] * 8);
    g_datLen = (1 == g_level) ? (g_L[version][0] * 8) :
               (2 == g_level) ? (g_M[version][0] * 8) :
               (3 == g_level) ? (g_Q[version][0] * 8) :
                                (g_H[version][0] * 8);
    g_eccLen = g_totLen - g_datLen;

    qr_function_patterns();
    qr_data_ecc_encode( string );
    #if (DEBUG_ENCODE)
    qr_show_codewords();
    #endif
    memcpy(g_matrixB, g_matrixA, MAX_SIZE*MAX_SIZE*sizeof(int));
    qr_matrix_placement();
    qr_matrix_masking();
    qr_format_version();
    qr_matrix_output();

    return 0;
}

