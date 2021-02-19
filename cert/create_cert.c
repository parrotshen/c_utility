#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>


/**
 * Create a signed certificate.
 * @param [out] ppCert     A @ref X509 object.
 * @param [out] ppPriKey   A @ref EVP_PKEY object.
 * @param [in]  pCountry   Country name.
 * @param [in]  pOrgName   Organization name.
 * @param [in]  pCommName  Common name.
 * @param [in]  pDnsName   DNS name.
 * @param [in]  serial     Serial number.
 * @param [in]  daysValid  Valid days.
 * @param [in]  pRoot      Private key file.
 * @returns  Success(1) or failure(0).
 */
int CreateCertificate(
    X509     **ppCert,
    EVP_PKEY **ppPriKey,
    char      *pCountry,
    char      *pOrgName,
    char      *pCommName,
    char      *pDnsName,
    int        serial,
    int        daysValid,
    char      *pRoot
)
{
    FILE *pFileRoot = NULL;
    X509 *pCertNew = NULL;
    EVP_PKEY *pPriKeyNew = NULL;
    EVP_PKEY *pPriKeyRoot = NULL;
    X509_NAME *pIssuer = NULL;
    RSA *pKeyPair = NULL;
    BIGNUM *pBigNumber = NULL;
    int success = 0;

    do
    {
        // Create the certificate object
        pCertNew = X509_new();
        if ( !pCertNew ) break;

        // Set version 2, and get version 3
        X509_set_version(pCertNew, 2);

        // Set the certificate's properties
        ASN1_INTEGER_set(X509_get_serialNumber(pCertNew), serial);
        X509_gmtime_adj(X509_get_notBefore(pCertNew), 0);
        X509_gmtime_adj(
            X509_get_notAfter(pCertNew),
            (long)(60 * 60 * 24 * (daysValid ? daysValid : 1))
        );
        pIssuer = X509_get_subject_name(pCertNew);
        if (pCountry && *pCountry)
        {
            X509_NAME_add_entry_by_txt(
                pIssuer,
                "C",
                MBSTRING_ASC,
                (unsigned char *)pCountry,
                -1,
                -1,
                0
            );
        }
        if (pCommName && *pCommName)
        {
            X509_NAME_add_entry_by_txt(
                pIssuer,
                "CN",
                MBSTRING_ASC,
                (unsigned char *)pCommName,
                -1,
                -1,
                0
            );
        }
        if (pOrgName && *pOrgName)
        {
            X509_NAME_add_entry_by_txt(
                pIssuer,
                "O",
                MBSTRING_ASC,
                (unsigned char *)pOrgName,
                -1,
                -1,
                0
            );
        }
        X509_set_issuer_name(pCertNew, pIssuer);

        // Set the DNS name
        if (pDnsName && *pDnsName)
        {
            X509_EXTENSION *Extension;
            char Buffer[512];

            // Format the value
            sprintf(Buffer, "DNS:%s", pDnsName);
            Extension = X509V3_EXT_conf_nid(NULL, NULL, NID_subject_alt_name, Buffer);
            if (Extension)
            {
                X509_add_ext(pCertNew, Extension, -1);
                X509_EXTENSION_free( Extension );
            }
        }

        // Create the RSA key pair object
        pKeyPair = RSA_new();
        if (!pKeyPair) break;

        // Create the big number object
        pBigNumber = BN_new();
        if (!pBigNumber) break;

        // Set the word
        if (!BN_set_word(pBigNumber, 65537)) break;

        // Generate the key pair; lots of computes here
        if (!RSA_generate_key_ex(pKeyPair, 1024, pBigNumber, NULL)) break;

        // Now we need a private key object
        pPriKeyNew = EVP_PKEY_new();
        if (!pPriKeyNew) break;

        // Assign the key pair to the private key object
        if (!EVP_PKEY_assign_RSA(pPriKeyNew, pKeyPair)) break;

        // pKeyPair now belongs to pPriKeyNew, so don't clean it up separately
        pKeyPair = NULL;

        // Set the certificate's public key from the private key object
        if (!X509_set_pubkey(pCertNew, pPriKeyNew)) break;

        // Sign it with SHA-1
        if ( pRoot )
        {
            pFileRoot = fopen(pRoot, "r");
            if (pFileRoot == NULL) break;

            pPriKeyRoot = PEM_read_PrivateKey(pFileRoot, NULL, NULL, NULL);
            if (pPriKeyRoot == NULL) break;

            // Sign by the root private key
            if (!X509_sign(pCertNew, pPriKeyRoot, EVP_sha1())) break;

            EVP_PKEY_free( pPriKeyRoot );
            fclose( pFileRoot );
        }
        else
        {
            // Sign by it self
            if (!X509_sign(pCertNew, pPriKeyNew, EVP_sha1())) break;
        }

        // Success
        success = 1;

    } while (0);  

    // Things we always clean up
    if ( pBigNumber )
    {
        BN_free( pBigNumber );
    }

    // Things we clean up only on failure
    if ( !success )
    {
        if ( pCertNew )
        {
            X509_free( pCertNew );
        }
        if ( pPriKeyNew )
        {
            EVP_PKEY_free( pPriKeyNew );
        }
        if ( pPriKeyRoot )
        {
            EVP_PKEY_free( pPriKeyRoot );
        }
        if ( pKeyPair )
        {
            RSA_free( pKeyPair );
        }
        if ( pFileRoot )
        {
            fclose( pFileRoot );
        }
        pCertNew = NULL;
        pPriKeyNew = NULL;
    }

    // Return the certificate (or NULL)
    (*ppCert) = pCertNew;
    (*ppPriKey) = pPriKeyNew;

    return success;
}

/**
 * Save a certificate file (.pem or .der).
 * @param [in]  pName    File name.
 * @param [in]  pCert    A @ref X509 object.
 * @param [in]  pPriKey  A @ref EVP_PKEY object.
 */
void SaveCertificate(
    char     *pName,
    X509     *pCert,
    EVP_PKEY *pPriKey
)
{
    FILE *pFile;
    char  fileName[256];

    // [1] .pem file
    sprintf(fileName, "%s.pem", pName);

    pFile = fopen(fileName, "w");
    if ( pFile )
    {
        PEM_write_PrivateKey(pFile, pPriKey, NULL, NULL, 0, NULL, NULL);
        fclose( pFile );
    }
    else
    {
        printf("ERR: fail to open file %s\n\n", fileName);
    }

    pFile = fopen(fileName, "a");
    if ( pFile )
    {
        PEM_write_X509(pFile, pCert);
        fclose( pFile );
    }
    else
    {
        printf("ERR: fail to open file %s\n\n", fileName);
    }

    // [2] .der file
    sprintf(fileName, "%s.der", pName);

    pFile = fopen(fileName, "w");
    if ( pFile )
    {
        i2d_X509_fp(pFile, pCert);
        fclose( pFile );
    }
    else
    {
        printf("ERR: fail to open file %s\n\n", fileName);
    }
}

void help(void)
{
    printf("Usage: create_cert [OPTION]...\n");
    printf("\n");
    printf("  -c COUNTRY        Country name.\n");
    printf("  -o ORGANIZATION   Organization name.\n");
    printf("  -n NAME           Common name.\n");
    printf("  -e DNS            DNS name.\n");
    printf("  -s SERIAL         Serial number.\n");
    printf("  -d DAYS           Valid days.\n");
    printf("  -f FILE           Output to a .pem and a .der file.\n");
    printf("  -k KEY            Private key to sign (.pem file).\n");
    printf("  -h                Show the help message.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    char *root = NULL;
    char *file = NULL;
    char *country = "TW";
    char *org = "Hello Technology";
    char *cn = "Example";
    char *dns = NULL;
    int   serial = 168;
    int   days = 365;

    X509 *x509 = NULL;
    EVP_PKEY *pkey = NULL;
    int  success;
    int  ch;


    opterr = 0;
    while ((ch=getopt(argc, argv, "c:o:n:e:s:d:f:k:h")) != -1)
    {
        switch ( ch )
        {
            case 'c':
                country = optarg;
                break;
            case 'o':
                org = optarg;
                break;
            case 'n':
                cn = optarg;
                break;
            case 'e':
                dns = optarg;
                break;
            case 's':
                serial = atoi( optarg );
                break;
            case 'd':
                days = atoi( optarg );
                break;
            case 'f':
                file = optarg;
                break;
            case 'k':
                root = optarg;
                break;
            case 'h':
            default:
                help();
                return 0;
        }
    }

    success = CreateCertificate(
                  &x509,
                  &pkey,
                  country,
                  org,
                  cn,
                  dns,
                  serial,
                  days,
                  root
              );
    if ( success )
    {
        printf("Country      : %s\n", country);
        printf("Organization : %s\n", org);
        printf("Common name  : %s\n", cn);
        printf("DNS          : %s\n", dns);
        printf("Serial number: %d\n", serial);
        printf("Valid days   : %d\n", days);
        printf("Sign         : %s\n", root);
        printf("\n");

        if ( file )
        {
            SaveCertificate(file, x509, pkey);
        }
        else
        {
            PEM_write_PrivateKey(stdout, pkey, NULL, NULL, 0, NULL, NULL);
            PEM_write_X509(stdout, x509);
            fprintf(stdout, "\n");
        }

        X509_free( x509 );
        EVP_PKEY_free( pkey );
    }
    else
    {
        printf("ERR: fail to create certificate\n\n");
    }

    return 0;
}

