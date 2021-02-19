#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h> 
#include <openssl/ssl.h>


int main(int argc, char *argv[])
{
   BIO *bio_user = NULL;
   BIO *bio_root = NULL;
   X509 *x509_user = NULL;
   X509 *x509_root = NULL;
   EVP_PKEY *pkey_root = NULL;

   FILE *fp_root = NULL;
   FILE *fp_user = NULL;
   struct stat stat_buf_root;
   struct stat stat_buf_user;
   long fsize_root = 0;
   long fsize_user = 0;
   unsigned char *fcert_root = NULL;
   unsigned char *fcert_user = NULL;
   int retval = 0;

   if (argc != 3)
   {
      printf("Usage: %s <root_cert.der> <user_cert.der>\n", argv[0]);
      printf("\n");
      return 0;
   }


   fp_user = fopen(argv[2], "r+");
   if (fp_user == NULL)
   {
      printf("Exit: cannot open file (%s) !!\n", argv[2]);
      return 1;
   }

   if (stat(argv[2], &stat_buf_user) != 0)
   {
      printf("Exit: cannot get file size (%s) !!\n", argv[2]);
      fclose(fp_user);
      return 2;
   }
   fsize_user = stat_buf_user.st_size;


   fp_root = fopen(argv[1], "r+");
   if (fp_root == NULL)
   {
      printf("Exit: cannot open file (%s) !!\n", argv[1]);
      fclose(fp_user);
      return 3;
   }

   if (stat(argv[1], &stat_buf_root) != 0)
   {
      printf("Exit: cannot get file size (%s) !!\n", argv[1]);
      fclose(fp_user);
      fclose(fp_root);
      return 4;
   }
   fsize_root = stat_buf_root.st_size;


   /* Read the file contents into memory */
   fcert_user = (unsigned char *)malloc( fsize_user );
   fread(fcert_user, fsize_user, fsize_user, fp_user);
   fclose(fp_user);

   fcert_root = (unsigned char *)malloc( fsize_root );
   fread(fcert_root, fsize_root, fsize_root, fp_root);
   fclose(fp_root);


   SSL_load_error_strings(); /* readable error messages */
   SSL_library_init();

   if ( !(bio_user = BIO_new_mem_buf(fcert_user, fsize_user)) )
   {
      printf("ERROR: BIO_new_mem_buf (user)\n");
      goto ERROR_EXIT;
   }

   if ( !(bio_root = BIO_new_mem_buf(fcert_root, fsize_root)) )
   {
      printf("ERROR: BIO_new_mem_buf (root)\n");
      goto ERROR_EXIT;
   }

   if ( !(x509_user = d2i_X509_bio(bio_user, NULL)) )
   {
      printf("ERROR: d2i_X509_bio (user)\n");
      goto ERROR_EXIT;
   }

   if ( !(x509_root = d2i_X509_bio(bio_root, NULL)) )
   {
      printf("ERROR: d2i_X509_bio (root)\n");
      goto ERROR_EXIT;
   }

   if ( !(pkey_root = X509_get_pubkey(x509_root)) )
   {
      printf("ERROR: X509_get_pubkey\n");
      goto ERROR_EXIT;
   }


   retval = X509_verify(x509_user, pkey_root);
   if (retval <= 0)
   {
      printf("Certificate verify fail !! (%d)\n", retval);
   }
   else
   {
      printf("Certificate verify successfully !!\n");
   }


ERROR_EXIT:
   if ( !pkey_root )
      EVP_PKEY_free( pkey_root );
   if ( !x509_user )
      X509_free( x509_user );
   if ( !x509_root )
      X509_free( x509_root );
   if ( !bio_user )
      BIO_free( bio_user );
   if ( !bio_root )
      BIO_free( bio_root );
   if ( !fcert_user )
      free( fcert_user );
   if ( !fcert_root )
      free( fcert_root );

   return 0;
}
