#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "libparser.h"


#define IP_HEADER_SIZE (20)

/*
*   0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7
* +--------+-------*------------+---*----------------*----------------+
* | Version| IHL   | DSCP       |ECN| Total Length                    |
* +--------+-------+------------+---+------+--------------------------+
* | Identification                  | Flags| Fragment Offset          |
* +----------------+----------------+------+--------------------------+
* | TTL            | Protocol       | Header Checksum                 |
* +----------------+----------------+---------------------------------+
* | Source IP Address                                                 |
* +-------------------------------------------------------------------+
* | Destination IP Address                                            |
* +----------------*----------------*----------------*----------------+
*/
static unsigned short ip_csum(unsigned char *pIpHdr)
{
    unsigned int csum = 0;
    unsigned short data;
    int i;

    for (i=0; i<IP_HEADER_SIZE; i+=2)
    {
        /* checksum is at pIpHdr[10-11] */
        if (i != 10)
        {
            data = ((pIpHdr[i] << 8) + pIpHdr[i + 1]);
            csum += data;
        }
    }

    csum = ((csum & 0xFFFF) + (csum >> 16));
    csum ^= 0xFFFF;

    return (unsigned short)csum;
}

int main(int argc, char *argv[])
{
    unsigned char  ipHdr[IP_HEADER_SIZE];
    unsigned int   ipHdrLen;
    unsigned short csum;


    if (argc < 2)
    {
        printf("Usage: ip_csum IP_HEADER.txt\n\n");
        return 0;
    }

    ipHdrLen = parse_hex_string_file(argv[1], ipHdr, IP_HEADER_SIZE);

    mem_dump("IP header:", ipHdr, ipHdrLen);

    csum = ip_csum( ipHdr );

    printf("IP checksum: 0x%04X\n\n", csum);

    return 0;
}

