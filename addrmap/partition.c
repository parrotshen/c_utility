#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned long long parse_size(char *pStr)
{
    unsigned long long size;
    unsigned int unit;

    if ((strlen(pStr) > 2) && ('0' == pStr[0]) && ('x' == pStr[1]))
    {
        sscanf(pStr, "0x%llx", &size);
    }
    else
    {
        switch ( pStr[strlen(pStr) - 1] )
        {
            case 'k': case 'K':
                unit = 10;
                break;
            case 'm': case 'M':
                unit = 20;
                break;
            case 'g': case 'G':
                unit = 30;
                break;
            default:
                unit = 0;
        }

        size = atoll( pStr );
        size <<= unit;
    }

    return size;
}

void print_addr(unsigned long long addr, int width)
{
    static int table[16][16] = {
        /* 1 ~ 4 */
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 },
        /* 5 ~ 8 */
        { 0,0,0,0, 1,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 1,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 1,0,0,0, 0,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 1,0,0,0, 0,0,0,0, 0,0,0,0 },
        /* 9 ~ 12 */
        { 0,0,0,0, 1,0,0,0, 1,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 1,0,0,0, 1,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 1,0,0,0, 1,0,0,0, 0,0,0,0 },
        { 0,0,0,0, 1,0,0,0, 1,0,0,0, 0,0,0,0 },
        /* 13 ~ 16 */
        { 0,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0 },
        { 0,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0 },
        { 0,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0 },
        { 0,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0 }
    };
    char buf[64];
    int len;
    int i;
    int j;

    len = sprintf(buf, "%llX", addr);

    printf("0x");
    for (i=width, j=0; i>0; i--)
    {
        if (i > len)
        {
            printf("0");
        }
        else
        {
            printf("%c", buf[j++]);
        }

        if ( table[width-1][i-1] ) printf("_");
    }
}

int main(int argc, char *argv[])
{
    char buf[64];
    unsigned long long start = 0;
    unsigned long long size[2];
    unsigned long long addr;
    int index = 1;
    int width[2];
    int j;

    if (argc < 2)
    {
        printf("Usage: partition [-s START_ADDR] SIZE1 SIZE2 ...\n\n");
        return 0;
    }

    if (0 == strcmp("-s", argv[1]))
    {
        if (argc > 2)
        {
            start = parse_size( argv[2] );
        }
        else
        {
            printf("ERR: START_ADDR is missed after -s\n\n");
            return 0;
        }
        index = 3;
    }

    size[0] = 0;
    addr = start;
    for (j=index; j<argc; j++)
    {
        size[1] = parse_size( argv[j] );
        if (size[1] > size[0]) size[0] = size[1];
        if (j == (argc - 1)) break;
        addr += size[1];
    }
    width[0] = sprintf(buf, "%llx", addr);
    width[1] = sprintf(buf, "%llu", size[0]);

    addr = start;
    printf("\n");
    for (j=index; j<argc; j++)
    {
        size[1] = parse_size( argv[j] );

        print_addr(addr, width[0]);
        printf(" - ");
        print_addr((addr + size[1] - 1), width[0]);
        printf(" : %*llu bytes\n", width[1], size[1]);

        addr += size[1];
    }
    printf("\n");

    return 0;
}

