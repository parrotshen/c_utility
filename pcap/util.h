#ifndef __UTIL_H__
#define __UTIL_H__


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

/* Little endian */
#define LE_BYTE_TO_UINT16(pBuf, val) \
    do { \
        val  = (*((unsigned char *)pBuf+1) <<  8); \
        val |= (*((unsigned char *)pBuf  )      ); \
    } while (0)

#define UINT16_TO_LE_BYTE(val, pBuf) \
    do { \
        *((unsigned char *)pBuf+1) = ((val >>  8) & 0xFF); \
        *((unsigned char *)pBuf  ) = ((val      ) & 0xFF); \
    } while (0)

#define LE_BYTE_TO_UINT32(pBuf, val) \
    do { \
        val  = (*((unsigned char *)pBuf+3) << 24); \
        val |= (*((unsigned char *)pBuf+2) << 16); \
        val |= (*((unsigned char *)pBuf+1) <<  8); \
        val |= (*((unsigned char *)pBuf  )      ); \
    } while (0)

#define UINT32_TO_LE_BYTE(val, pBuf) \
    do { \
        *((unsigned char *)pBuf+3) = ((val >> 24) & 0xFF); \
        *((unsigned char *)pBuf+2) = ((val >> 16) & 0xFF); \
        *((unsigned char *)pBuf+1) = ((val >>  8) & 0xFF); \
        *((unsigned char *)pBuf  ) = ((val      ) & 0xFF); \
    } while (0)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void dump(void *pAddr, int len);
long file_size(char *pIn);
char *get_token(char *pLine, char *pToken, int tsize);
int read_line(FILE *pFile, char *pLine, int lsize);
unsigned int read_file(char *pIn, unsigned char *pBuf, unsigned int bsize);


#endif
