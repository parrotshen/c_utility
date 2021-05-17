#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


long filesize(char *pName)
{
    struct stat  stat_buf;

    if ( !stat(pName, &stat_buf) )
    {
        return stat_buf.st_size;
    }

    return 0;
}

void dump(void *pAddr, unsigned int size)
{
    unsigned char *pByte = pAddr;
    unsigned int  i;

    if (pAddr == NULL)
    {
        printf("NULL pointer\n\n");
        return;
    }

    for (i=0; i<size; i++)
    {
        if ((i != 0) && ((i % 16) == 0))
        {
            printf("\n");
        }

        printf(" %02X", pByte[i]);
    }
    printf("\n");
    printf(" (%u bytes)\n", size);
    printf("\n");
}

unsigned char *hexstr2byte(char *pStr, unsigned int *pLen)
{
    unsigned char *pBuf = NULL;
    unsigned char  val1 = 0;
    unsigned char  val2 = 0;
    unsigned int   bufSize = 0;
    unsigned int   i;
    unsigned int   j;
    int   ignore;
    char  ch;

    *(pLen) = 0;

    pBuf = malloc(((strlen( pStr ) + 1) / 2) + 1);
    if ( pBuf )
    {
        i = j = 0;
        while ( (ch = pStr[i]) )
        {
            ignore = 0;

            switch ( ch )
            {
                case '0' ... '9':
                    val1 = (ch - '0');
                    break;
                case 'a' ... 'f':
                    val1 = (((ch - 'a')) + 10);
                    break;
                case 'A' ... 'F':
                    val1 = (((ch - 'A')) + 10);
                    break;
                case 0x20:
                case '\t':
                    ignore = 1;
                    break;
                case '#':
                    /* '#' is the begin of comments */
                    goto _EXIT;
                    break;
                default:
                    printf("incorrect HEX string '%c'\n", ch);
                    free( pBuf );
                    return NULL;
            }

            if ( !ignore )
            {
                if (0 == (j % 2))
                {
                    val2 = (val1 << 4);
                }
                else
                {
                    val2 += val1;

                    pBuf[j / 2] = val2;
                    bufSize++;
                }

                j++;
            }

            i++;
        }

_EXIT:
        if ((j % 2) != 0)
        {
            printf("Hex string is not byte-aligned\n");
            free( pBuf );
            return NULL;
        }
    }

    *(pLen) = bufSize;
    return pBuf;
}

