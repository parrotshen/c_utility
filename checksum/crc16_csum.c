#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "crc16.h"


#define MAX_FILE_SIZE (0x1000000)  // 16MB

static long query_file_size(char *pFileName)
{
    struct stat stat_buf;

    if ( !stat(pFileName, &stat_buf) )
    {
        return stat_buf.st_size;
    }

    return 0;
}

static void help(void)
{
    /* Test pattern: "123456789" -> 0xBB3D */
    printf("Usage: crc16_csum [OPTION]...\n");
    printf("\n");
    printf("  -p POLY      Polynomial (HEX).\n");
    printf("  -i INIT      Initial value (HEX).\n");
    printf("  -x XOROUT    XOR on output (HEX).\n");
    printf("  -r REFLECT   Input, output reflected (BOOL).\n");
    printf("  -s STRING    Input string.\n");
    printf("  -f FILE      Input file name.\n");
    printf("  -h           Show the help message.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    uint16  poly = 0x8005;
    uint16  init = 0x0000;
    uint16  xout = 0x0000;
    int     reflect = 1;
    uint32  hex;

    char *pFileName = NULL;
    char *pString = NULL;
    FILE *pInput  = NULL;
    long  fileSize;

    uint8  *pData = NULL;
    uint32  size = 0;
    uint16  crc = 0;
    int     ch;


    opterr = 0;
    while ((ch=getopt(argc, argv, "p:i:x:r:s:f:h")) != -1)
    {
        switch ( ch )
        {
            case 'p':
                sscanf(optarg, "%x", &hex);
                poly = (hex & 0xFFFF);
                break;
            case 'i':
                sscanf(optarg, "%x", &hex);
                init = (hex & 0xFFFF);
                break;
            case 'x':
                sscanf(optarg, "%x", &hex);
                xout = (hex & 0xFFFF);
                break;
            case 'r':
                reflect = atoi( optarg );
                break;
            case 'f':
                pFileName = optarg;
                break;
            case 's':
                pString = optarg;
                break;
            case 'h':
            default:
                help();
                return 0;
        }
    }

    if ( pFileName )
    {
        fileSize = query_file_size( pFileName );
        if ((fileSize == 0) || (fileSize > MAX_FILE_SIZE))
        {
            printf("ERR: illegal file size %ld\n\n", fileSize);
            return -1;
        }

        if ((pInput=fopen(pFileName, "r")) == NULL)
        {
            printf("ERR: fail to open %s\n\n", pFileName);
            return -1;
        }

        size = fileSize;
        pData = malloc( size );
        if ( pData )
        {
            memset(pData, 0x00, size);

            fread(pData, size, 1, pInput);

            crc = crc16(
                      poly,
                      init,
                      reflect,
                      reflect,
                      xout,
                      pData,
                      size
                  );

            free( pData );
        }

        fclose( pInput );
    }
    else if ( pString )
    {
        size = strlen( pString );
        crc = crc16(
                  poly,
                  init,
                  reflect,
                  reflect,
                  xout,
                  pString,
                  size
              );
    }
    else
    {
        help();
        return 0;
    }

    printf("CRC 0x%04X (size %u)\n\n", crc, size);

    return 0;
}

