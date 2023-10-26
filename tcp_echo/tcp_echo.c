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

#define MAX_BUFFER_SIZE (2047)
#define MAX_CLIENT_NUM  (10)
#define MAX_SOCKET_NUM  (MAX_CLIENT_NUM + 4)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

typedef struct _tSocket
{
    struct sockaddr_in  addr;
    socklen_t           addr_len;
    int                 fd;
} tSocket;


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

/* [0] standard input
 * [1] standard output
 * [2] standard error output
 * [3] server socket
 * [4] client socket #1
 * [5] client socket #2
 * [6] client socket #3
 * [7] client socket #4
 * ...
 * [13] client socket #10
 */
tSocket g_socket[MAX_SOCKET_NUM];

unsigned char g_buf[MAX_BUFFER_SIZE+1];
int fd_server = -1;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void mem_dump(const void *addr, unsigned int size)
{
    unsigned char *p = (unsigned char *)addr;
    unsigned int   i;

    if (p == NULL)
    {
        printf("%s: NULL\n", __func__);
        printf("\n");
        return;
    }

    for (i=0; i<size; i++)
    {
        if ((i != 0) && ((i % 16) == 0))
        {
            printf("\n");
        }

        if ((p[i] > 0x1F) && (p[i] < 0x7F))
        {
            printf(" %c ", p[i]);
        }
        else
        {
            printf("%02X ", p[i]);
        }
    }
    printf("\n");
    printf(" (%u bytes)\n", size);
    printf("\n");
}

static void _endHdlr(int arg)
{
    if (fd_server > 0)
    {
        close( fd_server );
        fd_server = -1;
        g_socket[3].fd = -1;
    }
}

int main(int argc, char *argv[])
{
    int  fd_client = -1;
    struct sockaddr_in  addr_client;
    socklen_t addr_len_client = sizeof(struct sockaddr_in);

    int  reuse_addr = 1;
    socklen_t  reuse_addr_len;

    unsigned short port = 0;
    int dump_flag = 0;
    int size;
    int i;


    if (2 == argc)
    {
        port = atoi( argv[1] );
    }
    else if (3 == argc)
    {
        port = atoi( argv[1] );
        if (0 == strcmp(argv[2], "-d"))
        {
            dump_flag = 1;
        }
    }
    else
    {
        printf("Usage: tcp_echo PORT [-d]\n");
        printf("\n");
        return 0;
    }

    if (0 == port)
    {
        printf("ERR: incorrect port number 0\n");
        return -1;
    }

    for (i=0; i<MAX_SOCKET_NUM; i++)
    {
        g_socket[i].fd = -1;
        g_socket[i].addr_len = sizeof( struct sockaddr_in );
        bzero(&(g_socket[i].addr), g_socket[i].addr_len);
    }

    signal(SIGINT,  _endHdlr);
    signal(SIGKILL, _endHdlr);
    signal(SIGTERM, _endHdlr);


    if ((fd_server=socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror( "socket" );
        return -1;
    }

    reuse_addr_len = sizeof( reuse_addr );
    if (setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, reuse_addr_len) < 0)
    {
        perror( "setsockopt" );
        close( fd_server );
        return -1;
    }

    /* Fill the sockaddr_in structure */
    g_socket[3].fd = fd_server;
    g_socket[3].addr.sin_family      = AF_INET;
    g_socket[3].addr.sin_port        = htons( port );
    g_socket[3].addr.sin_addr.s_addr = htonl( INADDR_ANY );

    if (bind(fd_server, (struct sockaddr *)&(g_socket[3].addr), g_socket[3].addr_len) < 0)
    {
        perror( "bind" );
        close( fd_server );
        return -1;
    }

    if (listen(fd_server, MAX_SOCKET_NUM) < 0)
    {
        perror( "listen" );
        close( fd_server );
        return -1;
    }

    printf("TCP echo server listen (port %u) ...\n\n", port);

    while ( 1 )
    {
        fd_set flags;
        int fd;

        FD_ZERO( &flags );
        for (i=0; i<MAX_SOCKET_NUM; i++)
        {
            if (g_socket[i].fd > 0)
            {
                FD_SET(g_socket[i].fd, &flags);
            }
        }

        if (select(MAX_SOCKET_NUM, &flags, NULL, NULL, NULL) < 0)
        {
            perror( "select" );
            break;
        }

        for (fd=0; fd<MAX_SOCKET_NUM; fd++)
        {
            if ( FD_ISSET(fd, &flags) )
            {
                if (fd_server == fd)
                {
                    /* fd is server socket */
                    fd_client = accept(
                                    fd_server,
                                    (struct sockaddr *)&addr_client,
                                    &addr_len_client
                                );
                    if (fd_client < 0)
                    {
                        perror( "accept" );
                    }
                    else if (fd_client >= MAX_SOCKET_NUM)
                    {
                        printf("ERR: fd > %d\n", MAX_SOCKET_NUM);
                        close( fd_client );
                    }
                    else
                    {
                        g_socket[fd_client].fd = fd_client;
                        g_socket[fd_client].addr = addr_client;
                        g_socket[fd_client].addr_len = addr_len_client;

                        printf(
                            "Connect[%d] from %s\n",
                            fd_client,
                            inet_ntoa( addr_client.sin_addr )
                        );
                    }
                }
                else
                {
                    /* fd is client socket */
                    size = read(fd, g_buf, MAX_BUFFER_SIZE);
                    if (size <= 0)
                    {
                        printf("Connection[%d] closed\n\n", fd);
                        g_socket[fd].fd = -1;
                        close( fd );
                    }
                    else
                    {
                        g_buf[size] = 0x00;
                        if ( dump_flag )
                        {
                            mem_dump(g_buf, size);
                        }

                        printf(
                            " -> %s:%d (%d bytes)\n",
                            inet_ntoa( g_socket[fd].addr.sin_addr ),
                            ntohs( g_socket[fd].addr.sin_port ),
                            size
                        );

                        write(fd, g_buf, size);
                    }
                }
            }
        }
    }

    for (i=0; i<MAX_SOCKET_NUM; i++)
    {
        if (g_socket[i].fd > 0)
        {
            close( g_socket[i].fd );
        }
    }

    printf("\n");
    printf("TCP echo server terminated\n");
    printf("\n");

    return 0;
}

