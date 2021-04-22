#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void size_with_unit(unsigned long long valueL)
{
    double valueF;

    if (valueL > 0x400)
    {
        valueF = (((double)valueL) / 1024);
        if (valueL % 0x400)
        {
            printf("= %lf[1;35mK[0m\n", valueF);
        }
        else
        {
            printf("= %.lf[1;35mK[0m\n", valueF);
        }
    }
    if (valueL > 0x100000)
    {
        valueF = (((double)valueL) / 1048576);
        if (valueL % 0x100000)
        {
            printf("= %lf[1;35mM[0m\n", valueF);
        }
        else
        {
            printf("= %.lf[1;35mM[0m\n", valueF);
        }
    }
    if (valueL > 0x40000000)
    {
        valueF = (((double)valueL) / 1073741824);
        if (valueL % 0x40000000)
        {
            printf("= %lf[1;35mG[0m\n", valueF);
        }
        else
        {
            printf("= %.lf[1;35mG[0m\n", valueF);
        }
    }
}

int main(int argc, char *argv[])
{
    unsigned long long valueL;
    double valueF;
    unsigned int unit[2];
    int floating = 0;
    int i;

    if (argc < 2)
    {
        printf(
            "Usage: memsize SIZE_in_DEC[[1;35mG[0m|[1;35mM[0m|[1;35mK[0m]\n"
        );
        printf(
            "     : memsize SIZE_in_HEX\n\n"
        );
        return 0;
    }

    if ((strlen(argv[1]) > 2) && ('0' == argv[1][0]) && ('x' == argv[1][1]))
    {
        sscanf(argv[1], "0x%llx", &valueL);

        printf("  %s\n", argv[1]);
        printf("= %llu\n", valueL);
        size_with_unit( valueL );
        printf("\n");
    }
    else
    {
        for (i=0; i<strlen(argv[1]); i++)
        {
            if ('.' == argv[1][i])
            {
                floating = 1;
                break;
            }
        }

        switch ( argv[1][strlen(argv[1]) - 1] )
        {
            case 'k': case 'K':
                argv[1][strlen(argv[1]) - 1] = 0x00;
                printf("  %s[1;35mK[0m\n", argv[1]);
                unit[0] = 10;
                unit[1] = 1024;
                break;
            case 'm': case 'M':
                argv[1][strlen(argv[1]) - 1] = 0x00;
                printf("  %s[1;35mM[0m\n", argv[1]);
                unit[0] = 20;
                unit[1] = 1048576;
                break;
            case 'g': case 'G':
                argv[1][strlen(argv[1]) - 1] = 0x00;
                printf("  %s[1;35mG[0m\n", argv[1]);
                unit[0] = 30;
                unit[1] = 1073741824;
                break;
            default:
                printf("  %s\n", argv[1]);
                unit[0] = 0;
                unit[1] = 1;
        }

        if ( floating )
        {
            valueF = (double)atof( argv[1] );
            valueF *= unit[1];
            valueL = (unsigned long long)valueF;
        }
        else
        {
            valueL = atoll( argv[1] );
            valueL <<= unit[0];
        }

        if (unit[0] > 0)
        {
            printf("= %llu\n", valueL);
        }
        else
        {
            size_with_unit( valueL );
        }
        printf("= 0x%llX\n", valueL);
        printf("\n");
    }

    return 0;
}

