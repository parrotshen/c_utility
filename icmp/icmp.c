#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define ICMP_MAX_LEN  (1023)
#define ICMP_HDR_LEN  sizeof(struct icmphdr)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

unsigned char  g_txBuff[ICMP_MAX_LEN+1];
unsigned char  g_rxBuff[ICMP_MAX_LEN+1];
int  g_sock = -1;
int  g_debug = 0;

unsigned char  g_packet[28] = {
    /* IP header (20) */
    0x45, 0x00, 0x00, 0x54,
    0x00, 0x00, 0x40, 0x00,
    0x40, 0x01, 0xb5, 0x4a,
    0xc0, 0xa8, 0x02, 0x02,
    0xc0, 0xa8, 0x02, 0x0c,
    /* ICMP header (8) */
    0x08, 0x00, 0x2e, 0x13,
    0x7a, 0x3f, 0x00, 0x01
};


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void dump(void *pAddr, int size)
{
    unsigned char *pByte = pAddr;
    int  i;

    if (pByte != NULL)
    {
        for (i=0; i<size; i++)
        {
            if ((i != 0) && ((i % 8) == 0))
            {
                printf("\n");
            }

            printf(" %02X", pByte[i]);
        }
        printf("\n");
    }
}

unsigned short checksum(void *pData, int len)
{
    unsigned short *pBuf = pData;
    unsigned int    sum = 0;
    unsigned short  result;

    for (sum=0; len>1; len-= 2)
    {
        sum += *pBuf++;
    }
    if (len == 1)
    {
        sum += *((unsigned char *)pBuf);
    }
    sum  = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

int sendICMP(char *pIpAddr, int type, int code)
{
    int retval;
    struct sockaddr_in toAddr;
    struct icmp *pIcmp;
    unsigned char *pData;
    int toAddrLen;
    int size = ICMP_HDR_LEN;


    /*           0      1      2      3
     * pIcmp -> +------+------+------+------+ \
     *          | Type | Code | Checksum    | |
     *          +------+------+------+------+ | ICMP header
     *          | Rest of Header            | |
     * pData -> +------+------+------+------+ /
     *          |                           |
     *         ...                         ...
     *          |                           |
     *          +------+------+------+------+
     */
    memset(g_txBuff, 0, ICMP_HDR_LEN);

    pIcmp = (struct icmp *)g_txBuff;
    pIcmp->icmp_type = type;

    pData = (g_txBuff + ICMP_HDR_LEN);

    switch ( type )
    {
        case ICMP_ECHOREPLY:
            pIcmp->icmp_id = getpid();
            pIcmp->icmp_seq = (unsigned short)time(NULL);
            memset(pData, 0xFF, 56);
            size = (ICMP_HDR_LEN + 56);
            break;
        case ICMP_DEST_UNREACH:
            pIcmp->icmp_code = code;
            pIcmp->icmp_pmvoid = 0;
            pIcmp->icmp_nextmtu = htons(1500);
            memcpy(pData, g_packet, 28);
            size = (ICMP_HDR_LEN + 28);
            break;
        case ICMP_SOURCE_QUENCH:
            pIcmp->icmp_void = 0;
            memcpy(pData, g_packet, 28);
            size = (ICMP_HDR_LEN + 28);
            break;
        case ICMP_REDIRECT:
            pIcmp->icmp_code = 0;
            inet_pton(AF_INET, "192.168.96.123", &(pIcmp->icmp_gwaddr));
            memcpy(pData, g_packet, 28);
            size = (ICMP_HDR_LEN + 28);
            break;
        case ICMP_ECHO:
            pIcmp->icmp_id = getpid();
            pIcmp->icmp_seq = (unsigned short)time(NULL);
            memset(pData, 0xFF, 56);
            size = (ICMP_HDR_LEN + 56);
            break;
        case ICMP_TIME_EXCEEDED:
            pIcmp->icmp_code = 0;
            pIcmp->icmp_void = 0;
            memcpy(pData, g_packet, 28);
            size = (ICMP_HDR_LEN + 28);
            break;
        case ICMP_PARAMETERPROB:
            pIcmp->icmp_code = 0;
            pIcmp->icmp_pptr = 5;
            memcpy(pData, g_packet, 28);
            size = (ICMP_HDR_LEN + 28);
            break;
        case ICMP_TIMESTAMP:
            pIcmp->icmp_id = getpid();
            pIcmp->icmp_seq = (unsigned short)time(NULL);
            pIcmp->icmp_otime = 0x11111111;
            pIcmp->icmp_rtime = 0x22222222;
            pIcmp->icmp_ttime = 0x33333333;
            size = (ICMP_HDR_LEN + 12);
            break;
        case ICMP_TIMESTAMPREPLY:
            pIcmp->icmp_id = getpid();
            pIcmp->icmp_seq = (unsigned short)time(NULL);
            pIcmp->icmp_otime = 0x44444444;
            pIcmp->icmp_rtime = 0x55555555;
            pIcmp->icmp_ttime = 0x66666666;
            size = (ICMP_HDR_LEN + 12);
            break;
        case ICMP_ADDRESS:
            pIcmp->icmp_id = getpid();
            pIcmp->icmp_seq = (unsigned short)time(NULL);
            pIcmp->icmp_mask = 0;
            size = (ICMP_HDR_LEN + 4);
            break;
        case ICMP_ADDRESSREPLY:
            pIcmp->icmp_id = getpid();
            pIcmp->icmp_seq = (unsigned short)time(NULL);
            pIcmp->icmp_mask = ((255) | (255 << 8) | (255 << 16) | (0 << 24));
            size = (ICMP_HDR_LEN + 4);
            break;
        default:
            printf("%s: incorrect type %d\n\n", __func__, type);
            return -1;
    }

    pIcmp->icmp_cksum = checksum(pIcmp, size);

    /* destination address */
    toAddrLen = sizeof( struct sockaddr_in );
    bzero(&toAddr, toAddrLen);
    toAddr.sin_family = AF_INET;
    inet_pton(AF_INET, pIpAddr, &(toAddr.sin_addr));

    retval = sendto(
                 g_sock,
                 g_txBuff,
                 size,
                 0,
                 (struct sockaddr *)(&toAddr),
                 toAddrLen
             );

    printf("%d bytes -> %s\n", retval, pIpAddr);
    if ( g_debug )
    {
        dump(g_txBuff, size);
    }
    printf("\n");

    return retval;
}

void recvICMP(void)
{
    struct sockaddr_in  fromAddr;
    socklen_t  fromAddrLen;
    struct iphdr *pIpHdr;
    struct icmphdr *pIcmpHdr;
    unsigned char *pData;
    int  dataLen;
    int  size;


    /* source address */
    fromAddrLen = sizeof( struct sockaddr_in );
    bzero(&fromAddr, fromAddrLen);

    size = recvfrom(
               g_sock,
               g_rxBuff,
               ICMP_MAX_LEN,
               0,
               (struct sockaddr *)(&fromAddr),
               &fromAddrLen
           );
    if (size <= 0)
    {
        printf(
            "ERR: fail to receive socket (%d)\n",
            size
        );
    }
    else
    {
        g_rxBuff[size] = 0;

        pIpHdr = (struct iphdr *)g_rxBuff;
        pIcmpHdr = (struct icmphdr *)(g_rxBuff + (pIpHdr->ihl * 4));
        pData = (g_rxBuff + (pIpHdr->ihl * 4) + ICMP_HDR_LEN);
        dataLen = (size - (pIpHdr->ihl * 4) - ICMP_HDR_LEN);

        printf("%d bytes <- %s\n", size, inet_ntoa( fromAddr.sin_addr ));
        if ( g_debug )
        {
            dump(g_rxBuff, size);
        }
        printf("\n");

        if (IPPROTO_ICMP == pIpHdr->protocol)
        {
            switch ( pIcmpHdr->type )
            {
                case ICMP_ECHOREPLY:
                    printf("ECHO REPLY (data length %d)\n", dataLen);
                    dump(pData, dataLen);
                    break;
                case ICMP_DEST_UNREACH:
                    printf("DESTINATION UNREACHABLE (data length %d)\n", dataLen);
                    printf(" code: %u\n", pIcmpHdr->code);
                    break;
                case ICMP_SOURCE_QUENCH:
                    printf("SOURCE QUENCH (data length %d)\n", dataLen);
                    break;
                case ICMP_REDIRECT:
                    printf("REDIRECT MESSAGE (data length %d)\n", dataLen);
                    printf(" code: %u\n", pIcmpHdr->code);
                    printf(" IP: %d.%d.%d.%d\n", pData[0], pData[1], pData[2], pData[3]);
                    break;
                case ICMP_ECHO:
                    printf("ECHO REQUEST (data length %d)\n", dataLen);
                    dump(pData, dataLen);
                    break;
                case ICMP_TIME_EXCEEDED:
                    printf("TIME EXCEEDED (data length %d)\n", dataLen);
                    printf(" code: %u\n", pIcmpHdr->code);
                    break;
                case ICMP_PARAMETERPROB:
                    printf("PARAMETER PROBLEM (data length %d)\n", dataLen);
                    printf(" code: %u\n", pIcmpHdr->code);
                    break;
                case ICMP_TIMESTAMP:
                    printf("TIMESTAMP (data length %d)\n", dataLen);
                    printf(" originate: 0x%X\n", ntohl( *((unsigned int *)(pData    )) ));
                    printf(" receive  : 0x%X\n", ntohl( *((unsigned int *)(pData + 4)) ));
                    printf(" transmit : 0x%X\n", ntohl( *((unsigned int *)(pData + 8)) ));
                    break;
                case ICMP_TIMESTAMPREPLY:
                    printf("TIMESTAMP REPLY (data length %d)\n", dataLen);
                    printf(" originate: 0x%X\n", ntohl( *((unsigned int *)(pData    )) ));
                    printf(" receive  : 0x%X\n", ntohl( *((unsigned int *)(pData + 4)) ));
                    printf(" transmit : 0x%X\n", ntohl( *((unsigned int *)(pData + 8)) ));
                    break;
                case ICMP_ADDRESS:
                    printf("ADDRESS MASK REQUEST (data length %d)\n", dataLen);
                    break;
                case ICMP_ADDRESSREPLY:
                    printf("ADDRESS MASK REPLY (data length %d)\n", dataLen);
                    printf(" mask: %d.%d.%d.%d\n", pData[0], pData[1], pData[2], pData[3]);
                    break;
                default:
                    printf("ERR: unknown type %d\n", pIcmpHdr->type);
                    dump(pData, dataLen);
            }
            printf("\n");
        }
    }
}


void help(void)
{
    printf("Usage: icmp [OPTION...]\n");
    printf("\n");
    printf("  -i IP     IP address.\n");
    printf("  -t type   ICMP type.\n");
    printf("               0: ECHO REPLY\n");
    printf("               3: DESTINATION UNREACHABLE\n");
    printf("               4: SOURCE QUENCH\n");
    printf("               5: REDIRECT MESSAGE\n");
    printf("               8: ECHO REQUEST\n");
    printf("              11: TIME EXCEEDED\n");
    printf("              12: PARAMETER PROBLEM\n");
    printf("              13: TIMESTAMP\n");
    printf("              14: TIMESTAMP REPLY\n");
    printf("              17: ADDRESS MASK REQUEST\n");
    printf("              18: ADDRESS MASK REPLY\n");
    printf("  -c code   ICMP code.\n");
    printf("  -s        Send packet only (No receive).\n");
    printf("  -d        Debug mode enable.\n");
    printf("  -h        Show the help message.\n");
    printf("\n");
}

int main (int argc, char *argv[])
{
    char *pIpAddr = "127.0.0.1";
    int   type = ICMP_ECHO;
    int   code = 0;
    int   recv = 1;
    int   ch;


    opterr = 0;
    while ((ch=getopt(argc, argv, "i:t:c:sdh")) != -1)
    {
        switch ( ch )
        {
            case 'i':
                pIpAddr = optarg;
                break;
            case 't':
                type = atoi( optarg );
                break;
            case 'c':
                code = atoi( optarg );
                break;
            case 's':
                recv = 0;
                break;
            case 'd':
                g_debug = 1;
                break;
            case 'h':
            default:
                help();
                return 0;
        }
    }


    g_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (g_sock < 0)
    {
        perror( "socket (SOCK_RAW)" );
        return -1;
    }

    /* close the root permission */
    if (setuid( getuid() ) != 0)
    {
        perror( "setuid" );
        return -1;
    }


    sendICMP(pIpAddr, type, code);

    if ( recv )
    {
        recvICMP();
    }

    close( g_sock );

    return 0;
}

