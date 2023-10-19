#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utility.h"


int main(int argc, char *argv[])
{
    FILE *pFileIn;
    unsigned char  byte;

    unsigned int   totalLen;
    unsigned int   offset;

    unsigned char *pPattern;
    unsigned int   patternLen;
    int  matched = 0;
    int  next;
    int  count;
    int  i;


    if (argc != 3)
    {
        printf("Usage: binfind FILE PATTERN\n");
        printf("\n");
        return 0;
    }

    totalLen = filesize( argv[1] );
    offset   = 0;

    if ((pFileIn=fopen(argv[1], "r")) == NULL)
    {
        printf("ERR: cannot open %s\n", argv[1]);
        return -1;
    }

    pPattern = hexstr2byte(argv[2], &patternLen);
    if ( pPattern )
    {
        if (patternLen > 0)
        {
            //dump(pPattern, patternLen);

            next  = 0;
            count = 1;
            for (i=0; i<totalLen; i++)
            {
                if (fread(&byte, 1, 1, pFileIn) != 1) break;

                if (byte == pPattern[next])
                {
                    //printf("%02x, %02x, %d\n", i, byte, next);
                    if (next == 0)
                    {
                        matched = 1;
                        offset = i;
                    }
                    
                    next++;
                    if (next == patternLen)
                    {
                        matched = 0;
                        printf("#%d found at 0x%X\n", count, offset);
                        next = 0;
                        count++;
                    }
                }
                else
                {
                    if ( matched )
                    {
                        matched = 0;
                        i = offset;
                        fseek(pFileIn, (i + 1), SEEK_SET);
                    }
                    next = 0;
                }
            }
        }

        free( pPattern );
    }

    fclose( pFileIn );

    return 0;
}

