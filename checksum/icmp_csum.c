#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "libparser.h"
#include "onec.h"


unsigned char g_icmpPacket[1500];

/*
*     0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7  0 1 2 3 4 5 6 7
*   +----------------/----------------/----------------/----------------+
* 0 | Type           | Code           | Checksum                        |
*   +----------------+----------------+---------------------------------+
* 4 | Rest of Header                                                    |
*   +----------------/----------------/----------------/----------------+
* 8 | Payload ...                                                       |
*   +----------------/----------------/----------------/----------------+
*/
#define ICMP_CSUM(data, dataLen) onec(NULL, 0, data, dataLen, 2)


int main(int argc, char *argv[])
{
    unsigned short csum;
    unsigned int   len;


    if (argc < 2)
    {
        printf("Usage: icmp_csum ICMP_PACKER.txt\n\n");
        return 0;
    }

    len = parse_hex_string_file(argv[1], g_icmpPacket, 1499);
    if (len & 0x1)
    {
        g_icmpPacket[len] = 0x00;
    }

    mem_dump("ICMP packet:", g_icmpPacket, len);

    csum = ICMP_CSUM(g_icmpPacket, len);

    printf("ICMP checksum: 0x%04X\n\n", csum);

    return 0;
}

