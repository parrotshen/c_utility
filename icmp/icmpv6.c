#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <time.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include <errno.h>


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define ICMPV6_MAX_LEN  (1500)
#define ICMPV6_HDR_LEN  sizeof(struct icmp6_hdr)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

unsigned char  g_txBuff[ICMPV6_MAX_LEN];
unsigned char  g_rxBuff[ICMPV6_MAX_LEN];
int  g_sock = -1;
int  g_debug = 0;

unsigned char  g_packet[80] = {
    /* IPv6 header (40) */
    0x60, 0x00, 0x00, 0x00, 0x00, 0x28, 0x3a, 0x80,
    0x20, 0x01, 0x0d, 0xb8, 0x00, 0x01, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x05,
    0x20, 0x01, 0x0d, 0xb8, 0x00, 0x01, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,

    /* ICMPv6 header (8) */
    0x80, 0x00, 0x59, 0x65, 0x00, 0x01, 0x00, 0x16,

    /* ICMPv6 payload (32) */
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x61,
    0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69
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

int sendICMPv6(char *pIpAddr, int type, int code)
{
    int retval;
    struct sockaddr_in6 toAddr;
    struct icmp6_hdr *pIcmp;
    struct nd_opt_hdr *pOpt;
    unsigned char *pData;
    int toAddrLen;
    int size = ICMPV6_HDR_LEN;


    /*           0      1      2      3
     * pIcmp -> +------+------+------+------+ \
     *          | Type | Code | Checksum    | |
     *          +------+------+------+------+ | ICMPv6 header
     *          | Message Body              | |
     * pData -> +------+------+------+------+ /
     *          |                           |
     *         ...                         ...
     *          |                           |
     *          +------+------+------+------+
     */
    memset(g_txBuff, 0, ICMPV6_HDR_LEN);

    pIcmp = (struct icmp6_hdr *)g_txBuff;
    pIcmp->icmp6_type = type;

    pData = (g_txBuff + ICMPV6_HDR_LEN);

    switch ( type )
    {
        case ICMP6_DST_UNREACH:
            pIcmp->icmp6_code = code;
            memcpy(pData, g_packet, 80);
            size = (ICMPV6_HDR_LEN + 80);
            break;
        case ICMP6_PACKET_TOO_BIG:
            pIcmp->icmp6_mtu = htonl(1500);
            memcpy(pData, g_packet, 80);
            size = (ICMPV6_HDR_LEN + 80);
            break;
        case ICMP6_TIME_EXCEEDED:
            pIcmp->icmp6_code = code;
            memcpy(pData, g_packet, 80);
            size = (ICMPV6_HDR_LEN + 80);
            break;
        case ICMP6_PARAM_PROB:
            pIcmp->icmp6_code = code;
            pIcmp->icmp6_pptr = htonl(5);
            memcpy(pData, g_packet, 80);
            size = (ICMPV6_HDR_LEN + 80);
            break;
        case ICMP6_ECHO_REQUEST:
            pIcmp->icmp6_id = htons( getpid() );
            pIcmp->icmp6_seq = htons( (unsigned short)time(NULL) );
            memset(pData, 0xFF, 56);
            size = (ICMPV6_HDR_LEN + 56);
            break;
        case ICMP6_ECHO_REPLY:
            pIcmp->icmp6_id = htons( getpid() );
            pIcmp->icmp6_seq = htons( (unsigned short)time(NULL) );
            memset(pData, 0xFF, 56);
            size = (ICMPV6_HDR_LEN + 56);
            break;
        case ND_ROUTER_SOLICIT:
            pOpt = (struct nd_opt_hdr *)pData;
            pOpt->nd_opt_type = 1;
            pOpt->nd_opt_len = 1;
            pData[2] = 0x00;
            pData[3] = 0x23;
            pData[4] = 0x24;
            pData[5] = 0x37;
            pData[6] = 0x7B;
            pData[7] = 0xFC;
            size = (ICMPV6_HDR_LEN + 8);
            break;
        case ND_ROUTER_ADVERT:
            pIcmp->icmp6_data8[0] = 64;
            pIcmp->icmp6_data8[1] = 0xC0;
            pIcmp->icmp6_data16[1] = htons(20);
            *((unsigned int *)&pData[0]) = htonl(100);
            *((unsigned int *)&pData[4]) = htonl(100);
            size = (ICMPV6_HDR_LEN + 8);
            break;
        case ND_NEIGHBOR_SOLICIT:
            inet_pton(AF_INET6, "2001:DB8::123", &pData[0]);
            pOpt = (struct nd_opt_hdr *)&pData[16];
            pOpt->nd_opt_type = 1;
            pOpt->nd_opt_len = 1;
            pData[18] = 0x00;
            pData[19] = 0x23;
            pData[20] = 0x24;
            pData[21] = 0x37;
            pData[22] = 0x7B;
            pData[23] = 0xFC;
            size = (ICMPV6_HDR_LEN + 24);
            break;
        case ND_NEIGHBOR_ADVERT:
            pIcmp->icmp6_data8[0] = 0x60;
            inet_pton(AF_INET6, "2001:DB8::123", &pData[0]);
            size = (ICMPV6_HDR_LEN + 16);
            break;
        case ND_REDIRECT:
            inet_pton(AF_INET6, "2001:DB8::123", &pData[0]);
            inet_pton(AF_INET6, "2001:DB9::1", &pData[16]);
            pOpt = (struct nd_opt_hdr *)&pData[32];
            pOpt->nd_opt_type = 1;
            pOpt->nd_opt_len = 1;
            pData[34] = 0x00;
            pData[35] = 0x23;
            pData[36] = 0x24;
            pData[37] = 0x37;
            pData[38] = 0x7B;
            pData[39] = 0xFC;
            size = (ICMPV6_HDR_LEN + 40);
            break;
        default:
            printf("%s: incorrect type %d\n\n", __func__, type);
            return -1;
    }

    pIcmp->icmp6_cksum = checksum(pIcmp, size);

    /* destination address */
    toAddrLen = sizeof( struct sockaddr_in6 );
    bzero(&toAddr, toAddrLen);
    toAddr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, pIpAddr, &(toAddr.sin6_addr));

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

void recvICMPv6(void)
{
    char ipv6Str[INET6_ADDRSTRLEN];
    struct sockaddr_in6  fromAddr;
    socklen_t  fromAddrLen;
    struct icmp6_hdr *pIcmpHdr;
    unsigned char *pData;
    int  dataLen;
    int  size;


    /* source address */
    fromAddrLen = sizeof( struct sockaddr_in6 );
    bzero(&fromAddr, fromAddrLen);

    size = recvfrom(
               g_sock,
               g_rxBuff,
               ICMPV6_MAX_LEN,
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

        pIcmpHdr = (struct icmp6_hdr *)g_rxBuff;
        pData = (g_rxBuff + ICMPV6_HDR_LEN);
        dataLen = (size - ICMPV6_HDR_LEN);

        inet_ntop(AF_INET6, &fromAddr.sin6_addr, ipv6Str, INET6_ADDRSTRLEN);
        printf("%d bytes <- %s\n", size, ipv6Str);
        if ( g_debug )
        {
            dump(g_rxBuff, size);
        }
        printf("\n");

        switch ( pIcmpHdr->icmp6_type )
        {
            case ICMP6_DST_UNREACH:
                printf("DESTINATION UNREACHABLE (data length %d)\n", dataLen);
                printf(" code: %u\n", pIcmpHdr->icmp6_code);
                break;
            case ICMP6_PACKET_TOO_BIG:
                printf("PACKET TOO BIG (data length %d)\n", dataLen);
                printf(" MTU: %d\n", ntohs(pIcmpHdr->icmp6_mtu));
                break;
            case ICMP6_TIME_EXCEEDED:
                printf("TIME EXCEEDED (data length %d)\n", dataLen);
                printf(" code: %u\n", pIcmpHdr->icmp6_code);
                break;
            case ICMP6_PARAM_PROB:
                printf("PARAMETER PROBLEM (data length %d)\n", dataLen);
                printf(" code: %u\n", pIcmpHdr->icmp6_code);
                break;
            case ICMP6_ECHO_REQUEST:
                printf("ECHO REQUEST (data length %d)\n", dataLen);
                dump(pData, dataLen);
                break;
            case ICMP6_ECHO_REPLY:
                printf("ECHO REPLY (data length %d)\n", dataLen);
                dump(pData, dataLen);
                break;
            case ND_ROUTER_SOLICIT:
                printf("ROUTER SOLICITATION (data length %d)\n", dataLen);
                break;
            case ND_ROUTER_ADVERT:
                printf("ROUTER ADVERTISEMENT (data length %d)\n", dataLen);
                break;
            case ND_NEIGHBOR_SOLICIT:
                printf("NEIGHBOR SOLICITATION (data length %d)\n", dataLen);
                break;
            case ND_NEIGHBOR_ADVERT:
                printf("NEIGHBOR ADVERTISEMENT (data length %d)\n", dataLen);
                break;
            case ND_REDIRECT:
                printf("REDIRECT MESSAGE (data length %d)\n", dataLen);
                break;
            default:
                printf("ERR: unknown type %d\n", pIcmpHdr->icmp6_type);
                dump(pData, dataLen);
        }
        printf("\n");
    }
}


void help(void)
{
    printf("Usage: icmpv6 [OPTION...]\n");
    printf("\n");
    printf("  -I DEV    Network device.\n");
    printf("  -i IP     IPv6 address.\n");
    printf("  -t TYPE   ICMPv6 type.\n");
    printf("               1: DESTINATION UNREACHABLE\n");
    printf("               2: PACKET TOO BIG\n");
    printf("               3: TIME EXCEEDED\n");
    printf("               4: PARAMETER PROBLEM\n");
    printf("             128: ECHO REQUEST\n");
    printf("             129: ECHO REPLY\n");
    printf("             133: ROUTER SOLICITATION\n");
    printf("             134: ROUTER ADVERTISEMENT\n");
    printf("             135: NEIGHBOR SOLICITATION\n");
    printf("             136: NEIGHBOR ADVERTISEMENT\n");
    printf("             137: REDIRECT MESSAGE\n");
    printf("  -c CODE   ICMPv6 code.\n");
    printf("  -s        Send packet only (No receive).\n");
    printf("  -d        Debug mode enable.\n");
    printf("  -h        Show the help message.\n");
    printf("\n");
}

int main (int argc, char *argv[])
{
    char *pIpAddr = "::1";
    int   type = ICMP6_ECHO_REQUEST;
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


    g_sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (g_sock < 0)
    {
        perror( "socket (SOCK_RAW)" );
        return -1;
    }

    /* close the root permission */
    if (setuid( getuid() ) != 0)
    {
        perror( "setuid" );
        close( g_sock );
        return -1;
    }


    sendICMPv6(pIpAddr, type, code);

    if ( recv )
    {
        recvICMPv6();
    }

    close( g_sock );

    return 0;
}

