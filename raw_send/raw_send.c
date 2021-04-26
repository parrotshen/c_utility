#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <netinet/in.h>  /* htons */


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#ifndef APP_NAME
#define APP_NAME  "raw_send"
#endif

#define IS_UPPER_ALHPABET(ch)  ((ch >= 'A') && (ch <=  'Z'))
#define IS_LOWER_ALHPABET(ch)  ((ch >= 'a') && (ch <=  'z'))
#define IS_NUMBER(ch)          ((ch >= '0') && (ch <=  '9'))
#define IS_SPACE(ch)           ((ch == ' ') || (ch == '\t'))
#define LINE_SIZE   1023
#define TOKEN_SIZE  31

/* see linux/if_ether.h */
#define ETH_MAC_LEN         ETH_ALEN       /* Octets in one ethernet addr   */
#define ETH_HEADER_LEN      ETH_HLEN       /* Total octets in header.       */
#define ETH_MIN_FRAME_LEN   ETH_ZLEN       /* Min. octets in frame sans FCS */
#define ETH_USER_DATA_LEN   ETH_DATA_LEN   /* Max. octets in payload        */
#define ETH_MAX_FRAME_LEN   ETH_FRAME_LEN  /* Max. octets in frame sans FCS */

#define RAW_BUF_SIZE        1500

#define ETH_FRAME_TOTALLEN  1518    /* (Header 14) + (User Data 1500) + (FCS 4) */
#define ETH_P_NULL          0x0     /* We are running without any protocol above the Ethernet Layer */
#define ETH_DEVICE          "eth0"  /* Device used for communication */


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

static unsigned char  _buf[RAW_BUF_SIZE] = {
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00,
};
static int  _len = ETH_HEADER_LEN;


/* RAW socket */
static int  _sockRaw = -1;

/* Ethernet MAC address */
static unsigned char  _srcMac[ETH_MAC_LEN] = {0};

/* Ethernet interface */
static struct ifreq  _ifReq;
static int           _ifIndex = 0;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void mem_dump(char *pName, void *pAddr, unsigned int len)
{
    unsigned char *pByte = pAddr;
    unsigned int   i;

    if (pByte == NULL)
    {
        printf("%s (NULL)\n", pName);
        printf("\n");
        return;
    }

    printf("%s (%u bytes)\n", pName, len);
    for(i=0; i<len; i++)
    {
        if ((i != 0) && ((i % 16) == 0))
        {
            printf("\n");
        }
        printf(" %02X", pByte[i]);
    }
    printf("\n\n");
}

char *get_token(char *pString, char *pToken, int tsize)
{
    char *pBuf = pToken;    /* pointer to token */
    char *pNext = pString;  /* pointer to line  */
    int   i = 0;

    if (0x0 == pString[0])
    {
        /* This is a NULL line */
        pToken[0] = 0x0;
        return NULL;
    }

    /* Pass space and tab character */
    for (; *pNext && IS_SPACE(*pNext); pNext++);

    /* Get the separation token */
    for (; *pNext && !IS_SPACE(*pNext) && i<tsize; pNext++, i++)
    {
        *pBuf++ = *pNext;
    }
    *pBuf = 0x0;

    return pNext;
}

int read_line(FILE *pFile, char *pLine, int lsize)
{
    pLine[0] = 0x0;

    if ( feof(pFile) )
    {
        return 0;
    }

    /* char *fgets(                                   */
    /*     char *s,      // character array to store  */
    /*     int   n,      // length to read            */
    /*     FILE *stream  // FILE pointer              */
    /* );                                             */
    fgets(pLine, lsize, pFile);

    /* remove the CR/LF character */
    if ((strlen(pLine) > 0) && (pLine[strlen(pLine)-1] == 0x0a))
    {
        pLine[strlen(pLine)-1] = 0x0;
    }
    if ((strlen(pLine) > 0) && (pLine[strlen(pLine)-1] == 0x0d))
    {
        pLine[strlen(pLine)-1] = 0x0;
    }

    return 1;
}

int read_file(char *pFileName, unsigned char *pBuf, int bsize)
{
    unsigned char *pByte = pBuf;
    int   count = 0;

    FILE *pInput = NULL;
    char  line[LINE_SIZE+1];
    char  token[TOKEN_SIZE+1];
    char *pNext;
    int   i;

    if ((pInput=fopen(pFileName, "r")) == NULL)
    {
        printf("ERR: cannot open file '%s'\n", pFileName);
        return 0;
    }

    /* start reading input file */
    while ( read_line(pInput, line, LINE_SIZE) )
    {
        pNext = line;

        do
        {
            pNext = get_token(pNext, token, TOKEN_SIZE);
            if ((0x0 == token[0]) || ('#' == token[0]))
            {
                /* ignore the comment and null line */
                break;
            }

            /* get the token and transfer to one byte */
            sscanf(token, "%x", &i);
            if ((count+1) > bsize)
            {
                printf("ERR: buffer size(%d) is un-enough !!\n", bsize);
                goto _EXIT_READ_FILE;
            }

            *pByte++ = (i & 0xFF);
            count++;
        } while ( pNext );
    }

_EXIT_READ_FILE:
    fclose( pInput );
    return count;
}


/**
*  Send data by a RAW socket.
*  @param [in]  pData  A pointer of data buffer.
*  @param [in]  size   Data size.
*  @returns  Message length (-1 is failed).
*/
int raw_sockSend(unsigned char *pData, unsigned short size)
{
    unsigned char *pDestMac = pData;
    struct sockaddr_ll  sockAddr;
    int  retval;


    if (NULL == pData)
    {
        printf("[%s] %s: NULL pointer\n", APP_NAME, __func__);
        return -1;
    }

    /* prepare sockaddr_ll */
    sockAddr.sll_family   = AF_PACKET;
    sockAddr.sll_protocol = htons(ETH_P_IP);
    sockAddr.sll_ifindex  = _ifIndex;
    sockAddr.sll_hatype   = ARPHRD_ETHER;
    sockAddr.sll_pkttype  = PACKET_OTHERHOST;
    sockAddr.sll_halen    = ETH_ALEN;
    sockAddr.sll_addr[0]  = pDestMac[0];
    sockAddr.sll_addr[1]  = pDestMac[1];
    sockAddr.sll_addr[2]  = pDestMac[2];
    sockAddr.sll_addr[3]  = pDestMac[3];
    sockAddr.sll_addr[4]  = pDestMac[4];
    sockAddr.sll_addr[5]  = pDestMac[5];
    sockAddr.sll_addr[6]  = 0x00; 
    sockAddr.sll_addr[7]  = 0x00;

    retval = sendto(
                 _sockRaw,
                 pData,
                 size,
                 0,
                 (struct sockaddr *)&sockAddr,
                 sizeof( sockAddr )
             );
    if (retval < 0)
    {
        perror( "sendto()" );
    }

    return retval;
}

/**
*  Initialize RAW socket library.
*  @param [in]  pEthDev    Ethernet device name.
*  @param [in]  pRecvFunc  Application's receive callback function.
*  @returns  Success(0) or failure(-1).
*/
int raw_sockInit(char *pEthDev)
{
    int  i;

    if (NULL == pEthDev)
    {
        pEthDev = ETH_DEVICE;
    }
    printf("[%s] Ethernet device: %s\n", APP_NAME, pEthDev);

    _sockRaw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (_sockRaw < 0)
    {
        perror( "socket()" );
        goto _RAW_INIT_1;
    }

    memset(&_ifReq, 0x00, sizeof( _ifReq ));
    strncpy(_ifReq.ifr_name, pEthDev, IFNAMSIZ);

    /* retrieve ethernet interface index */
    if (ioctl(_sockRaw, SIOCGIFINDEX, &_ifReq) < 0)
    {
        perror( "SIOCGIFINDEX" );
        goto _RAW_INIT_2;
    }
    _ifIndex = _ifReq.ifr_ifindex;

    /* retrieve corresponding MAC */
    if (ioctl(_sockRaw, SIOCGIFHWADDR, &_ifReq) < 0)
    {
        perror( "SIOCGIFHWADDR" );
        goto _RAW_INIT_2;
    }
    for (i=0; i<ETH_MAC_LEN; i++)
    {
        _srcMac[i] = _ifReq.ifr_hwaddr.sa_data[i];
    }

    return 0;

_RAW_INIT_2:
    close( _sockRaw );
    _sockRaw = -1;
_RAW_INIT_1:
    printf("[%s] failed to create a RAW socket\n", APP_NAME);
    return -1;
}

/**
*  Un-initialize RAW socket library.
*/
void raw_sockUninit(void)
{
    if (_sockRaw > 0)
    {
        close( _sockRaw );
        _sockRaw = -1;
    }
}


int main(int argc,char *argv[])
{
    char *pDev = "eth0";
    int retval;
    int count;
    int sent;
    int i;


    if (argc < 2)
    {
        printf("Usage: raw_send TEXT_FILE...\n");
        printf("\n");
        printf("EX   : raw_send 1.txt 2.txt 3.txt\n");
        printf("\n");
        return 0;
    }

    retval = raw_sockInit( pDev );
    if (retval != 0)
    {
        printf("[%s] fail to initial raw socket (%s)\n\n", APP_NAME, pDev);
        return -1;
    }

    count = 0;
    for (i=0; i<(argc - 1); i++)
    {
        if ((_len=read_file(argv[i+1], _buf, RAW_BUF_SIZE)) == 0)
        {
            printf("[%s] fail to open file %s\n\n", APP_NAME, argv[i+1]);
            continue;
        }

        /* send packet */
        sent = raw_sockSend(_buf, _len);
        if (sent < 0)
        {
            printf("[%s] send data failed (%d)\n", APP_NAME, sent);
        }
        else
        {
            mem_dump(argv[i+1], _buf, sent);
        }

        count++;

        usleep(100000);
    }

    raw_sockUninit();

    printf("[%s] %d packet(s) were transmitted\n\n", APP_NAME, count);
    return 0;
}

