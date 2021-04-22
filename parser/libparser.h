#ifndef __UTILITY_H__
#define __UTILITY_H__

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

typedef enum
{
    PARSE_CONTINUE = 0,
    PARSE_STOP
} tParseAction;

typedef int (*tParseLineCb)(char *pStr, int len, int count);
typedef int (*tParseTokenCb)(char *pStr, int len, int count);


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

/**
 * Bypass number of tokens in a string.
 * @param [in]  num      Number of tokens.
 * @param [in]  pString  Input string.
 * @returns  String after bypass number of tokens.
 */
char *bypass_token(int num, char *pString);

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
 * @param [out]  pLine  Output line string.
 * @param [in]   lsize  Output buffer size.
 * @returns  Success(1) or failure(0).
 */
int parse_line(FILE *pFile, char *pLine, int lsize);

/**
 * Parse a string into tokens.
 * @param [in]  pString     Input string.
 * @param [in]  pParseFunc  Token parsing callback function.
 * @returns  Success(> 0) or failure(0).
 */
int parse_string_into_token(char *pString, tParseTokenCb pParseFunc);

/**
 * Parse a text file into lines.
 * @param [in]  pFileName   Input file name.
 * @param [in]  pParseFunc  Line parsing callback function.
 * @returns  Success(> 0) or failure(0).
 */
int parse_file_into_line(char *pFileName, tParseLineCb pParseFunc);

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
 * Convert IP address data to string.
 * @param [in]   ver      IP version (4 or 6).
 * @param [in]   pAddr    IP address data.
 * @param [out]  pBuf     IP address string buffer.
 * @param [in]   bufSize  IP address string buffer size.
 * @returns  String length.
 */
int ip2str(int ver, void *pAddr, char *pBuf, int bufSize);

/**
 * Convert IP address string to byte array.
 * @param [in]   ver    IP version (4 or 6).
 * @param [in]   pStr   IP address string.
 * @param [out]  pAddr  IP address data buffer (16 bytes).
 * @returns  Data length.
 */
int str2ip(int ver, char *pStr, void *pAddr);


/**
 * Convert byte array data to HEX string.
 * @param [in]   pHex     Byte array data.
 * @param [in]   hexSize  Byte array data size.
 * @param [out]  pBuf     String buffer.
 * @param [in]   bufSize  String buffer size.
 * @returns  String length.
 */
int hex2str(void *pHex, int hexSize, char *pBuf, int bufSize);

/**
 * Convert HEX string to byte array.
 * @param [in]   pStr     HEX string.
 * @param [out]  pBuf     Byte array buffer.
 * @param [in]   bufSize  Byte array buffer size.
 * @returns  Data length.
 */
int str2hex(char *pStr, unsigned char *pBuf, int bufSize);


/**
 * Convert byte array data to PLMN ID string.
 * @param [in]   pPlmn     Byte array data.
 * @param [in]   plmnSize  Byte array data size.
 * @param [out]  pBuf      String buffer.
 * @param [in]   bufSize   String buffer size.
 * @returns  String length.
 */
int plmn2str(void *pPlmn, int plmnSize, char *pBuf, int bufSize);

/**
 * Convert PLMN ID string to byte array.
 * @param [in]   pStr     PLMN ID string.
 * @param [out]  pBuf     Byte array buffer.
 * @param [in]   bufSize  Byte array buffer size.
 * @returns  Data length.
 */
int str2plmn(char *pStr, unsigned char *pBuf, int bufSize);


/**
 * Dump memory.
 * @param [in]  pDesc  Description string.
 * @param [in]  pAddr  Memory address.
 * @param [in]  size   Memory size.
 */
void mem_dump(char *pDesc, void *pAddr, unsigned int size);


#endif
