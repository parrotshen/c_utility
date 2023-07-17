#ifndef __CRC16_H__
#define __CRC16_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

typedef signed char     int8;
typedef signed short    int16;
typedef signed int      int32;


uint16 crc16(
    uint16  poly,
    uint16  init,
    int     refin,
    int     refout,
    uint16  xout,
    void   *pData,
    size_t  size
);


#endif
