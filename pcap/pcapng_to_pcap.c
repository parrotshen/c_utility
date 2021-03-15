#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "util.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define PCAPNG_ID_RESERVED  0
#define PCAPNG_ID_INTERFACE_DESCRIPTION_BLOCK  0x00000001
#define PCAPNG_ID_PACKET_BLOCK                 0x00000002
#define PCAPNG_ID_SIMPLE_PACKET_BLOCK          0x00000003
#define PCAPNG_ID_NAME_RESOLUTION_BLOCK        0x00000004
#define PCAPNG_ID_INTERFACE_STATISTICS_BLOCK   0x00000005
#define PCAPNG_ID_ENHANCED_PACKET_BLOCK        0x00000006
#define PCAPNG_ID_IRIG_TIMESTAMP_BLOCK         0x00000007
#define PCAPNG_ID_ARINC_429_BLOCK              0x00000008
#define PCAPNG_ID_SECTION_HEADER_BLOCK         0x0A0D0D0A


#define PCAP_HEADER_LEN      (sizeof(pcap_hdr_t))
#define PCAP_SUB_HEADER_LEN  (sizeof(pcaprec_hdr_t))

#define PCAP_MAGIC_NUM       (0xA1B2C3D4)
#define PCAP_MAGIC_NUM_NANO  (0xA1B23C4D)  /* ns resolution */

#define MAX_BUF_SIZE  4096


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

/* .pcap Global Header */
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

int g_ifLinkType[32];
int g_ifNum = 0;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////


void convert_to_pcap(
    FILE          *pOut,
    unsigned char *pData,
    unsigned int   inclLen,
    unsigned int   origLen,
    unsigned int   linkType,
    unsigned int   tsSec,
    unsigned int   tsUsec,
    int            first
)
{
    unsigned char globalHdr[PCAP_HEADER_LEN];
    unsigned char packetHdr[PCAP_SUB_HEADER_LEN];
    pcap_hdr_t *pGlobalHdr = (pcap_hdr_t *)globalHdr;        
    pcaprec_hdr_t *pPacketHdr = (pcaprec_hdr_t *)packetHdr;

    if ( first )
    {
        pGlobalHdr->magic_number  = PCAP_MAGIC_NUM;
        pGlobalHdr->version_major = 2;
        pGlobalHdr->version_minor = 4;
        pGlobalHdr->thiszone      = 0;
        pGlobalHdr->sigfigs       = 0;
        pGlobalHdr->snaplen       = 262144;
        pGlobalHdr->network       = linkType;

        fwrite(globalHdr, 1, PCAP_HEADER_LEN, pOut);
    }

    pPacketHdr->ts_sec   = tsSec;
    pPacketHdr->ts_usec  = tsUsec;
    pPacketHdr->incl_len = (inclLen);
    pPacketHdr->orig_len = (origLen);

    fwrite(packetHdr, 1, PCAP_SUB_HEADER_LEN, pOut);
    fwrite(pData,     1, inclLen, pOut);
}

int main(int argc, char *argv[])
{
    FILE *pIn = NULL;
    FILE *pOut = NULL;
    unsigned char *pBlock = NULL;
    unsigned int   bufferSize = 0;
    unsigned int   blockNum = 0;
    unsigned int   blockType;
    unsigned int   blockLen;
    unsigned int   temp;
    long  totalSize;
    int   first = 1;
    int   retval;


    if (argc != 3)
    {
        printf("Usage: pcapng_to_pcap INPUT.pcapng OUTPUT.pcap\n\n");
        return 0;
    }

    pIn = fopen(argv[1], "r");
    if (NULL == pIn)
    {
        printf("ERR: cannot open file %s\n\n", argv[1]);
        goto _EXIT;
    }

    totalSize = file_size( argv[1] );
    if (totalSize < 12)
    {
        printf("ERR: incomplete .pcapng file\n\n");
        goto _EXIT;
    }

    pOut = fopen(argv[2], "w");
    if (NULL == pOut)
    {
        printf("ERR: cannot open file %s\n\n", argv[2]);
        goto _EXIT;
    }

    /*
    *  General Block Structure
    *
    *                      1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
    *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    * |                          Block Type                           |
    * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    * |                      Block Total Length                       |
    * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ <- pBlock
    * /                          Block Body                           /
    * /              variable length, padded to 32 bits               /
    * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    * |                      Block Total Length                       |
    * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */

    printf("\n");
    while (totalSize > 0)
    {
        retval = fread(&blockType, 4, 1, pIn);
        if (retval != 1)
        {
            printf("ERR: fail to read block type\n\n");
            goto _EXIT;
        }
        if ((0 == blockNum) &&
            (PCAPNG_ID_SECTION_HEADER_BLOCK != blockType))
        {
            printf("ERR: not a .pcapng file\n\n");
            goto _EXIT;
        }

        blockNum++;

        retval = fread(&blockLen, 4, 1, pIn);
        if (retval != 1)
        {
            printf("ERR: fail to read block total length\n\n");
            goto _EXIT;
        }
        if (blockLen < 12)
        {
            printf("ERR: incorrect block total length (%u)\n\n", blockLen);
            goto _EXIT;
        }

        totalSize -= blockLen;

        if (bufferSize < blockLen)
        {
            bufferSize = blockLen;
            if ( pBlock )
            {
                free( pBlock );
                pBlock = NULL;
            }
        }

        if (NULL == pBlock)
        {
            pBlock = malloc( bufferSize );
            if (NULL == pBlock)
            {
                printf("ERR: fail to allocate memory\n\n");
                goto _EXIT;
            }
        }

        retval = fread(pBlock, (blockLen - 12), 1, pIn);
        if (retval != 1)
        {
            printf("ERR: fail to read block body\n\n");
            goto _EXIT;
        }

        switch ( blockType )
        {
            case PCAPNG_ID_INTERFACE_DESCRIPTION_BLOCK:
            {
                unsigned int linkType;

                printf(
                    "1: interface description block (%u bytes)\n",
                    blockLen
                );

                //dump(pBlock, (blockLen - 12));

                LE_BYTE_TO_UINT32(pBlock, linkType);
                if (g_ifNum < 32)
                {
                    g_ifLinkType[g_ifNum++] = linkType;
                }
                break;
            }
            case PCAPNG_ID_PACKET_BLOCK:
            {
                printf(
                    "2: packet block (%u bytes)\n",
                    blockLen
                );
                break;
            }
            case PCAPNG_ID_SIMPLE_PACKET_BLOCK:
            {
                printf(
                    "3: simple packet block (%u bytes)\n",
                    blockLen
                );
                break;
            }
            case PCAPNG_ID_NAME_RESOLUTION_BLOCK:
            {
                printf(
                    "4: name resolution block (%u bytes)\n",
                    blockLen
                );
                break;
            }
            case PCAPNG_ID_INTERFACE_STATISTICS_BLOCK:
            {
                printf(
                    "5: interface statistics block (%u bytes)\n",
                    blockLen
                );
                break;
            }
            case PCAPNG_ID_ENHANCED_PACKET_BLOCK:
            {
                unsigned char *pData;
                unsigned int id;
                unsigned long long timestamp;
                unsigned int capturedLen;
                unsigned int originalLen;
                unsigned int tsSec;
                unsigned int tsUsec;

                printf(
                    "6: enhanced packet block (%u bytes)\n",
                    blockLen
                );

                //dump(pBlock, (blockLen - 12));

                pData = pBlock;
                LE_BYTE_TO_UINT32(pData, id); pData += 4;
                LE_BYTE_TO_UINT32(pData, tsSec); pData += 4;
                LE_BYTE_TO_UINT32(pData, tsUsec); pData += 4;
                LE_BYTE_TO_UINT32(pData, capturedLen); pData += 4;
                LE_BYTE_TO_UINT32(pData, originalLen); pData += 4;

                timestamp = (((unsigned long long)tsSec << 32) |
                             ((unsigned long long)tsUsec));

                if (id < g_ifNum)
                {
                    tsSec = (timestamp / 1000000);
                    tsUsec = (timestamp % 1000000);

                    convert_to_pcap(
                        pOut,
                        pData,
                        capturedLen,
                        originalLen,
                        g_ifLinkType[id],
                        tsSec,
                        tsUsec,
                        first
                    );
                }

                if ( first ) first = 0;
                break;
            }
            case PCAPNG_ID_IRIG_TIMESTAMP_BLOCK:
            {
                printf(
                    "7: IRIG timestamp block (%u bytes)\n",
                    blockLen
                );
                break;
            }
            case PCAPNG_ID_ARINC_429_BLOCK:
            {
                printf(
                    "8: ARINC 429 block (%u bytes)\n",
                    blockLen
                );
                break;
            }
            case PCAPNG_ID_SECTION_HEADER_BLOCK:
            {
                unsigned int endian;

                printf(
                    "0x0A0D0D0A: section field block (%u bytes)\n",
                    blockLen
                );

                LE_BYTE_TO_UINT32(pBlock, endian);
                if (0x1A2B3C4D != endian)
                {
                    printf("ERR: not little endian (0x%x)\n\n", endian);
                    goto _EXIT;
                }
                break;
            }
            default:
            {
                printf(
                    "%d: unknown block type (%u bytes)\n",
                    blockType,
                    blockLen
                );
            }
        }

        retval = fread(&temp, 4, 1, pIn);
        if (retval != 1)
        {
            printf("ERR: fail to read block total length\n\n");
            goto _EXIT;
        }
        if (temp != blockLen)
        {
            printf("ERR: incorrect block total length (%u)\n\n", temp);
            goto _EXIT;
        }
    }
    printf("\n");

    if (totalSize < 0) printf("ERR: un-matched file size\n\n");
    if (0 == g_ifNum) printf("ERR: absent interface description block\n\n");


_EXIT:
    if ( pIn ) fclose( pIn );
    if ( pOut ) fclose( pOut );
    if ( pBlock ) free( pBlock );
    return 0;
}

