#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "libparser.h"
#include "onec.h"


/*
*      0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7
*    +--------+-------/------------+---/----------------/----------------+
*  0 | Version| IHL   | DSCP       |ECN| Total Length                    |
*    +--------+-------+------------+---+------+--------------------------+
*  4 | Identification                  | Flags| Fragment Offset          |
*    +----------------+----------------+------+--------------------------+
*  8 | TTL            | Protocol       | Header Checksum                 |
*    +----------------+----------------+---------------------------------+
* 12 | Source IP Address                                                 |
*    +-------------------------------------------------------------------+
* 16 | Destination IP Address                                            |
*    +----------------/----------------/----------------/----------------+
* 20 | Options ...                                                       |
*    +----------------/----------------/----------------/----------------+
*/
#define IP_CSUM(data, dataLen) onec(NULL, 0, data, dataLen, 10)


int main(int argc, char *argv[])
{
    unsigned char  ipHdr[60];
    unsigned int   ipHdrLen;
    unsigned short csum;


    if (argc < 2)
    {
        printf("Usage: ip_csum IP_HEADER.txt\n\n");
        return 0;
    }

    ipHdrLen = parse_hex_string_file(argv[1], ipHdr, 60);
    if (ipHdrLen & 0x1)
    {
        ipHdr[ipHdrLen] = 0x00;
    }

    mem_dump("IP header:", ipHdr, ipHdrLen);

    csum = IP_CSUM(ipHdr, ipHdrLen);

    printf("IP checksum: 0x%04X\n\n", csum);

    return 0;
}

