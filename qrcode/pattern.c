#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "generate.h"
#include "pattern.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

int g_alignment[40][8] = {
/* Alignment pattern locations:
*      Alignment patterns
*      |  Location (Row and column)
*      |  |   |   |   |    |    |    | */
    {  0,                              }, //1
    {  1, 6, 18,                       }, //2  
    {  1, 6, 22,                       }, //3  
    {  1, 6, 26,                       }, //4  
    {  1, 6, 30,                       }, //5  
    {  1, 6, 34,                       }, //6  
    {  6, 6, 22, 38,                   }, //7 
    {  6, 6, 24, 42,                   }, //8 
    {  6, 6, 26, 46,                   }, //9 
    {  6, 6, 28, 50,                   }, //10 
    {  6, 6, 30, 54,                   }, //11 
    {  6, 6, 32, 58,                   }, //12 
    {  6, 6, 34, 62,                   }, //13 
    { 13, 6, 26, 46, 66,               }, //14
    { 13, 6, 26, 48, 70,               }, //15
    { 13, 6, 26, 50, 74,               }, //16
    { 13, 6, 30, 54, 78,               }, //17
    { 13, 6, 30, 56, 82,               }, //18
    { 13, 6, 30, 58, 86,               }, //19
    { 13, 6, 34, 62, 90,               }, //20
    { 22, 6, 28, 50, 72,  94,          }, //21
    { 22, 6, 26, 50, 74,  98,          }, //22
    { 22, 6, 30, 54, 78, 102,          }, //23
    { 22, 6, 28, 54, 80, 106,          }, //24
    { 22, 6, 32, 58, 84, 110,          }, //25
    { 22, 6, 30, 58, 86, 114,          }, //26
    { 22, 6, 34, 62, 90, 118,          }, //27
    { 33, 6, 26, 50, 74,  98, 122,     }, //28
    { 33, 6, 30, 54, 78, 102, 126,     }, //29
    { 33, 6, 26, 52, 78, 104, 130,     }, //30
    { 33, 6, 30, 56, 82, 108, 134,     }, //31
    { 33, 6, 34, 60, 86, 112, 138,     }, //32
    { 33, 6, 30, 58, 86, 114, 142,     }, //33
    { 33, 6, 34, 62, 90, 118, 146,     }, //34
    { 46, 6, 30, 54, 78, 102, 126, 150 }, //35
    { 46, 6, 24, 50, 76, 102, 128, 154 }, //36
    { 46, 6, 28, 54, 80, 106, 132, 158 }, //37
    { 46, 6, 32, 58, 84, 110, 136, 162 }, //38
    { 46, 6, 26, 54, 82, 110, 138, 166 }, //39
    { 46, 6, 30, 58, 86, 114, 142, 170 }  //40
};

unsigned char g_position[4][8][8] = {
    {
        /* finder and separator pattern: top left */
        { 1,1,1,1,1,1,1,0 },
        { 1,0,0,0,0,0,1,0 },
        { 1,0,1,1,1,0,1,0 },
        { 1,0,1,1,1,0,1,0 },
        { 1,0,1,1,1,0,1,0 },
        { 1,0,0,0,0,0,1,0 },
        { 1,1,1,1,1,1,1,0 },
        { 0,0,0,0,0,0,0,0 }
    },
    {
        /* finder and separator pattern: top right */
        { 0,1,1,1,1,1,1,1 },
        { 0,1,0,0,0,0,0,1 },
        { 0,1,0,1,1,1,0,1 },
        { 0,1,0,1,1,1,0,1 },
        { 0,1,0,1,1,1,0,1 },
        { 0,1,0,0,0,0,0,1 },
        { 0,1,1,1,1,1,1,1 },
        { 0,0,0,0,0,0,0,0 }
    },
    {
        /* finder and separator pattern: bottom left */
        { 0,0,0,0,0,0,0,0 },
        { 1,1,1,1,1,1,1,0 },
        { 1,0,0,0,0,0,1,0 },
        { 1,0,1,1,1,0,1,0 },
        { 1,0,1,1,1,0,1,0 },
        { 1,0,1,1,1,0,1,0 },
        { 1,0,0,0,0,0,1,0 },
        { 1,1,1,1,1,1,1,0 }
    },
    {
        /* alignment pattern */
        { 1,1,1,1,1 },
        { 1,0,0,0,1 },
        { 1,0,1,0,1 },
        { 1,0,0,0,1 },
        { 1,1,1,1,1 }
    }
};


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void qr_function_patterns(void)
{
    #if (DEBUG_PATTERN)
    int reserved = 2;
    #else
    int reserved = 0;
    #endif
    int version = g_version - 1;
    int locate[3][2];
    int loop;
    int offset;
    int col, row;
    int x, y;
    int i, j;

    g_matrixSize = (version * 4) + MIN_SIZE;
    for (y=0; y<MAX_SIZE; y++)
    {
        for (x=0; x<MAX_SIZE; x++)
        {
            g_matrixA[x][y] = -1;
        }
    }
    locate[0][0] = 0;
    locate[0][1] = 0;
    locate[1][0] = g_matrixSize - 8;
    locate[1][1] = 0;
    locate[2][0] = 0;
    locate[2][1] = g_matrixSize - 8;

    /* [1] finder and separator pattern */
    for (i=0; i<3; i++)
    {
        for (row=0; row<8; row++)
        {
            y = row + locate[i][1];
            for (col=0; col<8; col++)
            {
                x = col + locate[i][0];
                g_matrixA[x][y] = g_position[i][row][col];
            }
        }
    }

    /* [2] timing pattern */
    for (x=8, y=6; x<locate[1][0]; x++)
    {
        g_matrixA[x][y] = (x & 0x1) ^ 0x1;
    }
    for (x=6, y=8; y<locate[2][1]; y++)
    {
        g_matrixA[x][y] = (y & 0x1) ^ 0x1;
    }

    /* [3] alignment pattern */
    if (g_alignment[version][0] > 0)
    {
        loop = sqrt(g_alignment[version][0] + 3);
        for (j=0; j<loop; j++)
        {
            for (i=0; i<loop; i++)
            {
                if (((i == 0) && (j == 0)) ||
                    ((i == 0) && (j == (loop-1))) ||
                    ((i == (loop-1)) && (j == 0)))
                {
                    continue;
                }

                for (row=0; row<5; row++)
                {
                    y = row + g_alignment[version][j+1] - 2;
                    for (col=0; col<5; col++)
                    {
                        x = col + g_alignment[version][i+1] - 2;
                        g_matrixA[x][y] = g_position[3][row][col];
                    }
                }
            }
        }
    }

    /* [4] dark module */
    offset = (4 * g_version) + 9;
    g_matrixA[8][offset] = 1;

    /* [5] reserved areas */
    for (x=0, y=8; x<8; x++)
    {
        /* top left: horizontal */
        if (x != 6) g_matrixA[x][y] = reserved;
    }
    for (x=8, y=0; y<9; y++)
    {
        /* top left: vertical */
        if (y != 6) g_matrixA[x][y] = reserved;
    }
    for (x=8, y=(offset+1); y<(offset+8); y++)
    {
        /* bottom left: vertical */
        g_matrixA[x][y] = reserved;
    }
    for (x=offset, y=8; x<(offset+8); x++)
    {
        /* top right: horizontal */
        g_matrixA[x][y] = reserved;
    }
    if (g_version >= 7)
    {
        /* bottom left: 6x3 */
        for (x=0; x<6; x++)
        {
            for (y=(offset-3); y<(offset); y++)
            {
                g_matrixA[x][y] = reserved;
            }
        }
        /* top right: 3x6 */
        for (x=(offset-3); x<(offset); x++)
        {
            for (y=0; y<6; y++)
            {
                g_matrixA[x][y] = reserved;
            }
        }
    }
}

