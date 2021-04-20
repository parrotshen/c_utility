#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "crc32.h"


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
    /* Test pattern: "123456789" -> 0xCBF43926 */
    printf("Usage: crc32_csum [OPTION]...\n");
    printf("\n");
    printf("  -f FILE      Input file name.\n");
    printf("  -s STRING    Input string.\n");
    printf("  -h           Show the help message.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    char *pFileName = NULL;
    char *pString = NULL;
    FILE *pInput  = NULL;
    long  fileSize;

    uint8  *pData = NULL;
    uint32  len = 0;
    uint32  crc = 0;
    int     ch;


    opterr = 0;
    while ((ch=getopt(argc, argv, "f:s:h")) != -1)
    {
        switch ( ch )
        {
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

        len = fileSize;
        pData = malloc( len );
        if ( pData )
        {
            memset(pData, 0x00, len);

            fread(pData, len, 1, pInput);

            crc = crc32(0, pData, len);

            free( pData );
        }

        fclose( pInput );
    }
    else if ( pString )
    {
        len = strlen( pString );
        crc = crc32(0, pString, len);
    }
    else
    {
        help();
        return 0;
    }

    printf("CRC 0x%08X (size %u)\n\n", crc, len);

    return 0;
}

