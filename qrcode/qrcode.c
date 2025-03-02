#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "generate.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    char *string = NULL;
    int version = 1; // 1 ~ 40
    int level = 1;   // 1 ~ 4
    char EC = 'L';

    if (argc > 3)
    {
        string = argv[1];
        version = atoi( argv[2] );
        EC = argv[3][0];
    }
    else if (argc > 2)
    {
        string = argv[1];
        version = atoi( argv[2] );
    }
    else
    {
        printf("Usage: qrcode STRING VERSION [L|M|Q|H]\n\n");
        return 0;
    }

    switch ( EC )
    {
        default:
        case 'L': case 'l':
            level = 1;
            break;
        case 'M': case 'm':
            level = 2;
            break;
        case 'Q': case 'q':
            level = 3;
            break;
        case 'H': case 'h':
            level = 4;
            break;
    }

    qr_code_generate(string , version, level);

    return 0;
}

