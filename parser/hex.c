#include <stdio.h>
#include "libparser.h"

int main(int argc, char *argv[])
{
    unsigned char buf[128];
    char str[256];
    int  size;

    if (argc != 2)
    {
        printf("Usage: %s <HEX string>\n", argv[0]);
        printf("\n");
        return -1;
    }

    size = str2hex(argv[1], buf, 128);
    mem_dump("HEX", buf, size);

    hex2str(buf, size, str, 256);
    printf("%s\n", str);

    return 0;
}

