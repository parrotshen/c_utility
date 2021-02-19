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

#define MAX_BUFF_SIZE  (2047)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

unsigned char g_buf[MAX_BUFF_SIZE+1];
int fd_udp = -1;


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
    if (fd_udp > 0)
    {
        close( fd_udp );
        fd_udp = -1;
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in  addr_local;
    struct sockaddr_in  addr_to;
    struct sockaddr_in  addr_from;
    socklen_t addr_len_local = sizeof(struct sockaddr_in);
    socklen_t addr_len_to    = sizeof(struct sockaddr_in);
    socklen_t addr_len_from  = sizeof(struct sockaddr_in);

    int port = 0;
    int dump_flag = 0;
    int size;


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
        printf("Usage: udp_echo PORT [-d]\n");
        printf("\n");
        return 0;
    }

    signal(SIGINT,  _endHdlr);
    signal(SIGKILL, _endHdlr);
    signal(SIGTERM, _endHdlr);

    if ((fd_udp=socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror( "socket" );
        return -1;
    }

    /* Fill the sockaddr_in structure */
    bzero(&addr_local, addr_len_local);
    addr_local.sin_family      = AF_INET;
    addr_local.sin_port        = htons( port );
    addr_local.sin_addr.s_addr = htonl( INADDR_ANY );

    if (bind(fd_udp, (struct sockaddr *)&addr_local, addr_len_local) < 0)
    {
        perror( "bind" );
        close( fd_udp );
        return -1;
    }

    /* Fill the sockaddr_in structure */
    bzero(&addr_to, addr_len_to);
    addr_to.sin_family = AF_INET;

    printf("UDP echo server start ...\n\n");

    while ( 1 )
    {
        size = recvfrom(
                   fd_udp,
                   g_buf,
                   MAX_BUFF_SIZE,
                   0,
                   (struct sockaddr *)&addr_from,
                   &addr_len_from
               );
        if (size <= 0)
        {
            break;
        }

        g_buf[size] = 0x00;
        if ( dump_flag )
        {
            mem_dump(g_buf, size);
        }

        addr_to.sin_port        = addr_from.sin_port;
        addr_to.sin_addr.s_addr = addr_from.sin_addr.s_addr;
        printf(
            " -> %s:%d (%d bytes)\n",
            inet_ntoa( addr_to.sin_addr ),
            ntohs( addr_to.sin_port ),
            size
        );

        sendto(
            fd_udp,
            g_buf,
            size,
            0,
            (struct sockaddr *)&addr_to,
            addr_len_to
        );
    }

    if (fd_udp > 0)
    {
        close( fd_udp );
    }

    printf("\n");
    printf("UDP echo server terminated\n");
    printf("\n");

    return 0;
}

