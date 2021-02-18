#ifndef _SOS_COMMON_H_
#define _SOS_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define LOG_INFO( fmt, a... )  printf( "[SOS-I] " fmt, ##a )
#define LOG_WARN( fmt, a... )  printf( "[SOS-W] " fmt, ##a )
#define LOG_ERROR( fmt, a... ) printf( "[SOS-E] " fmt, ##a )
#define LOG_DUMP( var, len )   sos_dumpMem( #var, var, len )

#define trace() printf(">> %s:%d\n", __FUNCTION__, __LINE__)

#define REMOVE_CRLF( pStr ) \
{ \
    char *ch = pStr; \
    for (; (*ch) && (*ch != '\n'); ch++); \
    (*ch) = 0x00; \
}

#define BOOL_FALSE (0)
#define BOOL_TRUE  (1)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint24;
typedef unsigned int   uint32;

typedef signed char    int8;
typedef signed short   int16;
typedef signed int     int24;
typedef signed int     int32;

typedef unsigned char  bool;

typedef unsigned long  intptr;


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

/* IMS parameters */
extern char g_imsDomain[256];
extern char g_imsIp[20];
extern int  g_imsPort;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void sos_dumpMem(const char *pName, const void *pAddr, unsigned int size);

void sos_genRandByte(void *pBuf, int size);
void sos_genRandNumber(char *pStr, int size);
void sos_genRandString(char *pStr, int size);

int sos_str2ip(char *pStr, unsigned char *pIp);


#endif //_SOS_COMMON_H_
