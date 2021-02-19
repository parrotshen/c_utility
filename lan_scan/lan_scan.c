#include <sys/socket.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <netinet/in.h>  /* htons */


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define RX_BUF_SIZE  (4096)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

typedef struct _tLanHost
{
    /* store 1 to 8 MAC addresses per IP */
    unsigned char  mac[8][6];
    int            macNum;
} tLanHost;


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

static unsigned char  _txBuf[RX_BUF_SIZE];
static int            _txLen;
static unsigned char  _rxBuf[RX_BUF_SIZE];
static int            _rxLen;

static unsigned char  _macDst[6] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
static unsigned char  _macSrc[6];
static unsigned char  _ipDst[4];
static unsigned char  _ipSrc[4];
static unsigned char  _ipZero[4];

static int  _ifIndex = 0;  /* Ethernet Interface index */
static int  _sock = -1;

static tLanHost _hosts[256];


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

#if 0
void dump(char *pName, const void *pAddr, unsigned int size)
{
    unsigned char *pByte = (unsigned char *)pAddr;
    unsigned int   i;

    if (pByte == NULL)
    {
        printf("%s: NULL\n\n", pName);
        return;
    }

    printf("%s: %u bytes\n", pName, size);
    for (i=0; i<size; i++)
    {
        if ((i != 0) && ((i % 16) == 0))
        {
            printf("\n");
        }
        printf(" %02X", pByte[i]);
    }
    printf("\n\n");
}
#endif

int openSocket(char *pNetDev)
{
    struct ifreq ifr;
    unsigned int ip;
    int  fd = -1;
    int  i;

    /* open socket */
    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0)
    {
        perror( "socket()" );
        return -1;
    }

    /* retrieve ethernet interface index */
    strncpy(ifr.ifr_name, pNetDev, IFNAMSIZ);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0)
    {
        perror( "SIOCGIFINDEX" );
        close( fd );
        return -1;
    }
    _ifIndex = ifr.ifr_ifindex;

    /* retrieve corresponding MAC */
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
    {
        perror( "SIOCGIFHWADDR" );
        close( fd );
        return -1;
    }

    for (i=0; i<ETH_ALEN; i++)
    {
        _macSrc[i] = ifr.ifr_hwaddr.sa_data[i];
    }

    /* retrieve corresponding IP address */
    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
    {
        perror( "SIOCGIFADDR" );
        close( fd );
        return -1;
    }

    ip = ntohl( ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr );
    _ipSrc[0] = ((ip >> 24) & 0xFF);
    _ipSrc[1] = ((ip >> 16) & 0xFF);
    _ipSrc[2] = ((ip >>  8) & 0xFF);
    _ipSrc[3] = ((ip      ) & 0xFF);

    return fd;
}

void closeSocket(int fd)
{
    if (fd > 0)
    {
        close(fd);
    }
}

void sendArp(unsigned char *pIpDst)
{
    struct ethhdr *pEth = (struct ethhdr *)_txBuf;
    struct sockaddr_ll sockAddr;
    unsigned char *pBuf;
    int  retval;
    int  i = 0;


    /* prepare sockaddr_ll */
    sockAddr.sll_family   = AF_PACKET;
    sockAddr.sll_protocol = htons( ETH_P_IP );
    sockAddr.sll_ifindex  = _ifIndex;
    sockAddr.sll_hatype   = ARPHRD_ETHER;
    sockAddr.sll_pkttype  = PACKET_OTHERHOST;
    sockAddr.sll_halen    = ETH_ALEN;
    sockAddr.sll_addr[0]  = _macDst[0];
    sockAddr.sll_addr[1]  = _macDst[1];
    sockAddr.sll_addr[2]  = _macDst[2];
    sockAddr.sll_addr[3]  = _macDst[3];
    sockAddr.sll_addr[4]  = _macDst[4];
    sockAddr.sll_addr[5]  = _macDst[5];
    sockAddr.sll_addr[6]  = 0x00; 
    sockAddr.sll_addr[7]  = 0x00;


    /* prepare Ethernet header */
    memcpy(pEth->h_dest,   _macDst, ETH_ALEN);
    memcpy(pEth->h_source, _macSrc, ETH_ALEN);
    pEth->h_proto = htons( ETH_P_ARP );

    pBuf   = (_txBuf + ETH_HLEN);
    _txLen = ETH_HLEN;

    /* fill Ethernet payload data.... */
    pBuf[i++] = 0x00;
    pBuf[i++] = 0x01;
    pBuf[i++] = 0x08;
    pBuf[i++] = 0x00;
    pBuf[i++] = 6;
    pBuf[i++] = 4;
    pBuf[i++] = 0x00;
    pBuf[i++] = 0x01;
    pBuf[i++] = _macSrc[0];
    pBuf[i++] = _macSrc[1];
    pBuf[i++] = _macSrc[2];
    pBuf[i++] = _macSrc[3];
    pBuf[i++] = _macSrc[4];
    pBuf[i++] = _macSrc[5];
    #if 0
    pBuf[i++] = _ipSrc[0];
    pBuf[i++] = _ipSrc[1];
    pBuf[i++] = _ipSrc[2];
    pBuf[i++] = _ipSrc[3];
    #else
    pBuf[i++] = _ipZero[0];
    pBuf[i++] = _ipZero[1];
    pBuf[i++] = _ipZero[2];
    pBuf[i++] = _ipZero[3];
    #endif
    pBuf[i++] = 0x00;
    pBuf[i++] = 0x00;
    pBuf[i++] = 0x00;
    pBuf[i++] = 0x00;
    pBuf[i++] = 0x00;
    pBuf[i++] = 0x00;
    pBuf[i++] = pIpDst[0];
    pBuf[i++] = pIpDst[1];
    pBuf[i++] = pIpDst[2];
    pBuf[i++] = pIpDst[3];
    _txLen += i;


    /* send packet */
    retval = sendto(
                 _sock,
                 _txBuf,
                 _txLen,
                 0,
                 (struct sockaddr *)&sockAddr,
                 sizeof( sockAddr )
             );
    if (retval < 0)
    {
        perror( "sendto()" );
    }
    #if 0
    else
    {
        dump("TX", _txBuf, retval);
    }
    #endif
}

void *recvTask(void *pParam)
{
    struct ethhdr *pEth = (struct ethhdr *)_rxBuf;
    unsigned char *pArp;
    unsigned char *pMac;
    unsigned char *pIp;
    int found;
    int i;
    int j;

    while ( 1 )
    {
        /* wait for incoming packet... */    
        _rxLen = recvfrom(_sock, _rxBuf, RX_BUF_SIZE, 0, NULL, NULL);
        if (_rxLen < 0)
        {
            printf("EXIT: socket closed\n");
            break;
        }

        if (ETH_P_ARP == ntohs( pEth->h_proto ))
        {
            #if 0
            dump("RX", _rxBuf, _rxLen);
            #endif

            pArp = (_rxBuf + ETH_HLEN);
            pMac = (pArp + 8);
            pIp  = (pMac + 6);

            if ((pIp[0] == _ipDst[0]) &&
                (pIp[1] == _ipDst[1]) &&
                (pIp[2] == _ipDst[2]))
            {
                if (0 == _hosts[ pIp[3] ].macNum)
                {
                    /* display IP and MAC address */
                    memcpy(_hosts[ pIp[3] ].mac[0], pMac, 6);
                    _hosts[ pIp[3] ].macNum = 1;

                    printf(
                        "%u.%u.%u.%u\t%02x:%02x:%02x:%02x:%02x:%02x",
                        pIp[0], pIp[1], pIp[2], pIp[3],
                        pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]
                    );

                    if ((0 == memcmp(_ipSrc,  pIp,  4)) &&
                        (0 == memcmp(_macSrc, pMac, 6)))
                    {
                        printf(" *\n");
                    }
                    else
                    {
                        printf("\n");
                    }
                }
                else
                {
                    /* check whether IP address is conflicted */
                    found = 0;
                    for (i=0; i<_hosts[ pIp[3] ].macNum; i++)
                    {
                        if (0 == memcmp(_hosts[ pIp[3] ].mac[i], pMac, 6))
                        {
                            found = 1;
                            break;
                        }
                    }

                    if ( !found )
                    {
                        /* have a duplicated IP address */
                        j = _hosts[ pIp[3] ].macNum;
                        if (j < 8)
                        {
                            memcpy(_hosts[ pIp[3] ].mac[j], pMac, 6);
                            _hosts[ pIp[3] ].macNum++;
                        }

                        printf(
                            "%u.%u.%u.%u\t%02x:%02x:%02x:%02x:%02x:%02x",
                            pIp[0], pIp[1], pIp[2], pIp[3],
                            pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]
                        );
                        printf(" !\n");
                    }
                }
            }
        }
    }

    pthread_exit( NULL );
}

void sigint(int signum)
{
    closeSocket( _sock );
    _sock = -1;

    exit(0);
}

int main(int argc, char *argv[])
{
    pthread_t  thread;
    char *pLanIp  = "192.168.8.0";
    char *pNetDev = "eth0";
    int   start = 1;
    int   end = 254;
    int   retval;
    int   a, b, c, d;
    int   i;


    if (3 == argc)
    {
        pLanIp  = argv[1];
        pNetDev = argv[2];
    }
    else if (2 == argc)
    {
        pLanIp  = argv[1];
    }
    else
    {
        printf("Usage: lan_scan <LAN IP ADDRESS>\n");
        printf("     : lan_scan <LAN IP ADDRESS> <NETWORK INTERFACE>\n");
        printf("\n");
        printf("EX   : lan_scan 192.168.8.0\n");
        printf("\n");
        return 0;
    }

    /* establish signal handler */
    signal(SIGINT,  sigint);
    signal(SIGKILL, sigint);
    signal(SIGTERM, sigint);

    memset(_hosts, 0x00, (sizeof(tLanHost) * 256));
    memset(_ipZero, 0x0, 4);

    _sock = openSocket( pNetDev );
    if (_sock < 0)
    {
        printf("ERR: fail to create raw socket\n");
        return -1;
    }

    retval = pthread_create(&thread, NULL, recvTask, NULL);
    if ( retval )
    {
        printf("ERR: fail to create receive task\n");
        closeSocket( _sock );
        return -1;
    }

    sscanf(
        pLanIp,
        "%u.%u.%u.%u",
        &a, &b, &c, &d
    );
    _ipDst[0] = (a & 0xFF);
    _ipDst[1] = (b & 0xFF);
    _ipDst[2] = (c & 0xFF);
    _ipDst[3] = (d & 0xFF);

    printf("\n");
    printf("Scanning LAN ...\n");
    printf("\n");

    if (_ipDst[3] != 0)
    {
        start = end = _ipDst[3];
    }

    for (i=start; i<=end; i++)
    {
        _ipDst[3] = i;

        sendArp( _ipDst );

        usleep( 50000 );
    }


    getchar();

    closeSocket( _sock );
    _sock = -1;

    pthread_join(thread, NULL);

    return 0;
}

