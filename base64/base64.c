#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define B64_DEF_LINE_SIZE   76
#define B64_MIN_LINE_SIZE    4

#define THIS_OPT(ac, av) (ac > 1 ? av[1][0] == '-' ? av[1][1] : 0 : 0)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

/*
* Translation Table as described in RFC1113
*/
static const char cb64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
* Translation Table to decode (created by author)
*/
static const char cd64[] =
    "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

/*
* encodeblock
*
* encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
void encodeblock(unsigned char in[3], unsigned char out[4], int len)
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/*
* encode
*
* base64 encode a stream adding padding and line breaks as per spec.
*/
void encode(FILE *infile, FILE *outfile, int linesize)
{
    unsigned char in[3], out[4];
    int i, len, blocksout = 0;

    while( !feof( infile ) ) {
        len = 0;
        for( i = 0; i < 3; i++ ) {
            in[i] = (unsigned char) getc( infile );
            if( !feof( infile ) ) {
                len++;
            }
            else {
                in[i] = 0;
            }
        }
        if( len ) {
            encodeblock( in, out, len );
            for( i = 0; i < 4; i++ ) {
                putc( out[i], outfile );
            }
            blocksout++;
        }
        if( (blocksout >= (linesize / 4)) || feof( infile ) ) {
            if( blocksout ) {
                fprintf( outfile, "\n" );
            }
            blocksout = 0;
        }
    }
}

/*
* decodeblock
*
* decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
void decodeblock( unsigned char in[4], unsigned char out[3] )
{   
    out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
    out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
    out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

/*
* decode
*
* decode a base64 encoded stream discarding padding, line breaks and noise
*/
void decode(FILE *infile, FILE *outfile)
{
    unsigned char in[4], out[3], v;
    int i, len;

    while( !feof( infile ) ) {
        for( len = 0, i = 0; i < 4 && !feof( infile ); i++ ) {
            v = 0;
            while( !feof( infile ) && v == 0 ) {
                v = (unsigned char) getc( infile );
                v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
                if( v ) {
                    v = (unsigned char) ((v == '$') ? 0 : v - 61);
                }
            }
            if( !feof( infile ) ) {
                len++;
                if( v ) {
                    in[ i ] = (unsigned char) (v - 1);
                }
            }
            else {
                in[i] = 0;
            }
        }
        if( len ) {
            decodeblock( in, out );
            for( i = 0; i < len - 1; i++ ) {
                putc( out[i], outfile );
            }
        }
    }
}

/*
* returnable errors
*
* Error codes returned to the operating system.
*/
#define B64_SYNTAX_ERROR        1
#define B64_FILE_ERROR          2
#define B64_FILE_IO_ERROR       3
#define B64_ERROR_OUT_CLOSE     4
#define B64_LINE_SIZE_TO_MIN    5

/*
* b64_message
*
* Gather text messages in one place.
*/
char *b64_message(int errcode)
{
    #define B64_MAX_MESSAGES 6
    char *msgs[ B64_MAX_MESSAGES ] = {
            "b64:000:Invalid Message Code.\n",
            "b64:001:Syntax Error -- check help for usage.\n",
            "b64:002:File Error Opening/Creating Files.\n",
            "b64:003:File I/O Error -- Note: output file not removed.\n",
            "b64:004:Error on output file close.\n",
            "b64:004:linesize set to minimum.\n"
    };
    char *msg = msgs[ 0 ];

    if( errcode > 0 && errcode < B64_MAX_MESSAGES ) {
        msg = msgs[ errcode ];
    }

    return( msg );
}

/*
* b64
*
* 'engine' that opens streams and calls encode/decode
*/
int b64(int opt, char *infilename, char *outfilename, int linesize)
{
    FILE *infile;
    int retcode = B64_FILE_ERROR;

    if( !infilename ) {
        infile = stdin;
    }
    else {
        infile = fopen( infilename, "rb" );
    }
    if( !infile ) {
        perror( infilename );
    }
    else {
        FILE *outfile;
        if( !outfilename ) {
            outfile = stdout;
        }
        else {
            outfile = fopen( outfilename, "wb" );
        }
        if( !outfile ) {
            perror( outfilename );
        }
        else {
            if( opt == 'e' ) {
                encode( infile, outfile, linesize );
            }
            else {
                decode( infile, outfile );
            }
            if (ferror( infile ) || ferror( outfile )) {
                retcode = B64_FILE_IO_ERROR;
            }
            else {
                 retcode = 0;
            }
            if( outfile != stdout ) {
                if( fclose( outfile ) != 0 ) {
                    perror( b64_message( B64_ERROR_OUT_CLOSE ) );
                    retcode = B64_FILE_IO_ERROR;
                }
            }
        }
        if( infile != stdin ) {
            fclose( infile );
        }
    }

    return( retcode );
}

/*
* showuse
*
* display usage information, help, version info
*/
void showuse(void)
{
    printf( "  base64   (Base64 Encode/Decode)      Bob Trower 08/03/01\n" );
    printf( "           (C) Copr Bob Trower 1986-01.      Version 0.00B\n" );
    printf( "  Usage:   base64 -option [-l num] [<FileIn> [<FileOut>]]\n" );
    printf( "\n" );
    printf( "  Options: -e  encode to Base64\n" );
    printf( "           -d  decode from Base64\n" );
    printf( "  Note:    -l  use to change line size (from 76 characters)\n" );
    printf( "\n" );
    printf( "  Returns: 0 = success, non-zero is an error code.\n" );
    printf( "  ErrCode: 1 = Bad Syntax, 2 = File Open, 3 = File I/O\n" );
}

/*
* main
*
* parse and validate arguments and call b64 engine or help
*/
int main(int argc, char **argv)
{
    char *infilename  = NULL;
    char *outfilename = NULL;
    int linesize = B64_DEF_LINE_SIZE;
    int status = 0;
    int opt = 0;

    if (1 == argc)
    {
        showuse();
        return status;
    }
    
    while ( THIS_OPT(argc, argv) )
    {
        switch ( THIS_OPT(argc, argv) )
        {
            case 'l':
                linesize = atoi( &(argv[1][2]) );
                if (linesize < B64_MIN_LINE_SIZE)
                {
                    linesize = B64_MIN_LINE_SIZE;
                    printf("%s\n", b64_message( B64_LINE_SIZE_TO_MIN ));
                }
                break;
            case '?':
            case 'h':
                opt = 'h';
                break;
            case 'e':
            case 'd':
                opt = THIS_OPT(argc, argv);
                break;
            default:
                opt = 0;
                break;
        }
        argv++;
        argc--;
    }

    switch ( opt )
    {
        case 'e':
        case 'd':
            infilename  = argc > 1 ? argv[1] : NULL;
            outfilename = argc > 2 ? argv[2] : NULL;
            status = b64(opt, infilename, outfilename, linesize);
            break;
        case 0:
            status = B64_SYNTAX_ERROR;
        case 'h':
            showuse();
            break;
    }

    if ( status )
    {
        printf("%s\n", b64_message( status ));
    }

    return status;
}

