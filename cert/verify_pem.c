#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>


int main(int argc, char *argv[])
{
   BIO *fcert1, *fcert2;
   X509 *x1, *x2;
   EVP_PKEY *k1, *k2;
   int ret;

   if (argc != 3)
   {
      printf("Usage: %s <root_cert.pem> <user_cert.pem>\n", argv[0]);
      printf("\n");
      return 0;
   }


   SSL_load_error_strings(); /* readable error messages */
   SSL_library_init();  

   fcert1 = BIO_new( BIO_s_file() );
   fcert2 = BIO_new( BIO_s_file() );

   BIO_read_filename(fcert1, argv[1]);
   BIO_read_filename(fcert2, argv[2]);

   x1 = PEM_read_bio_X509_AUX(fcert1, NULL, NULL, NULL);
   x2 = PEM_read_bio_X509_AUX(fcert2, NULL, NULL, NULL);


   #define show_hehe(x)\
   printf("ver = %d\n", (int)X509_get_version(x));\
   \
   puts("issuer:");\
   {\
      char buf[1000];\
      X509_NAME *a = X509_get_issuer_name(x);\
      X509_NAME_oneline(a, buf, 1000);\
      puts(buf);\
   }\
   \
   puts("subject:");\
   {\
      char buf[1000];\
      X509_NAME *a = X509_get_subject_name(x);\
      X509_NAME_oneline(a, buf, 1000);\
      puts(buf);\
   }

   show_hehe(x1);
   show_hehe(x2);
   k1 = X509_get_pubkey(x1);
   k2 = X509_get_pubkey(x2);

   #if 1 // .........................
   // ..ok..
   ret = X509_verify(x1, k1);  // ..1
   printf("k1 -> x1: %d\n", ret);
   ret = X509_verify(x2, k1);  // 1
   printf("k1 -> x2: %d\n", ret);
   ret = X509_verify(x1, k2);  // 0
   printf("k2 -> x1: %d\n", ret);
   ret = X509_verify(x2, k2);  // 0
   printf("k2 -> x2: %d\n", ret);
   #endif


   if (X509_verify(x1, k1) > 0)
      puts("root OK");
   if (X509_verify(x2, k1) > 0)
      puts("user OK");


   EVP_PKEY_free( k1 );
   EVP_PKEY_free( k2 );

   X509_free( x1 );
   X509_free( x2 );

   BIO_free( fcert1 );
   BIO_free( fcert2 );

   return 0;
}

