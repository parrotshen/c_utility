#include "parser.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

/**
 * Convert HEX to DEC.
 * @param [in]  ch  HEX character (0 ~ F).
 * @returns  Integer 0 ~ 15.
 */
static unsigned char _hex2dec(char ch)
{
    unsigned char val = 0;

    switch ( ch )
    {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            val = (ch - '0');
            break;
        case 'a': case 'A':
            val = 10;
            break;
        case 'b': case 'B':
            val = 11;
            break;
        case 'c': case 'C':
            val = 12;
            break;
        case 'd': case 'D':
            val = 13;
            break;
        case 'e': case 'E':
            val = 14;
            break;
        case 'f': case 'F':
            val = 15;
            break;
        default:
            ;
    }

    return val;
}

/**
 * Get the first token from a string.
 * @param [in]   pString  Input string.
 * @param [out]  pToken   Output token.
 * @param [in]   tsize    Output buffer size.
 * @returns  String after remove the first token.
 */
char *parse_token(char *pString, char *pToken, int tsize)
{
    int i = 0;

    if (0x0 == pString[0])
    {
        /* This is a NULL line */
        pToken[0] = 0x0;
        return NULL;
    }

    /* Bypass space and tab characters */
    for (; *pString && IS_SPACE(*pString); pString++);

    /* Pull out a token */
    for (; *pString && !IS_SPACE(*pString) && i<tsize; pString++, i++)
    {
        *pToken++ = *pString;
    }
    *pToken = 0x0;

    return pString;
}

/**
 * Read one line from a text file.
 * @param [in]   pFile  Input file pointer.
 * @param [out]  pLine  Output buffer.
 * @param [in]   lsize  Output buffer size.
 * @returns  Success(line length) or failure(-1).
 */
int parse_line(FILE *pFile, char *pLine, int lsize)
{
    pLine[0] = 0x0;

    if ( feof(pFile) )
    {
        /* end of file */
        return -1;
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

    return strlen(pLine);
}

/**
 * Parse a HEX string file to byte array.
 * @param [in]   pFileName  Input file name.
 * @param [out]  pBuf       Byte array buffer.
 * @param [in]   bufSize    Byte array buffer size.
 * @returns  Data length.
 */
unsigned int parse_hex_string_file(
    char          *pFileName,
    unsigned char *pBuf,
    unsigned int   bufSize
)
{
    FILE *pInput = NULL;
    char  line[LINE_SIZE+1];
    char  token[TOKEN_SIZE+1];
    char *pNext;

    unsigned char  nibbleH;
    unsigned char  nibbleL;
    unsigned int   len;
    int  more;


    if ((pInput=fopen(pFileName, "r")) == NULL)
    {
        printf("%s: cannot open file %s\n", __func__, pFileName);
        return 0;
    }

    /* start reading input file */
    len = 0;
    do
    {
        if ((more=parse_line(pInput, line, LINE_SIZE)) > 0)
        {
            pNext = line;
            do
            {
                pNext = parse_token(pNext, token, TOKEN_SIZE);
                if ((0x0 == token[0]) || ('#' == token[0]))
                {
                    /* ignore the null or comment line */
                    break;
                }

                if (len >= bufSize)
                {
                    printf("%s: un-enough buffer size (%u)\n", __func__, bufSize);
                    goto _EXIT_PARSE;
                }

                if (strlen( token ) > 2)
                {
                    printf("%s: wrong HEX token (%s)\n", __func__, token);
                    goto _EXIT_PARSE;
                }
                else if (strlen( token ) > 1)
                {
                    nibbleH = _hex2dec( token[0] );
                    nibbleL = _hex2dec( token[1] );

                    pBuf[len++] = ((nibbleH << 4) | (nibbleL));
                }
                else
                {
                    pBuf[len++] = _hex2dec( token[0] );
                }
            } while ( pNext );
        }
    } while (more != -1);

_EXIT_PARSE:
    fclose( pInput );

    return len;
}

/**
 * Dump memory.
 * @param [in]  pDesc  Description string.
 * @param [in]  pAddr  Memory address.
 * @param [in]  size   Memory size.
 */
void mem_dump(char *pDesc, void *pAddr, unsigned int size)
{
    unsigned char *pByte = pAddr;
    unsigned int i = 0;

    if (pAddr == NULL)
    {
        fprintf(stderr, "%s (NULL)\n", pDesc);
        fprintf(stderr, "\n");
        return;
    }

    fprintf(stderr, "%s\n", pDesc);
    for (i=0; i<size; i++)
    {
        if ((i != 0) && ((i % 16) == 0))
        {
            fprintf(stderr, "\n");
        }
        fprintf(stderr, " %02X", pByte[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, " (%u bytes)\n", size);
    fprintf(stderr, "\n");
}

/**
 * Parse IPv6 address string.
 * @param [in]   src  IPv6 address string.
 * @param [out]  dst  IPv6 address buffer.
 * @returns  Success(0) or failure(-1).
 */
static int _parseIpv6Str(const char *src, unsigned char *dst)
{
#define NS_IN6ADDRSZ  16
#define NS_INT16SZ    2
    static const char xdigits_l[] = "0123456789abcdef",
                      xdigits_u[] = "0123456789ABCDEF";
    unsigned char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
    const char *xdigits, *curtok;
    int ch, seen_xdigits;
    int val;

    memset((tp = tmp), '\0', NS_IN6ADDRSZ);
    endp = tp + NS_IN6ADDRSZ;
    colonp = NULL;

    /* Leading :: requires some special handling. */
    if (*src == ':')
    {
        if (*++src != ':')
        {
            return -1;
        }
    }

    curtok = src;
    seen_xdigits = 0;
    val = 0;
    while ((ch = *src++) != '\0')
    {
        const char *pch;
        
        if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
        {
            pch = strchr((xdigits = xdigits_u), ch);
        }
        if (pch != NULL)
        {
            val <<= 4;
            val |= (pch - xdigits);
            if (++seen_xdigits > 4)
            {
                return -1;
            }
            continue;
        }
        if (ch == ':')
        {
            curtok = src;
            if (!seen_xdigits)
            {
                if (colonp)
                        return -1;
                colonp = tp;
                continue;
            }
            else if (*src == '\0')
            {
                return -1;
            }
            if (tp + NS_INT16SZ > endp)
            {
                return -1;
            }
            *tp++ = (unsigned char) (val >> 8) & 0xff;
            *tp++ = (unsigned char) val & 0xff;
            seen_xdigits = 0;
            val = 0;
            continue;
        }
        return -1;
    }
    if (seen_xdigits)
    {
        if (tp + NS_INT16SZ > endp)
        {
            return -1;
        }
        *tp++ = (unsigned char) (val >> 8) & 0xff;
        *tp++ = (unsigned char) val & 0xff;
    }
    if (colonp != NULL)
    {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;
        int i;
        
        if (tp == endp)
        {
            return -1;
        }
        for (i = 1; i <= n; i++)
        {
            endp[- i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
    {
        return -1;
    }
    memcpy(dst, tmp, NS_IN6ADDRSZ);
    return 0;
}

/**
 * Convert IP address string to byte array.
 * @param [in]   ver    IP version (4 or 6).
 * @param [in]   pStr   IP address string.
 * @param [out]  pAddr  IP address data buffer (16 bytes).
 * @returns  Data length.
 */
int str2ip(int ver, char *pStr, void *pAddr)
{
    unsigned char *pIp = pAddr;
    int  buf[16] = {0};
    int  retval = 0;

    if (4 == ver)
    {
        retval = sscanf(
                     pStr,
                     "%u.%u.%u.%u",
                     &(buf[0]),
                     &(buf[1]),
                     &(buf[2]),
                     &(buf[3])
                 );

        pIp[0] = (buf[0] & 0xFF);
        pIp[1] = (buf[1] & 0xFF);
        pIp[2] = (buf[2] & 0xFF);
        pIp[3] = (buf[3] & 0xFF);
    }
    else if (6 == ver)
    {
        retval = _parseIpv6Str(pStr, pIp);
        retval = ((0 == retval) ? 16 : 0);
    }

    return retval;
}

