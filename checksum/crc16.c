/*
*  CRC16 calculation
*/

#include "crc16.h"


uint16 crc16(
    uint16  poly,
    uint16  init,
    int     refin,
    int     refout,
    uint16  xout,
    void   *pData,
    size_t  size
)
{
    uint8  *pByte = pData;
    uint16  crc = init;
    uint8   val8;
    uint16  val16;
    int     i;
    int     j;

    for (i=0; i<size; i++)
    {
        for (j=0x80; j!=0; j=j>>1)
        {
            if ((crc & 0x8000) !=0)
            {
                crc = (crc << 1);
                crc = (crc ^ poly);
            }
            else
            {
                crc = (crc << 1);
            }

            if ( refin )
            {
                val8 = ((pByte[i] & 0x80) >> 7) |
                       ((pByte[i] & 0x40) >> 5) |
                       ((pByte[i] & 0x20) >> 3) |
                       ((pByte[i] & 0x10) >> 1) |
                       ((pByte[i] & 0x01) << 7) |
                       ((pByte[i] & 0x02) << 5) |
                       ((pByte[i] & 0x04) << 3) |
                       ((pByte[i] & 0x08) << 1);
            }
            else
            {
                val8 = pByte[i];
            }

            if ((val8 & j) != 0) crc = (crc ^ poly);
        }
    }

    if ( refout )
    {
        val16 = ((crc & 0x8000) >> 15) |
                ((crc & 0x4000) >> 13) |
                ((crc & 0x2000) >> 11) |
                ((crc & 0x1000) >>  9) |
                ((crc & 0x0800) >>  7) |
                ((crc & 0x0400) >>  5) |
                ((crc & 0x0200) >>  3) |
                ((crc & 0x0100) >>  1) |
                ((crc & 0x0001) << 15) |
                ((crc & 0x0002) << 13) |
                ((crc & 0x0004) << 11) |
                ((crc & 0x0008) <<  9) |
                ((crc & 0x0010) <<  7) |
                ((crc & 0x0020) <<  5) |
                ((crc & 0x0040) <<  3) |
                ((crc & 0x0080) <<  1);
        crc = val16;
    }

    return (crc ^ xout);
}

