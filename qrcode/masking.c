#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "generate.h"
#include "masking.h"
#include "encode.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

int g_masking[8][MAX_SIZE][MAX_SIZE];
int g_maskId = 0;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

int evaluate_1(int matrix[MAX_SIZE][MAX_SIZE])
{
    int result[2];
    int score;
    int count;
    int color;
    int x, y;
    int i, j, n;

    #if (DEBUG_MASKING)
    printf("Evaluate #1\n");
    #endif

    for (n=0; n<2; n++)
    {
        result[n] = 0;
        for (j=0; j<g_matrixSize; j++)
        {
            score = 0;
            count = 1;
            if (0 == n)
            {
                y = j;
                color = matrix[0][y];
            }
            else
            {
                x = j;
                color = matrix[x][0];
            }
            for (i=1; i<g_matrixSize; i++)
            {
                if (0 == n)
                {
                    x = i;
                }
                else
                {
                    y = i;
                }
                if (color == matrix[x][y])
                {
                    count++;
                    if (count == 5) score += 3;
                    else if (count > 5) score += 1;
                }
                else
                {
                    count = 1;
                    color = matrix[x][y];
                }
            }
            result[n] += score;
            #if (DEBUG_MASKING)
            if (0 == n)
            {
                printf("horizontal[%d] %d -> %d\n", y, score, result[n]);
            }
            else
            {
                printf("vertical[%d] %d -> %d\n", x, score, result[n]);
            }
            #endif
        }
    }

    return (result[0] + result[1]);
}

int evaluate_2(int matrix[MAX_SIZE][MAX_SIZE])
{
    #if (DEBUG_MASKING)
    int count[2] = { 0, 0 };
    #endif
    int result = 0;
    int loop;
    int color;
    int x, y;

    #if (DEBUG_MASKING)
    printf("Evaluate #2\n");
    #endif

    loop = g_matrixSize - 1;
    for (y=0; y<loop; y++)
    {
        for (x=0; x<loop; x++)
        {
            color = matrix[x][y];
            if ((color == matrix[x+1][y  ]) &&
                (color == matrix[x  ][y+1]) &&
                (color == matrix[x+1][y+1]))
            {
                result += 3;
                #if (DEBUG_MASKING)
                count[color & 0x1]++;
                #endif
            }
        }
    }

    #if (DEBUG_MASKING)
    printf("%d = (%d + %d) * 3\n", result, count[0], count[1]);
    #endif
    return result;
}

int evaluate_3(int matrix[MAX_SIZE][MAX_SIZE])
{
    #if (DEBUG_MASKING)
    int count[4] = { 0, 0, 0, 0 };
    #endif
    int pattern[2][11] = {
        { 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0  },
        { 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1  }
    };
    int result[4];
    int loop;
    int x, y;
    int i, j, k, n;

    #if (DEBUG_MASKING)
    printf("Evaluate #3\n");
    #endif

    loop = g_matrixSize - 11 + 1;
    for (n=0; n<4; n++)
    {
        result[n] = 0;
        for (j=0; j<loop; j++)
        {
            if (0 == n)
            {
                y = j;
            }
            else
            {
                x = j;
            }
            for (i=0; i<loop; i++)
            {
                if (0 == n)
                {
                    x = i;
                    for (k=0; k<11; k++)
                    {
                        if (pattern[n>>1][k] != matrix[x+k][y]) break;
                    }
                }
                else
                {
                    y = i;
                    for (k=0; k<11; k++)
                    {
                        if (pattern[n>>1][k] != matrix[x][y+k]) break;
                    }
                }
                if (11 == k)
                {
                    result[n] += 40;
                    #if (DEBUG_MASKING)
                    count[n]++;
                    #endif
                }
            }
        }
    }

    #if (DEBUG_MASKING)
    for (n=0; n<4; n++)
    {
        printf("%d = %d * 40\n", result[n], count[n]);
    }
    #endif
    return (result[0] + result[1] + result[2] + result[3]);
}

int evaluate_4(int matrix[MAX_SIZE][MAX_SIZE])
{
    int result;
    int percent[3];
    int temp[2];
    int total;
    int dark;
    int x, y;

    #if (DEBUG_MASKING)
    printf("Evaluate #4\n");
    #endif

    total = (g_matrixSize * g_matrixSize);
    dark = 0;
    for (y=0; y<g_matrixSize; y++)
    {
        for (x=0; x<g_matrixSize; x++)
        {
            if (1 == matrix[x][y]) dark++;
        }
    }

    percent[1] = (dark * 100) / total;
    if ((percent[1] % 5) != 0)
    {
        percent[0] = (percent[1] / 5) * 5;
        percent[2] = (percent[0] + 5);
    }
    else
    {
        percent[0] = (percent[1]);
        percent[2] = (percent[0] + 5);
    }
    temp[0] = abs(percent[0] - 50) / 5;
    temp[1] = abs(percent[2] - 50) / 5;
    result = ((temp[0] < temp[1]) ? temp[0] : temp[1]);
    #if (DEBUG_MASKING)
    printf("%d / %d = %d%%\n", dark, total, percent[1]);
    printf("%d = %d * 10\n", (result * 10), result);
    #endif
    return (result * 10);
}

void qr_matrix_masking(void)
{
    int score = 0;
    int scoreMin = 2147483647;
    int x, y;
    int n;

    for (n=0; n<8; n++)
    {
        score = 0;

        memcpy(g_masking[n], g_matrixB, MAX_SIZE*MAX_SIZE*sizeof(int));
        for (y=0; y<g_matrixSize; y++) /* Row */
        {
            for (x=0; x<g_matrixSize; x++) /* Column */
            {
                if (-1 == g_matrixA[x][y]) /* Data and EC bits */
                {
                    switch ( n )
                    {
                        case 0:
                            g_masking[n][x][y] ^= (((y + x) % 2) == 0);
                            break;
                        case 1:
                            g_masking[n][x][y] ^= ((y % 2) == 0);
                            break;
                        case 2:
                            g_masking[n][x][y] ^= ((x % 3) == 0);
                            break;
                        case 3:
                            g_masking[n][x][y] ^= (((y + x) % 3) == 0);
                            break;
                        case 4:
                            g_masking[n][x][y] ^= ((((y / 2) + (x / 3)) % 2) == 0);
                            break;
                        case 5:
                            g_masking[n][x][y] ^= ((((y * x) % 2) + ((y * x) % 3)) == 0);
                            break;
                        case 6:
                            g_masking[n][x][y] ^= (((((y * x) % 2) + ((y * x) % 3)) % 2) == 0);
                            break;
                        default:
                            g_masking[n][x][y] ^= (((((y + x) % 2) + ((y * x) % 3)) % 2) == 0);
                            break;
                    }
                }
                #if (DEBUG_PATTERN)
                else if (2 == g_matrixA[x][y]) /* Reserved area */
                {
                    g_masking[n][x][y] = 0;
                }
                #endif
            }
        }

        score += evaluate_1( g_masking[n] );
        score += evaluate_2( g_masking[n] );
        score += evaluate_3( g_masking[n] );
        score += evaluate_4( g_masking[n] );
        #if (DEBUG_MASKING)
        printf("mask[%d] %d\n\n", n, score);
        #endif
        if (score < scoreMin)
        {
            scoreMin = score;
            g_maskId = n;
        }
    }

    #if (DEBUG_MASKING)
    printf("Select mask[%d] %d\n\n", g_maskId, scoreMin);
    #endif
}

