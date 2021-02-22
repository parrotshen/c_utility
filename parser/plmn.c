#include <stdio.h>
#include "libparser.h"

int main(int argc, char *argv[])
{
    unsigned char buf[4];
    char str[16];
    int  size;

    if (argc != 2)
    {
        printf("Usage: %s <PLMN ID>\n", argv[0]);
        printf("\n");
        return -1;
    }

    size = str2plmn(argv[1], buf, sizeof( buf ));
    mem_dump("PLMN", buf, size);

    plmn2str(buf, size, str, 16);
    printf("%s\n", str);

    return 0;
}

