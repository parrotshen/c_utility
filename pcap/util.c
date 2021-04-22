#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define IS_UPPER_ALHPABET(ch)  ((ch >= 'A') && (ch <=  'Z'))
#define IS_LOWER_ALHPABET(ch)  ((ch >= 'a') && (ch <=  'z'))
#define IS_NUMBER(ch)          ((ch >= '0') && (ch <=  '9'))
#define IS_SPACE(ch)           ((ch == ' ') || (ch == '\t'))
#define LINE_SIZE   1023
#define TOKEN_SIZE  31


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void dump(void *pAddr, unsigned int len)
{
    unsigned char *pByte = pAddr;
    unsigned int i;

    if ( pByte )
    {
        for (i=0; i<len; i++)
        {
            if ((i != 0) && ((i % 16) == 0))
            {
                printf("\n");
            }
            printf(" %02X", pByte[i]);
        }
        printf("\n");
        printf(" (%u bytes)\n", len);
        printf("\n");
    }
}

long file_size(char *pIn)
{
    struct stat  stat_buf;

    if ( !stat(pIn, &stat_buf) )
    {
        return stat_buf.st_size;
    }

    return 0;
}

char *get_token(char *pLine, char *pToken, int tsize)
{
    int i = 0;

    if (0x0 == pLine[0])
    {
        /* This is a NULL line */
        pToken[0] = 0x0;
        return NULL;
    }

    /* Pass space and tab character */
    for (; *pLine && IS_SPACE(*pLine); pLine++);

    /* Get the separation token */
    for (; *pLine && !IS_SPACE(*pLine) && i<tsize; pLine++, i++)
    {
        *pToken++ = *pLine;
    }
    *pToken = 0x0;

    return pLine;
}

int read_line(FILE *pFile, char *pLine, int lsize)
{
    pLine[0] = 0x0;

    if ( feof(pFile) )
    {
        return 0;
    }

    /* char *fgets(                                   */
    /*     char *s,      // character array to store  */
    /*     int   n,      // length to read            */
    /*     FILE *stream  // FILE pointer              */
    /* );                                             */
    fgets(pLine, lsize, pFile);

    /* remove the CR/LF character */
    if ((strlen(pLine) > 0) && (pLine[strlen(pLine)-1] == 0x0a))
    {
        pLine[strlen(pLine)-1] = 0x0;
    }
    if ((strlen(pLine) > 0) && (pLine[strlen(pLine)-1] == 0x0d))
    {
        pLine[strlen(pLine)-1] = 0x0;
    }

    return 1;
}

unsigned int read_file(char *pIn, unsigned char *pBuf, unsigned int bsize)
{
    unsigned char *pByte = pBuf;
    unsigned int   count = 0;

    FILE *pFile = NULL;
    char  line[LINE_SIZE+1];
    char  token[TOKEN_SIZE+1];
    char *pNext;
    int   i;

    if ((pFile=fopen(pIn, "r")) == NULL)
    {
        printf("ERR: fail to open %s\n", pIn);
        return 0;
    }

    /* start reading input file */
    while ( read_line(pFile, line, LINE_SIZE) )
    {
        pNext = line;

        do
        {
            pNext = get_token(pNext, token, TOKEN_SIZE);
            if ((0x0 == token[0]) || ('#' == token[0]))
            {
                /* ignore the comment and null line */
                break;
            }

            /* get the token and transfer to one byte */
            sscanf(token, "%x", &i);
            if ((count + 1) > bsize)
            {
                printf("ERR: buffer size(%u) is un-enough\n", bsize);
                goto _EXIT_READ_FILE;
            }

            *pByte++ = (i & 0xFF);
            count++;
        } while ( pNext );
    }

_EXIT_READ_FILE:
    fclose( pFile );
    return count;
}

