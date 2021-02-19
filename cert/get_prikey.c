#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h> /* isspace */
#include <sys/stat.h>
#include <openssl/x509.h>
#include <openssl/pem.h>


int main(int argc, char *argv[])
{
    FILE *pFile = NULL;
    EVP_PKEY *pPriKey = NULL;
    RSA *pRsa = NULL;

    unsigned char *pByte;
    unsigned char *pData;
    unsigned int   dataSize;
    int i;


    if (argc != 2)
    {
        printf("Usage: %s <private_key.pem>\n", argv[0]);
        printf("\n");
        return 0;
    }


    pFile = fopen(argv[1], "r");
    if (pFile == NULL)
    {
        printf("Exit: cannot open file (%s)\n", argv[1]);
        return 0;
    }

    pPriKey = PEM_read_PrivateKey(pFile, NULL, NULL, NULL);
    fclose( pFile );

    if (pPriKey == NULL)
    {
        printf("PEM_read_PrivateKey ... failed\n");
        return 0;
    }

    pRsa = EVP_PKEY_get1_RSA( pPriKey );
    EVP_PKEY_free( pPriKey );

    if (pRsa == NULL)
    {
        printf("EVP_PKEY_get1_RSA ... failed\n");
        return 0;
    }

    dataSize = RSA_size( pRsa );
    pData = (unsigned char *)pRsa->d->d;

    if ( pData )
    {
        printf("RSA> private key size = %d\n", dataSize);
        printf("RSA> RK:\n");
        pByte = pData;
        for (i=0; i<dataSize; i++)
        {
            if ((i != 0) && ((i % 16) == 0))
            {
                printf("\n");
            }
            printf(" %02X", *(pByte+i));
        }
        printf("\n");
        printf("RSA>\n");

        RSA_free( pRsa );
    }
    else
    {
        printf(" crypto_get_public_key() Error\n" );
        return 0;
    }

    return 0;
}

