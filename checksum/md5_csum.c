#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "md5.h"


#define DIV_CEIL(X, Y)  (((X) + ((Y) - 1)) / (Y))

unsigned char g_buf[4096];

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
    /* Test pattern: "123456789" -> 25f9e794323b453885f5181f1b624d0b */
    printf("Usage: md5_csum [OPTION]...\n");
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
    FILE *pInput = NULL;
    long fileSize;
    long loop;
    long i;

    MD5_CTX md5;
    unsigned char digest[16];
    int len;
    int ch;


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
        if (fileSize <= 0)
        {
            printf("ERR: file size is %ld\n\n", fileSize);
            return -1;
        }

        if ((pInput=fopen(pFileName, "r")) == NULL)
        {
            printf("ERR: fail to open %s\n\n", pFileName);
            return -1;
        }

        MD5Init( &md5 );
        loop = DIV_CEIL(fileSize, 4096);
        for (i=0; i<loop; i++)
        {
            memset(g_buf, 0x00, 4096);
            if ((loop - 1) == i)
            {
                len = (fileSize % 4096);
                if (0 == len) len = 4096;
            }
            else
            {
                len = 4096;
            }
            fread(g_buf, len, 1, pInput);

            MD5Update(&md5, g_buf, len);
        }
        MD5Final(&md5, digest);

        fclose( pInput );
    }
    else if ( pString )
    {
        MD5Init( &md5 );
        MD5Update(&md5, (unsigned char *)pString, strlen(pString));
        MD5Final(&md5, digest);
    }
    else
    {
        help();
        return 0;
    }

    printf("MD5 checksum:\n");
    for (i=0; i<16; i++)
    {
        printf("%02x", digest[i]);
    }
    printf("\n\n");


    return 0;
}

