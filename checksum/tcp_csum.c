#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "libparser.h"
#include "onec.h"


unsigned char g_pseudoIp[40];
unsigned char g_tcpPacket[1500];

/*
*      0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7
*    +----------------/----------------/----------------/----------------+
*  0 | Source Port                     | Destination Port                |
*    +---------------------------------+---------------------------------+
*  4 | Sequence Number                                                   |
*    +-------------------------------------------------------------------+
*  8 | Acknowledgment Number                                             |
*    +--------+-----+------------------+---------------------------------+
* 12 | Offset |     | Flags            | Window Size                     |
*    +--------+-----+------------------+---------------------------------+
* 16 | Checksum                        | Urgent Pointer                  |
*    +----------------/----------------/----------------/----------------+
* 20 | Options and payload ...                                           |
*    +----------------/----------------/----------------/----------------+
*/
#define TCP_CSUM(hdr, hdrLen, data, dataLen) onec(hdr, hdrLen, data, dataLen, 16)


static void help(void)
{
    printf("Usage: tcp_csum [OPTION]...\n");
    printf("\n");
    printf("  -f FILE    TCP packet .txt file.\n");
    printf("  -s ADDR    Source address.\n");
    printf("  -d ADDR    Destination address.\n");
    printf("  -h         Show the help message.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    unsigned short csum;
    unsigned int   len;
    char *pFileName = NULL;
    char *pSrcAddr = NULL;
    char *pDstAddr = NULL;
    int   ipv4v6 = 0;
    int   ch;
    int   n;


    opterr = 0;
    while ((ch=getopt(argc, argv, "f:s:d:h")) != -1)
    {
        switch ( ch )
        {
            case 'f':
                pFileName = optarg;
                break;
            case 's':
                pSrcAddr = optarg;
                break;
            case 'd':
                pDstAddr = optarg;
                break;
            case 'h':
            default:
                help();
                return 0;
        }
    }

    if (NULL == pFileName)
    {
        printf("ERR: TCP packet file is missing\n\n");
        help();
        return 0;
    }
    if (NULL == pSrcAddr)
    {
        printf("ERR: source address is missing\n\n");
        help();
        return 0;
    }
    if (NULL == pDstAddr)
    {
        printf("ERR: destination address is missing\n\n");
        help();
        return 0;
    }

    for (n=0; n<strlen(pSrcAddr); n++)
    {
        if ('.' == pSrcAddr[n])
        {
            ipv4v6 = 4;
            break;
        }
        else if (':' == pSrcAddr[n])
        {
            ipv4v6 = 6;
            break;
        }
    }
    if ((4 != ipv4v6) && (6 != ipv4v6))
    {
        printf("ERR: neither IPv4 nor IPv6 address\n\n");
        help();
        return 0;
    }

    len = parse_hex_string_file(pFileName, &(g_tcpPacket[0]), 1499);
    if (len & 0x1)
    {
        g_tcpPacket[len] = 0x00;
    }

    if (4 == ipv4v6)
    {
        str2ip(4, pSrcAddr, &(g_pseudoIp[0]));
        str2ip(4, pDstAddr, &(g_pseudoIp[4]));
        g_pseudoIp[8]  = 0;
        g_pseudoIp[9]  = 6;
        g_pseudoIp[10] = ((len >> 8) & 0xFF);
        g_pseudoIp[11] = ((len     ) & 0xFF);

        csum = TCP_CSUM(&(g_pseudoIp[0]), 12, &(g_tcpPacket[0]), len);

        mem_dump("Pseudo IP:", g_pseudoIp, 12);
    }
    else
    {
        str2ip(6, pSrcAddr, &(g_pseudoIp[0]));
        str2ip(6, pDstAddr, &(g_pseudoIp[16]));
        g_pseudoIp[32] = ((len >> 24) & 0xFF);
        g_pseudoIp[33] = ((len >> 16) & 0xFF);
        g_pseudoIp[34] = ((len >>  8) & 0xFF);
        g_pseudoIp[35] = ((len      ) & 0xFF);
        g_pseudoIp[36] = 0;
        g_pseudoIp[37] = 0;
        g_pseudoIp[38] = 0;
        g_pseudoIp[39] = 6;

        csum = TCP_CSUM(&(g_pseudoIp[0]), 40, &(g_tcpPacket[0]), len);

        mem_dump("Pseudo IP:", g_pseudoIp, 40);
    }

    mem_dump("TCP packet:", g_tcpPacket, len);

    printf("TCP checksum: 0x%04X\n\n", csum);

    return 0;
}

