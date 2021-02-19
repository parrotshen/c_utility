#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>  /* getaddrinfo */
#include <arpa/inet.h>


#define MCAST_PORT  "12345"
#define MAX_MSG_SIZE  1023

char g_buf[MAX_MSG_SIZE+1];

int main(int argc, char *argv[])
{
    struct addrinfo *mcastAddr = NULL;
    struct addrinfo *localAddr = NULL;
    struct addrinfo  hints;
    int fd_sock = -1;
    int reuse = 1;
    int status;

    struct sockaddr remoteAddr;
    socklen_t remoteAddrLen;
    int n;


    if (argc != 2)
    {
        printf("Usage: multicast_recv MCAST-ADDRESS\n");
        printf("\n");
        return 0;
    }

    /* resolve the multicast group address */
    memset(&hints, 0x00, sizeof( struct addrinfo ));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags  = AI_NUMERICHOST;

    status = getaddrinfo(argv[1], NULL, &hints, &mcastAddr);
    if (status != 0)
    {
        printf("ERR: getaddrinfo [%s]\n", gai_strerror( status ));
        printf("\n");
        goto _EXIT;
    }


    /* get a local address to bind */
    memset(&hints, 0x00, sizeof( struct addrinfo ));
    hints.ai_family   = mcastAddr->ai_family;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(NULL, MCAST_PORT, &hints, &localAddr);
    if (status != 0)
    {
        printf("ERR: getaddrinfo [%s]\n", gai_strerror( status ));
        printf("\n");
        goto _EXIT;
    }

    /* create socket for receiving datagrams */
    fd_sock = socket(localAddr->ai_family, localAddr->ai_socktype, 0);
    if (fd_sock < 0)
    {
        perror( "socket" );
        printf("\n");
        goto _EXIT;
    }

    /* allow multiple sockets to use the same PORT number */
    status = setsockopt(
                 fd_sock,
                 SOL_SOCKET,
                 SO_REUSEADDR,
                 &reuse,
                 sizeof( reuse )
             );
    if (status != 0)
    {
        perror( "setsockopt" );
        printf("\n");
        goto _EXIT;
    }

    /* bind the local address to the multicast port */
    status = bind(fd_sock, localAddr->ai_addr, localAddr->ai_addrlen);
    if (status != 0)
    {
        perror( "bind" );
        printf("\n");
        goto _EXIT;
    }


    /* get/set socket receive buffer */ 
    #if 0
    {
        int optval = 0; 
        socklen_t optval_len = sizeof(optval); 
        int dfltrcvbuf; 

        status = getsockopt(
                     fd_sock,
                     SOL_SOCKET,
                     SO_RCVBUF,
                     &optval,
                     &optval_len
                 );
        if (status != 0)
        {
            perror( "getsockopt" );
            printf("\n");
            goto _EXIT;
        }
        
        dfltrcvbuf = optval; 
        optval = (MAX_MSG_SIZE + 1); 
        status = setsockopt(
                     fd_sock,
                     SOL_SOCKET,
                     SO_RCVBUF,
                     &optval,
                     sizeof( optval )
                 );
        if (status != 0)
        {
            perror( "setsockopt" );
            printf("\n");
            goto _EXIT;
        }

        optval = 0;
        status = getsockopt(
                     fd_sock,
                     SOL_SOCKET,
                     SO_RCVBUF,
                     &optval,
                     &optval_len
                 );
        if (status != 0)
        {
            perror( "getsockopt" );
            printf("\n");
            goto _EXIT;
        }

        printf(
            "set socket receive buffer from %d to %d ==> %d\n", 
            dfltrcvbuf,
            (MAX_MSG_SIZE + 1),
            optval
        );
    }
    #endif


    /* join multicast group */
    if ((mcastAddr->ai_family  == AF_INET) &&
        (mcastAddr->ai_addrlen == sizeof(struct sockaddr_in)))
    {
        struct ip_mreq mreq;

        /* specify the multicast group */
        memcpy(
            &mreq.imr_multiaddr,
            &(((struct sockaddr_in *)(mcastAddr->ai_addr))->sin_addr),
            sizeof( mreq.imr_multiaddr )
        );

        /* accept multicast from any interface */
        mreq.imr_interface.s_addr = htonl( INADDR_ANY );

        /* join the multicast address */
        status = setsockopt(
                     fd_sock,
                     IPPROTO_IP,
                     IP_ADD_MEMBERSHIP,
                     &mreq,
                     sizeof( mreq )
                 );
        if (status != 0)
        {
            perror( "setsockopt" );
            printf("\n");
            goto _EXIT;
        }
    }
    else if ((mcastAddr->ai_family  == AF_INET6) &&
             (mcastAddr->ai_addrlen == sizeof(struct sockaddr_in6)))
    {
        struct ipv6_mreq mreq;

        /* specify the multicast group */
        memcpy(
            &mreq.ipv6mr_multiaddr,
            &(((struct sockaddr_in6 *)(mcastAddr->ai_addr))->sin6_addr),
            sizeof( mreq.ipv6mr_multiaddr )
        );

        /* accept multicast from any interface */
        mreq.ipv6mr_interface = 0;

        /* join the multicast address */
        status = setsockopt(
                     fd_sock,
                     IPPROTO_IPV6,
                     IPV6_ADD_MEMBERSHIP,
                     &mreq,
                     sizeof( mreq )
                 );
        if (status != 0)
        {
            perror( "setsockopt" );
            printf("\n");
            goto _EXIT;
        }
    }
    else
    {
        printf("ERR: neither IPv4 nor IPv6\n");
        printf("\n");
        goto _EXIT;
    }

    if ( localAddr )
    {
        freeaddrinfo( localAddr );
        localAddr = NULL;
    }
    if ( mcastAddr )
    {
        freeaddrinfo( mcastAddr );
        mcastAddr = NULL;
    }


    printf(
        "RECV>> receiving multicast UDP port(%s) ...\n\n",
        MCAST_PORT
    );

    remoteAddrLen = sizeof(struct sockaddr);

    while (1)
    {
        bzero(&remoteAddr, remoteAddrLen);

        n = recvfrom(
               fd_sock,
               g_buf,
               MAX_MSG_SIZE,
               0,
               &remoteAddr,
               &remoteAddrLen
            );
        if (n < 0)
        {
            perror( "recvfrom" );
            continue;
        }

        g_buf[n] = 0x00;
        if (remoteAddr.sa_family == AF_INET)
        {
            printf(
                "RECV>> from %s:%d on %s\n",
                inet_ntoa( ((struct sockaddr_in *)&remoteAddr)->sin_addr ),
                ntohs( ((struct sockaddr_in *)&remoteAddr)->sin_port ),
                argv[1]
            );
        }
        else
        {
            char ipv6Str[INET6_ADDRSTRLEN];

            inet_ntop(
                AF_INET6,
                &(((struct sockaddr_in6 *)&remoteAddr)->sin6_addr),
                ipv6Str,
                INET6_ADDRSTRLEN
            );
            printf(
                "RECV>> from %s:%d on %s\n",
                ipv6Str,
                ntohs( ((struct sockaddr_in6 *)&remoteAddr)->sin6_port ),
                argv[1]
            );
        }
        printf("RECV>  %s\n\n", g_buf);
    }


_EXIT:
    /* close socket and exit */
    if (fd_sock >= 0)
    {
        close( fd_sock );
    }
    if ( localAddr )
    {
        freeaddrinfo( localAddr );
    }
    if ( mcastAddr )
    {
        freeaddrinfo( mcastAddr );
    }

    return 0;
}

