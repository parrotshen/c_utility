#ifndef _DHCP_TYPES_H_
#define _DHCP_TYPES_H_

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define BYTE_ARRAY_TO_UINT16(p, n) \
    do { \
        n  = (*((uint8*)p  ) <<  8); \
        n |= (*((uint8*)p+1)      ); \
    } while(0)

#define UINT16_TO_BYTE_ARRAY(n, p) \
    do { \
        *((uint8*)p  ) = ((n >>  8) & 0xFF); \
        *((uint8*)p+1) = ((n      ) & 0xFF); \
    } while (0)

#define BYTE_ARRAY_TO_UINT32(p, n) \
    do { \
        n  = (*((uint8*)p  ) << 24); \
        n |= (*((uint8*)p+1) << 16); \
        n |= (*((uint8*)p+2) <<  8); \
        n |= (*((uint8*)p+3)      ); \
    } while (0)

#define UINT32_TO_BYTE_ARRAY(n, p) \
    do { \
        *((uint8*)p  ) = ((n >> 24) & 0xFF); \
        *((uint8*)p+1) = ((n >> 16) & 0xFF); \
        *((uint8*)p+2) = ((n >>  8) & 0xFF); \
        *((uint8*)p+3) = ((n      ) & 0xFF); \
    } while (0)


#define BOOL_FALSE   0
#define BOOL_TRUE    1


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

/*  integers  */
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

typedef signed char     int8;
typedef signed short    int16;
typedef signed int      int32;

typedef unsigned char   bool;


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////



#endif
