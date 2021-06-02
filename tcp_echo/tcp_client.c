#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define SOCKET_PACKET_LEN  (1023)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

unsigned char g_buf[SOCKET_PACKET_LEN+1];
int fd_stdin = STDIN_FILENO;
int fd_tcp   = -1;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void mem_dump(const void *addr, unsigned int size)
{
    unsigned char *byte = (unsigned char *)addr;
    unsigned int   i;

    if ( byte )
    {
        for (i=0; i<size; i++)
        {
            if ((i != 0) && ((i % 16) == 0))
            {
                printf("\n");
            }
            printf(" %02X", byte[i]);
        }
        printf("\n");
        printf(" (%u bytes)\n", size);
        printf("\n");
    }
}

static void _endHdlr(int arg)
{
    if (fd_tcp > 0)
    {
        close( fd_tcp );
        fd_tcp = -1;
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in  addr_local;
    struct sockaddr_in  addr_remote;
    socklen_t addr_len_local = sizeof(struct sockaddr_in);
    socklen_t addr_len_to    = sizeof(struct sockaddr_in);

    char *dst_ip   = NULL;
    int   dst_port = 0;
    int   src_port = 0;
    int   debug    = 0;

    fd_set flags;
    int  option;
    int  retval;
    int  size;


    if (3 == argc)
    {
        dst_ip   = argv[1];
        dst_port = atoi( argv[2] );
    }
    else if (4 == argc)
    {
        dst_ip   = argv[1];
        dst_port = atoi( argv[2] );
        if (0 == strcmp(argv[3], "-d"))
        {
            debug = 1;
        }
        else
        {
            src_port = atoi( argv[3] );
        }
    }
    else if (5 == argc)
    {
        dst_ip   = argv[1];
        dst_port = atoi( argv[2] );
        src_port = atoi( argv[3] );
        if (0 == strcmp(argv[4], "-d"))
        {
            debug = 1;
        }
    }
    else
    {
        printf("Usage: tcp_client dst_ip dst_port [-d]\n");
        printf("     : tcp_client dst_ip dst_port src_port [-d]\n");
        printf("\n");
        return 0;
    }

    signal(SIGINT,  _endHdlr);
    signal(SIGKILL, _endHdlr);
    signal(SIGTERM, _endHdlr);

    if ((fd_tcp=socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror( "socket" );
        return -1;
    }

    /* Enable the port number re-use */
    option = 1;
    setsockopt(fd_tcp, SOL_SOCKET, SO_REUSEADDR, &option, sizeof( option ));

    /* Fill the sockaddr_in structure */
    bzero(&addr_local, addr_len_local);
    addr_local.sin_family      = AF_INET;
    addr_local.sin_port        = htons( src_port );
    addr_local.sin_addr.s_addr = htonl( INADDR_ANY );

    if (bind(fd_tcp, (struct sockaddr *)&addr_local, addr_len_local) < 0)
    {
        perror( "bind" );
        close( fd_tcp );
        return -1;
    }

    /* Fill the sockaddr_in structure */
    bzero(&addr_remote, addr_len_to);
    addr_remote.sin_family      = AF_INET;
    addr_remote.sin_port        = htons( dst_port );
    addr_remote.sin_addr.s_addr = inet_addr( dst_ip );

    retval = connect(
                 fd_tcp,
                 (struct sockaddr *)&addr_remote,
                 addr_len_to
             );
    if (retval < 0)
    {
        perror( "connect" );
        close( fd_tcp );
        return -1;
    }


    printf("Please type some words to send ...\n\n");

    FD_ZERO( &flags );
    while ( 1 )
    {
        printf("SEND >>\n");

        FD_CLR(fd_stdin, &flags);
        FD_CLR(fd_tcp,   &flags);
        FD_SET(fd_stdin, &flags);
        FD_SET(fd_tcp,   &flags);

        retval = select(fd_tcp+1, &flags, NULL, NULL, NULL);

        if (retval > 0)
        {
            if ( FD_ISSET(fd_stdin, &flags) )
            {
                size = read(fd_stdin, g_buf, SOCKET_PACKET_LEN);
                printf("\n");

                if (size > 0)
                {
                    g_buf[size] = 0x00;

                    write(fd_tcp, g_buf, size);
                }
            }

            if ( FD_ISSET(fd_tcp, &flags) )
            {
                size = read(fd_tcp, g_buf, SOCKET_PACKET_LEN);

                if (size > 0)
                {
                    g_buf[size] = 0x00;

                    printf("RECV >>\n");
                    printf("%s\n\n", g_buf);
                    if ( debug )
                    {
                        mem_dump(g_buf, size);
                    }
                }
            }
        }
        else if (retval == 0)
        {
            /* A timeout occured, restart the timer. */
            ;
        }
        else
        {
            perror( "select" );
            break;
        }
    }


    if (fd_tcp > 0) close( fd_tcp );

    return 0;
}

