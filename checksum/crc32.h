#ifndef __CRC32_H__
#define __CRC32_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>


typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

typedef signed char     int8;
typedef signed short    int16;
typedef signed int      int32;


uint32 crc32(uint32 crc, void *pBuf, size_t size);

#endif
