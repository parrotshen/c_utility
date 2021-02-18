#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utility.h"


/**
*  Merge multiple file to a single file.
*  @param [in]  pFileOut   Output file name.
*  @param [in]  pFileIn    Input file name array.
*  @param [in]  fileInNum  input file name number.
*/
void merge(char *pFileOut, char *pFileIn[], int fileInNum)
{
    FILE *pInput  = NULL;
    FILE *pOutput = NULL;
    unsigned int  finalSize;
    unsigned int  singleSize;

    unsigned char byte;
    unsigned int  j;
    int   i;

    if ((pOutput=fopen(pFileOut, "w")) == NULL)
    {
        printf("Exit: cannot open file '%s'\n", pFileOut);
        printf("\n");
        goto _EXIT;
    }

    finalSize = 0;

    for (i=1; i<=fileInNum; i++)
    {
        singleSize = filesize( pFileIn[i] );

        if ((pInput=fopen(pFileIn[i], "r")) == NULL)
        {
            printf("ERR: cannot open file %s\n", pFileIn[i]);
            printf("\n");
            break;
        }

        printf(
            "#%d combine file size: %u\n",
            (i + 1),
            singleSize
        );

        for (j=0; j<singleSize; j++)
        {
            fread(&byte, 1, 1, pInput);
            fwrite(&byte, 1, 1, pOutput);
        }

        fclose( pInput );
        pInput = NULL;

        finalSize += singleSize;
    }

    printf("\n");
    printf("Final merged file size: %u\n", finalSize);
    printf("\n");

_EXIT:
    if ( pInput  ) fclose( pInput );
    if ( pOutput ) fclose( pOutput );
}


void help(void)
{
    // argv[]      0       1      2      3          4
    printf("Usage: bincomb FILE_1 FILE_2 FILE_3 ... FILE_OUT\n");
    printf("\n");
    printf("EX   : bincomb foo.bin bar.bin test.bin\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        help();
        return 0;
    }

    merge(argv[argc - 1], argv, (argc - 2));

    return 0;
}

