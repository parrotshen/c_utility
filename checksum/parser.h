#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define IS_UPPER_ALHPABET(ch)  ((ch >= 'A') && (ch <= 'Z'))
#define IS_LOWER_ALHPABET(ch)  ((ch >= 'a') && (ch <= 'z'))
#define IS_NUMBER(ch)          ((ch >= '0') && (ch <= '9'))
#define IS_SPACE(ch)           ((ch == ' ') || (ch == '\t'))
#define LINE_SIZE              (1023)
#define TOKEN_SIZE             (255)


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
 * Get the first token from a string.
 * @param [in]   pString  Input string.
 * @param [out]  pToken   Output token.
 * @param [in]   tsize    Output buffer size.
 * @returns  String after remove the first token.
 */
char *parse_token(char *pString, char *pToken, int tsize);

/**
 * Read one line from a text file.
 * @param [in]   pFile  Input file pointer.
 * @param [out]  pLine  Output buffer.
 * @param [in]   lsize  Output buffer size.
 * @returns  Success(line length) or failure(-1).
 */
int parse_line(FILE *pFile, char *pLine, int lsize);

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
);

/**
 * Dump memory.
 * @param [in]  pDesc  Description string.
 * @param [in]  pAddr  Memory address.
 * @param [in]  size   Memory size.
 */
void mem_dump(char *pDesc, void *pAddr, unsigned int size);

/**
 * Convert IP address string to byte array.
 * @param [in]   ver    IP version (4 or 6).
 * @param [in]   pStr   IP address string.
 * @param [out]  pAddr  IP address data buffer (16 bytes).
 * @returns  Data length.
 */
int str2ip(int ver, char *pStr, void *pAddr);


#endif
