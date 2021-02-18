#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
   unsigned char c = 0;
   int all = 0;
   int i, j;

   if ((argc > 1) && (0 == strcmp("-a", argv[1])))
   {
       all = 1;
   }

   for (i=0; i<64; i++)
   {
       for (j=0; j<4; j++)
       {
           if ((c > 0x1F) && (c < 0x80))
           {
               if (c == 0x20) printf("\033[7m");
               printf("%02Xh %03d  %c", c, c, c);
               if (c == 0x7F) printf("\033[0m");
               if (j < 3) printf("   :   ");
               else       printf("   \n");
           }
           else if ( all )
           {
               printf("%02Xh %03d  %c", c, c, c);
               if (j < 3) printf("   :   ");
               else       printf("   \n");
           }
           c++;
       }
   }

   printf("\n");
   return 0;
}

