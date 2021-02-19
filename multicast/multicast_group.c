#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef IN_IS_ADDR_MULTICAST 
#define IN_IS_ADDR_MULTICAST(a)  ((((in_addr_t)(a)) & 0xf0000000) == 0xe0000000) 
#endif 

#ifndef IN6_IS_ADDR_MULTICAST 
#define IN6_IS_ADDR_MULTICAST(a) ((a)->s6_addr[0] == 0xff) 
#endif 


/*
* struct in_addr {
*     unsigned long  s_addr;  // load with inet_aton()
* };
*
* struct sockaddr_in {
*     short           sin_family;   // e.g. AF_INET
*     unsigned short  sin_port;     // e.g. htons(3490)
*     struct in_addr  sin_addr;     // see struct in_addr
*     char            sin_zero[8];  // zero this if you want to
* };
* 
* struct ip_mreq
* {
*     struct in_addr  imr_multiaddr;  // IP multicast address of group
*     struct in_addr  imr_interface;  // local IP address of interface
* };
*/


int joinGroup(char *pGroup)
{
    struct ip_mreq mreq;
    struct in_addr addr;
    int retval;
    int fd;

    addr.s_addr = inet_addr( pGroup );
    if ( !IN_IS_ADDR_MULTICAST( htonl(addr.s_addr) ) )
    {
        printf("ERR: %s is not a multicast group\n", pGroup);
        return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror( "socket" );
        return -1;
    }

    /* specify the multicast group */
    mreq.imr_multiaddr.s_addr = addr.s_addr;

    /* accept multicast from any interface */
    mreq.imr_interface.s_addr = htonl( INADDR_ANY );

    /* join the multicast address */
    retval = setsockopt(
                 fd,
                 IPPROTO_IP,
                 IP_ADD_MEMBERSHIP,
                 &mreq,
                 sizeof( mreq )
             );
    if (retval != 0)
    {
        perror( "setsockopt" );
        close( fd );
        return -1;
    }

    printf("join the multicast group: %s\n", pGroup);
    return fd;
}

int leaveGroup(char *pGroup)
{
    struct ip_mreq mreq;
    struct in_addr addr;
    int retval;
    int fd;

    addr.s_addr = inet_addr( pGroup );
    if ( !IN_IS_ADDR_MULTICAST( htonl(addr.s_addr) ) )
    {
        printf("ERR: %s is not a multicast group\n", pGroup);
        return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror( "socket" );
        return -1;
    }

    /* specify the multicast group */
    mreq.imr_multiaddr.s_addr = addr.s_addr;

    /* accept multicast from any interface */
    mreq.imr_interface.s_addr = htonl( INADDR_ANY );

    /* leave the multicast address */
    retval = setsockopt(
                 fd,
                 IPPROTO_IP,
                 IP_DROP_MEMBERSHIP,
                 &mreq,
                 sizeof( mreq )
             );
    if (retval != 0)
    {
        perror( "setsockopt" );
        close( fd );
        return -1;
    }

    printf("leave the multicast group: %s\n", pGroup);
    return fd;
}

void help(void)
{
    /*
    * Setup routing rule before join/leave a multicast group.
    *
    * route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0
    *
    */
    printf("Usage: mgroup [OPTION]...\n");
    printf("\n");
    printf("  -j ADDR    Join a multicast group.\n");
    printf("  -l ADDR    Leave a multicast group.\n");
    printf("  -h         Show this help message.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    char buffer[256];
    int sock = -1;
    int retval;
    int ch;

    if (1 == argc)
    {
        help();
        return -1;
    }

    opterr = 0;
    while ((ch=getopt(argc, argv, "j:l:h")) != -1)
    {
        switch ( ch )
        {
            case 'j':
                sock = joinGroup( optarg );
                break;
            case 'l':
                sock = leaveGroup( optarg );
                break;
            case 'h':
            default:
                help();
                return 0;
        }
    }

    printf("Enter 'exit' to terminate ...\n");
    while (1)
    {
        retval = read(STDIN_FILENO, buffer, 256);
        if (retval > 0)
        {
            if (strncmp(buffer, "exit", 4) == 0) break;
        }
    }

    if (sock > 0)
    {
        close( sock );
    }

    return 0;
}

