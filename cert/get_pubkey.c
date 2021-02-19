#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h> /* isspace */
#include <sys/stat.h>
#include <openssl/x509.h>
#include <openssl/pem.h>


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

    if (argc != 2)
    {
        printf("Usage: %s <cert_file.der>\n", argv[0]);
        printf("     : %s <cert_file.pem>\n", argv[0]);
        printf("\n");
        return 0;
    }


    if (0 == strcmp(".der", get_file_extension( argv[1] )))
    {
        EVP_PKEY *pPubKey = NULL;
        X509     *pX509   = NULL;
        RSA      *pRsa    = NULL;
        unsigned char n[300];
        unsigned char e[300];
        unsigned int  len;
        unsigned int  i;

        FILE *pFile = NULL;
        unsigned char *pBuf = NULL;
        struct stat  info;
        long  size;


        if (stat(argv[1], &info) != 0)
        {
            printf("ERR: fail to get file size (%s)\n", argv[1]);
            goto _EXIT_DER;
        }
        size = info.st_size;

        pFile = fopen(argv[1], "r");
        if (NULL == pFile)
        {
            printf("ERR: fail to open %s\n", argv[1]);
            goto _EXIT_DER;
        }

        pBuf = (unsigned char *)malloc( size );
        if (NULL == pBuf)
        {
            printf("ERR: fail to malloc()\n");
            goto _EXIT_DER;
        }
        fread(pBuf, size, 1, pFile);

        pX509 = X509_new();
        if (NULL == pX509)
        {
            printf("ERR: fail to X509_new()\n");
            goto _EXIT_DER;
        }

        d2i_X509(&pX509, (const unsigned char **)&pBuf, size);

        pPubKey = X509_get_pubkey( pX509 );
        if (NULL == pPubKey)
        {
            printf("ERR: fail to X509_get_pubkey()\n");
            goto _EXIT_DER;
        }

        pRsa = EVP_PKEY_get1_RSA( pPubKey );
        if (NULL == pRsa)
        {
            printf("ERR: fail to EVP_PKEY_get1_RSA()\n");
            goto _EXIT_DER;
        }

        if (pRsa->n != NULL)
        {
            BN_bn2bin(pRsa->n, n);
            len = BN_num_bytes( pRsa->n );

        	printf("N (%d bytes):\n", len);
            for (i=0; i<len; i++)
            {
                if ((i != 0) && ((i % 16) == 0))
                {
                    printf("\n");
                }
                printf(" %02X", n[i]);
            }
            printf("\n");
            printf("\n");
        }
        else
        {
            printf("ERR: cannot find RSA(n)\n");
        }

        if (pRsa->e != NULL)
        {
            BN_bn2bin(pRsa->e, e);
            len = BN_num_bytes( pRsa->e );

        	printf("E (%d bytes):\n", len);
            for (i=0; i<len; i++)
            {
                if ((i != 0) && ((i % 16) == 0))
                {
                    printf("\n");
                }
                printf(" %02X", e[i]);
            }
            printf("\n");
            printf("\n");
        }
        else
        {
            printf("ERR: cannot find RSA(e)\n");
        }

_EXIT_DER:
        if ( pFile   ) fclose( pFile );
        if ( pRsa    ) RSA_free( pRsa );
        if ( pX509   ) X509_free( pX509 );
        if ( pPubKey ) EVP_PKEY_free( pPubKey );
        return 0;
    }


    if (0 == strcmp(".pem", get_file_extension( argv[1] )))
    {
        EVP_PKEY *pPubKey = NULL;
        BIO      *pCert = NULL;
        X509     *pX509 = NULL;
        int  retval;

        //[2.1] read certificate file (*.pem)
        pCert = BIO_new( BIO_s_file() );

        retval = BIO_read_filename(pCert, argv[1]);
        if (retval <= 0)
        {
            printf("BIO_read_filename ... failed\n");
            goto _EXIT_PEM;
        }

        pX509 = PEM_read_bio_X509(pCert, NULL, 0, NULL);
        if (NULL == pX509)
        {
            printf("PEM_read_bio_X509 ... failed\n");
            goto _EXIT_PEM;
        }

        pPubKey = X509_get_pubkey( pX509 );
        if (NULL == pPubKey)
        {
            printf("X509_get_pubkey ... failed\n");
            goto _EXIT_PEM;
        }

        switch ( pPubKey->type )
        {
            case EVP_PKEY_RSA:
                printf(
                    "%d bit RSA Key\n\n",
                    EVP_PKEY_bits( pPubKey )
                );
                break;
            case EVP_PKEY_DSA:
                printf(
                    "%d bit DSA Key\n\n",
                    EVP_PKEY_bits( pPubKey )
                );
                break;
            default:
                printf(
                    "%d bit non-RSA/DSA Key\n\n",
                    EVP_PKEY_bits( pPubKey )
                );
                break;
        }

        if ( !PEM_write_PUBKEY(stdout, pPubKey) )
        {
            printf("ERR: fail to get the public key\n");
        }

_EXIT_PEM:
        if ( pPubKey ) EVP_PKEY_free( pPubKey );
        if ( pX509   ) X509_free( pX509 );
        if ( pCert   ) BIO_free( pCert );
        return 0;
    }


    printf("ERR: incorrect file extension\n");
    return 0;
}

