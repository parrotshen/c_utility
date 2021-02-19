#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>


/**
 * Convert the certificate .pem to .der file.
 * @param [in]  pemName  .pem file name.
 * @param [in]  derName  .der file name.
 * @returns  Success(0) or failure(-1).
 */
int pem2der(char *pemName, char *derName)
{
    FILE *pFile = NULL;
    X509 *x509 = NULL;
    BIO  *bio = NULL;


    bio = BIO_new( BIO_s_file() );
    if (NULL == bio)
    {
         printf("ERR: fail to create BIO\n");
         return -1;
    }
    BIO_read_filename(bio, pemName);

    x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
    if (NULL == x509)
    {
         printf("ERR: fail to read X509\n");
         BIO_free( bio );
         return -1;
    }

    pFile = fopen(derName,"w");
    if (NULL == pFile)
    {
         printf("ERR: fail to open %s\n", derName);
         X509_free( x509 );
         BIO_free( bio );
         return -1;
    }
    i2d_X509_fp(pFile, x509);
    fclose( pFile );

    X509_free( x509 );
    BIO_free( bio );

    return 0;
}

/**
 * Convert the certificate .der to .pem file.
 * @param [in]  derName  .der file name.
 * @param [in]  pemName  .pem file name.
 * @returns  Success(0) or failure(-1).
 */
int der2pem(char *derName, char *pemName)
{
    X509 *x509 = NULL;
    BIO  *bio = NULL;

    FILE *pFile = NULL;
    unsigned char *pBuf;
    struct stat  info;
    long  size;


    if (stat(derName, &info) != 0)
    {
        printf("ERR: fail to get file size (%s)\n", derName);
        return -1;
    }
    size = info.st_size;

    pFile = fopen(derName, "r");
    if (NULL == pFile)
    {
        printf("ERR: fail to open %s\n", derName);
        return -1;
    }

    pBuf = (unsigned char *)malloc( size );
    if (NULL == pBuf)
    {
        printf("ERR: fail to malloc()\n");
        fclose( pFile );
        return -1;
    }
    fread(pBuf, size, 1, pFile);
    fclose( pFile );

    bio = BIO_new_mem_buf(pBuf, size);
    if (NULL == bio)
    {
        printf("ERR: fail to BIO_new_mem_buf()\n");
        return -1;
    }

    x509 = d2i_X509_bio(bio, NULL);
    if (NULL == x509)
    {
        printf("ERR: fail to d2i_X509_bio()\n");
        BIO_free( bio );
        return -1;
    }

    pFile = fopen(pemName, "w");
    if (NULL == pFile)
    {
        printf("ERR: cannot open %s\n", pemName);
        BIO_free( bio );
        X509_free( x509 );
        return -1;
    }

    PEM_write_X509(pFile, x509);
    fclose( pFile );

    BIO_free( bio );
    X509_free( x509 );

    return 0;
}

char *get_file_extension(char *pName)
{
    int len = strlen( pName );

    if ((len > 4) && ('.' == pName[len - 4]))
    {
        return &(pName[len - 4]);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    char *pNameA = NULL;
    char *pNameB = NULL;


    if (argc != 3)
    {
        printf("Usage: convert_cert .der .pem\n");
        printf("     : convert_cert .pem .der\n");
        printf("\n");
        return 0;
    }

    pNameA = argv[1];
    pNameB = argv[2];

    if (0 == strcmp(".der", get_file_extension( pNameA )))
    {
        if (0 == strcmp(".pem", get_file_extension( pNameB )))
        {
            if (0 == der2pem(pNameA, pNameB))
            {
                printf("Convert .der to .pem successfully !\n");
            }
            else
            {
                printf("ERR: fail to convert .der to .pem\n");
            }
            return 0;
        }
    }

    if (0 == strcmp(".pem", get_file_extension( pNameA )))
    {
        if (0 == strcmp(".der", get_file_extension( pNameB )))
        {
            if (0 == pem2der(pNameA, pNameB))
            {
                printf("Convert .pem to .der successfully !\n");
            }
            else
            {
                printf("ERR: fail to convert .pem to .der\n");
            }
            return 0;
        }
    }

    printf("ERR: incorrect file extension\n");

    return 0;
}

