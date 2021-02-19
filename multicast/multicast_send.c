#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>  /* getaddrinfo */
#include <arpa/inet.h>


#define MCAST_PORT  "12345"

int main(int argc, char *argv[])
{
    struct addrinfo *mcastAddr = NULL;
    struct addrinfo  hints;
    int fd_sock = -1;
    int ttl = 1;
    int status;
    int i;


    if (argc < 3)
    {
        printf("Usage: multicast_send MGROUP DATA-1 DATA-2 ... DATA-n\n");
        printf("\n");
        return 0;
    }

    /* resolve destination address for multicast datagrams */
    memset(&hints, 0x00, sizeof( struct addrinfo ));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_NUMERICHOST;

    status = getaddrinfo(argv[1], MCAST_PORT, &hints, &mcastAddr);
    if (status != 0)
    {
        printf("ERR: getaddrinfo [%s]\n", gai_strerror( status ));
        printf("\n");
        goto _EXIT;
    }

    /* create socket to send multicast datagrams */
    fd_sock = socket(mcastAddr->ai_family, mcastAddr->ai_socktype, 0);
    if (fd_sock < 0)
    {
        perror( "socket" );
        printf("\n");
        goto _EXIT;
    }

    status = setsockopt(
                 fd_sock,
                 ((mcastAddr->ai_family == AF_INET6) ? IPPROTO_IPV6        : IPPROTO_IP),
                 ((mcastAddr->ai_family == AF_INET6) ? IPV6_MULTICAST_HOPS : IP_MULTICAST_TTL),
                 &ttl,
                 sizeof( ttl )
             );
    if (status != 0)
    {
        perror( "setsockopt" );
        printf("\n");
        goto _EXIT;
    }

    /* set the sending interface */
    if (mcastAddr->ai_family == AF_INET)
    {
        in_addr_t iface = INADDR_ANY;

        status = setsockopt(
                     fd_sock,
                     IPPROTO_IP,
                     IP_MULTICAST_IF,
                     &iface,
                     sizeof( iface )
                 );
        if (status != 0)
        {
            perror( "setsockopt" );
            printf("\n");
            goto _EXIT;
        }
    }
    if (mcastAddr->ai_family == AF_INET6)
    {
        unsigned int ifindex = 0;

        status = setsockopt(
                     fd_sock,
                     IPPROTO_IPV6,
                     IPV6_MULTICAST_IF,
                     &ifindex,
                     sizeof( ifindex )
                 );
        if (status != 0)
        {
            perror( "setsockopt" );
            printf("\n");
            goto _EXIT;
        }
    }


    printf(
        "SEND>> sending data on multicast group %s:%s\n",
        argv[1],
        MCAST_PORT
    );

    /* send data */
    for (i=2; i<argc; i++)
    {
        status = sendto(
                     fd_sock,
                     argv[i],
                     (strlen(argv[i]) + 1),
                     0,
                     mcastAddr->ai_addr,
                     mcastAddr->ai_addrlen
                 );
        if (status < 0)
        {
            perror( "sendto" );
            printf("\n");
        }
        else
        {
            printf("SEND>  %s\n", argv[i]);
        }
    }

    printf("SEND>>\n\n");


_EXIT:
    /* close socket and exit */
    if (fd_sock >= 0)
    {
        close( fd_sock );
    }
    if ( mcastAddr )
    {
        freeaddrinfo( mcastAddr );
    }

    return 0;
}

