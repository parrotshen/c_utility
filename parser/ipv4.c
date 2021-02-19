#include <stdio.h>
#include "libparser.h"

int main(int argc, char *argv[])
{
    unsigned char buf[16];
    char str[64];
    int  size;

    if (argc != 2)
    {
        printf("Usage: %s <IPv4 address>\n", argv[0]);
        printf("\n");
        return -1;
    }

    size = str2ip(4, argv[1], buf);
    mem_dump("IPv4", buf, size);

    ip2str(4, buf, str, 64);
    printf("%s\n", str);

    return 0;
}

