#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "generate.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

unsigned char multiply(unsigned char x, unsigned char y)
{
    unsigned char z = 0;
    int i;

    for (i=7; i>=0; i--)
    {
        z = ((z << 1) ^ ((z >> 7) * 0x11D));
        z ^= ((y >> i) & 0x1) * x;
    }

    return z;
}

void reedSolomonPoly(unsigned char result[], int degree)
{
    unsigned char root = 1;
    int i;
    int j;

    memset(result, 0, degree);
    result[degree - 1] = 1;

    for (i=0; i<degree; i++)
    {
        for (j=0; j<degree; j++)
        {
            result[j] = multiply(result[j], root);
            if ((j + 1) < degree) result[j] ^= result[j + 1];
        }
        root = multiply(root, 0x2);
    }
}

void reedSolomonCalc( 
    unsigned char generator[],
    unsigned char data[],
    int           dataLen,
    unsigned char result[],
    int           degree
)
{
    unsigned char factor;
    int i;
    int j;

    memset(result, 0, degree);
    for (i=0; i<dataLen; i++)
    {
        factor = data[i] ^ result[0];
        memmove(&result[0], &result[1], (degree - 1));
        result[degree - 1] = 0;
        for (j=0; j<degree; j++)
        {
            result[j] ^= multiply(generator[j], factor);
        }
    }
}

