#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "util.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define PCAP_MAGIC_NUM       (0xA1B2C3D4)
#define PCAP_MAGIC_NUM_NANO  (0xA1B23C4D)  /* ns resolution */
#define PCAP_HEADER_LEN      24
#define PCAP_SUB_HEADER_LEN  16

#define MAX_BUF_SIZE  9000


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

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


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

int generate_pcap(int argc, char *argv[])
{
    unsigned char globalHdr[PCAP_HEADER_LEN];
    unsigned char packetHdr[PCAP_SUB_HEADER_LEN];
    pcap_hdr_t *pGlobalHdr = (pcap_hdr_t *)globalHdr;        
    pcaprec_hdr_t *pPacketHdr = (pcaprec_hdr_t *)packetHdr;

    FILE *pFile = NULL;
    int len;
    int i;


    pFile = fopen(argv[2], "wb");
    if (NULL == pFile)
    {
        printf("ERR: cannot open %s\n\n", argv[2]);
        return -1;
    }

    for (i=3; i<argc; i++)
    {
        if ((len=read_file(argv[i], g_data, MAX_BUF_SIZE)) == 0)
        {
            printf("ERR: cannot open %s\n\n", argv[i]);
            continue;
        }

        if (3 == i)
        {
            pGlobalHdr->magic_number  = PCAP_MAGIC_NUM;
            pGlobalHdr->version_major = 2;
            pGlobalHdr->version_minor = 4;
            pGlobalHdr->thiszone      = 0;
            pGlobalHdr->sigfigs       = 0;
            pGlobalHdr->snaplen       = 262144;
            pGlobalHdr->network       = atoi( argv[1] );
            fwrite(globalHdr, PCAP_HEADER_LEN, 1, pFile);
        }

        pPacketHdr->ts_sec   = 0;
        pPacketHdr->ts_usec  = 0;
        pPacketHdr->incl_len = len;
        pPacketHdr->orig_len = len;
        fwrite(packetHdr, PCAP_SUB_HEADER_LEN, 1, pFile);
        fwrite(g_data, len, 1, pFile);

        dump(g_data, len);
    }

    fclose( pFile );

    return 0;
}

int main(int argc, char *argv[])
{
    int rc;

    if (argc < 4)
    {
        printf("Usage: pcap_dlt_user DLT OUTPUT.pcap INPUT.txt ...\n\n");
        printf("   EX: pcap_dlt_user 150 nas-5gs.pcap nas-5gs_1.txt nas-5gs_2.txt nas-5gs_3.txt\n\n");
        return 0;
    }

    rc = generate_pcap(argc, argv);
    if (0 == rc)
    {
        printf("%s was generated\n\n", argv[2]);
    }

    return 0;
}

