#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned long long parse_hex_string(char *pHex)
{
    unsigned long long val = 0;
    int offset = 0;
    int i;

    if ((strlen(pHex) > 2) && ('0' == pHex[0]) && ('x' == pHex[1]))
    {
        offset = 2;
    }

    for (i=offset; i<strlen(pHex) && i<16; i++)
    {
        val <<= 4;

        switch ( pHex[i] )
        {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                val |= ((pHex[i] - '0') & 0xF);
                break;
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                val |= ((pHex[i] - 'W') & 0xF);
                break;
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                val |= ((pHex[i] - '7') & 0xF);
                break;
            default:
                printf("ERR: invalid HEX '%c'\n", pHex[i]);
        }
    }

    return val;
}

int main(int argc, char *argv[])
{
    char buf[32];
    unsigned long long addr;
    unsigned long long size;
    int number;
    int width[2];
    int i;

    /*
    * +-------------------------+ \
    * | ADDR                    | |
    * +-------------------------+ |
    * | ADDR + (SIZE * 1)       | |
    * +-------------------------+ |
    * | ADDR + (SIZE * 2)       | |
    * +-------------------------+ | NUMBER (N)
    * | ADDR + ...              | |
    * +-------------------------+ |
    * | ADDR + (SIZE * (N - 2)) | |
    * +-------------------------+ |
    * | ADDR + (SIZE * (N - 1)) | |
    * +-------------------------+ /
    */
    if (argc < 4)
    {
        printf("Usage: addrmap BASE_ADDR MEMORY_SIZE NUMBER\n");
        printf("               (HEX)     (HEX)       (DEC)\n\n");
        return 0;
    }

    addr = parse_hex_string( argv[1] );
    size = parse_hex_string( argv[2] );
    number = atoi( argv[3] );

    if (number > 0)
    {
        width[0] = sprintf(buf, "%d", (number - 1));
        width[1] = sprintf(buf, "%llx", (addr + (size * number)));

        for (i=0; i<number; i++)
        {
            printf("%*llX  %*d\n", width[1], addr, width[0], i);
            addr += size;
        }
        printf("%*llX\n", width[1], addr);
    }

    return 0;
}

