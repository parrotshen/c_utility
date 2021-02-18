#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "util.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define PCAP_HEADER_LEN      (sizeof(pcap_hdr_t))
#define PCAP_SUB_HEADER_LEN  (sizeof(pcaprec_hdr_t))

#define PCAP_MAGIC_NUM       (0xA1B2C3D4)
#define PCAP_MAGIC_NUM_NANO  (0xA1B23C4D)  /* ns resolution */

#define MAX_BUF_SIZE  4096


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

/* Global Header */
typedef struct pcap_hdr_s {
    unsigned int    magic_number;   /* magic number */
    unsigned short  version_major;  /* major version number */
    unsigned short  version_minor;  /* minor version number */
    int             thiszone;       /* GMT to local correction */
    unsigned int    sigfigs;        /* accuracy of timestamps */
    unsigned int    snaplen;        /* max length of captured packets, in octets */
    unsigned int    network;        /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
    unsigned int    ts_sec;         /* timestamp seconds */
    unsigned int    ts_usec;        /* timestamp microseconds */
    unsigned int    incl_len;       /* number of octets of packet saved in file */
    unsigned int    orig_len;       /* actual length of packet */
} pcaprec_hdr_t;


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

unsigned char g_data[MAX_BUF_SIZE];
unsigned int  g_dataLen;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void foo_example(void)
{
    char *pOut = "foo_example.pcap";
    FILE *pFile = NULL;

    unsigned char globalHdr[PCAP_HEADER_LEN];
    unsigned char packetHdr[PCAP_SUB_HEADER_LEN];

    unsigned char eth[14] = {
        0xfe, 0xfe, 0xfe, 0x00, 0x00, 0x00,
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
        0x08, 0x00  /* IP */
    };
    int ethLen = sizeof( eth );
    unsigned char ip[20] = {
        0x45, 0x00,
        0x00, 0x24,  /* total length (20 + 16) */
        0x04, 0xd2,  /* identification */
        0x00, 0x00,
        0x01, 0x11,  /* UDP */
        0x00, 0x00,  /* checksum */
        192, 168, 3, 100,  /* source IP address */
        192, 168, 3, 200   /* destination IP address */
    };
    int ipLen = sizeof( ip );
    unsigned char udp[8] = {
        0xd9, 0x03,  /* source port number */
        0xd9, 0x03,  /* destination port number */
        0x00, 0x10,  /* length (8 + 8) */
        0x00, 0x00   /* checksum */
    };
    int udpLen = sizeof( udp );
    unsigned char foo[8] = {
        0x01,
        0x01,
        0x03, 0xe7,
        0x0a, 0x0a, 0x0a, 0x02
    };
    int fooLen = sizeof( foo );
    int totalLen = (ethLen + ipLen + udpLen + fooLen);


    pFile = fopen(pOut, "w");
    if (NULL == pFile)
    {
        printf("ERR: fail to open %s\n", pOut);
        return;
    }

    pcap_hdr_t *pGlobalHdr = (pcap_hdr_t *)globalHdr;        
    pcaprec_hdr_t *pPacketHdr = (pcaprec_hdr_t *)packetHdr;

    pGlobalHdr->magic_number  = PCAP_MAGIC_NUM;
    pGlobalHdr->version_major = 2;
    pGlobalHdr->version_minor = 4;
    pGlobalHdr->thiszone      = 0;
    pGlobalHdr->sigfigs       = 0;
    pGlobalHdr->snaplen       = 262144;
    pGlobalHdr->network       = 1;  /* Ethernet */

    pPacketHdr->ts_sec   = 0;
    pPacketHdr->ts_usec  = 0;
    pPacketHdr->incl_len = (totalLen);
    pPacketHdr->orig_len = (totalLen);

    fwrite(globalHdr, 1, PCAP_HEADER_LEN,     pFile);
    fwrite(packetHdr, 1, PCAP_SUB_HEADER_LEN, pFile);
    fwrite(eth,       1, ethLen, pFile);
    fwrite(ip,        1, ipLen,  pFile);
    fwrite(udp,       1, udpLen, pFile);
    fwrite(foo,       1, fooLen, pFile);
    
    fclose( pFile );        
}

int generate_pcap(
    char          *pOut,
    unsigned char *pData,
    unsigned int   dataLen,
    unsigned int   user,
    int            append
)
{
    FILE *pFile = NULL;

    unsigned char globalHdr[PCAP_HEADER_LEN];
    unsigned char packetHdr[PCAP_SUB_HEADER_LEN];


    if ( !append )
    {
        pFile = fopen(pOut, "w");
        if (NULL == pFile)
        {
            printf("ERR: fail to open %s\n", pOut);
            return 0;
        }

        pcap_hdr_t *pGlobalHdr = (pcap_hdr_t *)globalHdr;        
        pcaprec_hdr_t *pPacketHdr = (pcaprec_hdr_t *)packetHdr;

        pGlobalHdr->magic_number  = PCAP_MAGIC_NUM;
        pGlobalHdr->version_major = 2;
        pGlobalHdr->version_minor = 4;
        pGlobalHdr->thiszone      = 0;
        pGlobalHdr->sigfigs       = 0;
        pGlobalHdr->snaplen       = 262144;
        pGlobalHdr->network       = ((0 == user) ? 1 : user);

        pPacketHdr->ts_sec   = 0;
        pPacketHdr->ts_usec  = 0;
        pPacketHdr->incl_len = (dataLen);
        pPacketHdr->orig_len = (dataLen);

        fwrite(globalHdr, 1, PCAP_HEADER_LEN,     pFile);
        fwrite(packetHdr, 1, PCAP_SUB_HEADER_LEN, pFile);
        fwrite(pData,     1, dataLen, pFile);

        fclose( pFile );        
    }
    else
    {
        pFile = fopen(pOut, "a+");
        if (NULL == pFile)
        {
            printf("ERR: fail to open %s\n", pOut);
            return 0;
        }

        pcaprec_hdr_t *pPacketHdr = (pcaprec_hdr_t *)packetHdr;

        pPacketHdr->ts_sec   = 0;
        pPacketHdr->ts_usec  = 0;
        pPacketHdr->incl_len = (dataLen);
        pPacketHdr->orig_len = (dataLen);

        fwrite(packetHdr, 1, PCAP_SUB_HEADER_LEN, pFile);
        fwrite(pData,     1, dataLen, pFile);

        fclose( pFile );        
    }

    return 1;
}

void help(void)
{
    printf("Usage: pcap_gen [OPTION]...\n");
    printf("\n");
    printf("  -i INPUT    Input .txt file.\n");
    printf("  -o OUTPUT   Output .pcap file.\n");
    printf("  -u DLT      DLT User number (147 ~ 162).\n");
    printf("  -a          Append to an existed .pcap file.\n");
    printf("  -t          Generate a .pcap example.\n");
    printf("  -h          Show the help message.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    char *pOut = "pcap_gen.pcap";
    char *pIn = NULL;
    int user = 1; /* Ethernet */
    int append = 0;
    int retval;
    int ch;


    if (1 == argc)
    {
        help();
        return 0;
    }

    opterr = 0;
    while ((ch=getopt(argc, argv, "i:o:u:ath")) != -1)
    {
        switch ( ch )
        {
            case 'i':
                pIn = optarg;
                break;
            case 'o':
                pOut = optarg;
                break;
            case 'u':
                user = atoi( optarg );
                if ((user < 147) || (user > 162))
                {
                    printf("ERR: wront DLT User number %s\n", optarg);
                    return -1;
                }
                break;
            case 'a':
                append = 1;
                break;
            case 't':
                foo_example();
                return 0;
                break;
            case 'h':
            default:
                help();
                return 0;
        }
    }

    if (NULL == pIn)
    {
        printf("ERR: no specified input file\n");
        return -1;
    }

    g_dataLen = read_file(pIn, g_data, MAX_BUF_SIZE);
    if (0 == g_dataLen)
    {
        printf("ERR: fail to read %s\n", pIn);
        return -1;
    }

    retval = generate_pcap(pOut, g_data, g_dataLen, user, append);
    if (retval > 0)
    {
        printf("%s (packet %d bytes)\n", pOut, g_dataLen);
    }

    return 0;
}

