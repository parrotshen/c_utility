#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* one's complement sum */
unsigned short onec(
    unsigned char *pHdr,
    unsigned int   hdrLen,
    unsigned char *pData,
    unsigned int   dataLen,
    unsigned int   ignore
)
{
    unsigned int csum = 0;
    unsigned short val16;
    unsigned int i;

    for (i=0; i<hdrLen; i+=2)
    {
        val16 = ((pHdr[i] << 8) + pHdr[i + 1]);
        csum += val16;
    }
    for (i=0; i<dataLen; i+=2)
    {
        /* ignore the checksum field */
        if (i != ignore)
        {
            val16 = ((pData[i] << 8) + pData[i + 1]);
            csum += val16;
        }
    }

    csum = ((csum & 0xFFFF) + (csum >> 16));
    csum ^= 0xFFFF;

    return (unsigned short)csum;
}

