#include "sos_common.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

static char _chArray[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};
static int _chArraySize = sizeof( _chArray );


/* IMS parameters */
char g_imsDomain[256] = "ims.mnc001.mcc001.3gppnetwork.org";
char g_imsIp[20]      = "127.0.0.1";
int  g_imsPort        = 5060;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void sos_dumpMem(const char *pName, const void *pAddr, unsigned int size)
{
    unsigned char *p = (unsigned char *)pAddr;
    unsigned int   i;

    if (p == NULL)
    {
        printf("[SOS-D] %s is NULL !!\n", pName);
        printf("\n");
        return;
    }

    printf("[SOS-D] %s\n", pName);
    printf("size = %u\n", size);
    for (i=0; i<size; i++)
    {
        if ((i != 0) && ((i % 16) == 0))
        {
            printf("\n");
        }

        printf("%02X ", p[i]);
    }
    printf("\n");
    printf("[SOS-D]\n");
    printf("\n");
}

void sos_genRandByte(void *pBuf, int size)
{
    unsigned char *pData = (unsigned char *)pBuf;
    int i;

    if (pData != NULL)
    {
        srand( (int)time(0) );
        for (i=0; i<size; i++)
        {
            pData[i] = (unsigned char)(256.0*rand() / (RAND_MAX));
        }
    }
}

void sos_genRandNumber(char *pStr, int size)
{
    int i;

    if (pStr != NULL)
    {
        srand( (int)time(0) );
        for (i=0; i<size; i++)
        {
            pStr[i] = _chArray[ rand() % 10 ];
        }
    }
}

void sos_genRandString(char *pStr, int size)
{
    int i;

    if (pStr != NULL)
    {
        srand( (int)time(0) );
        for (i=0; i<size; i++)
        {
            pStr[i] = _chArray[ rand() % _chArraySize ];
        }
    }
}


int sos_str2ip(char *pStr, unsigned char *pIp)
{
    int tmpIp[4] = {0};
    int scnRet = 0;
    int status = 0;

    if ((NULL == pStr) || (NULL == pIp))
    {
        LOG_ERROR("%s: NULL pointer\n", __func__);
        return -1;
    }

    /* "ddd.ddd.ddd.ddd" - max : 15 characters */
    if (strlen(pStr) > 15)
    {
        LOG_ERROR("incorrect IPv4 address length %d!\n", strlen(pStr));
        return -1;
    }

    scnRet = sscanf(
                 pStr,
                 "%d.%d.%d.%d",
                 &tmpIp[0],
                 &tmpIp[1],
                 &tmpIp[2],
                 &tmpIp[3]
             );
    if (scnRet != 4)
    {
        LOG_ERROR("incorrect IPv4 address format %s\n", pStr);
        return -1;
    }

    pIp[0] = (tmpIp[0] & 0xFF);
    pIp[1] = (tmpIp[1] & 0xFF);
    pIp[2] = (tmpIp[2] & 0xFF);
    pIp[3] = (tmpIp[3] & 0xFF);

    return status;
}


