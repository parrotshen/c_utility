#include "dhcp_main.h"
#include "dhcp_options.h"
#include "dhcp_packet.h"
#include "dhcp_log.h"
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

struct client_config_t client_config = {
    /* Default options. */
    interface: "eth0",
    clientid: NULL,
    hostname: NULL,
    ifindex: 0,
    arp: "\0\0\0\0\0\0",  /* appease gcc-3.0 */
};


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

/* Lookup MAC address */
char *find_arp(uint8 *ip)
{
    static char  line[256];
    char  target[20];
    FILE *fp = NULL;
    char *token;

    sprintf(target, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

    if ((fp=fopen("/proc/net/arp", "r")) == NULL)
    {
        LOG_ERRO("cannot open /proc/net/arp\n");
        return NULL;
    }

    while ( !feof( fp ) )
    {
        fgets(line, 255, fp);
        if ( line[0] )
        {
            token = strtok(line, " \t");
            if ((token != NULL) && (0 == strcmp(token, target)))
            {
                do
                {
                    if ((strlen(token) >= 17) && (':' == token[2]))
                    {
                        /* found */
                        fclose( fp );
                        return token;
                    }
                    token = strtok(NULL, " \t");
                } while ( token );
            }
        }
    }

    fclose( fp );

    return NULL;
}

/* Create a random xid */
uint32 random_xid(void)
{
    static int initialized = 0;

    if ( !initialized )
    {
        uint32 seed;
        int fd;

        fd = open("/dev/urandom", 0);
        if ((fd < 0) || (read(fd, &seed, sizeof(seed)) < 0))
        {
            LOG_WARN(
                "could not load seed from /dev/urandom: %s",
                strerror(errno)
            );
            seed = time(0);
        }
        if (fd >= 0) close(fd);
        srand(seed);
        initialized++;
    }

    return rand();
}

/* Constuct a ip/udp header for a packet, and specify the source and dest hardware address */
int raw_sock_send(
    struct dhcpMessage *payload,
    uint32  source_ip,
    int     source_port,
    uint32  dest_ip,
    int     dest_port,
    uint8  *dest_arp,
    int     ifindex
)
{
    struct udp_dhcp_packet packet;
    struct sockaddr_ll dest;
    int result;
    int fd;

    if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0)
    {
        LOG_ERRO("socket call failed: %s\n", strerror(errno));
        return -1;
    }

    memset(&dest, 0, sizeof(dest));
    memset(&packet, 0, sizeof(packet));

    dest.sll_family = AF_PACKET;
    dest.sll_protocol = htons(ETH_P_IP);
    dest.sll_ifindex = ifindex;
    dest.sll_halen = 6;
    memcpy(dest.sll_addr, dest_arp, 6);
    if (bind(fd, (struct sockaddr *)&dest, sizeof(struct sockaddr_ll)) < 0)
    {
        LOG_ERRO("bind call failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    packet.ip.protocol = IPPROTO_UDP;
    packet.ip.saddr = source_ip;
    packet.ip.daddr = dest_ip;
    packet.udp.source = htons(source_port);
    packet.udp.dest = htons(dest_port);
    packet.udp.len = htons(sizeof(packet.udp) + sizeof(struct dhcpMessage)); /* cheat on the psuedo-header */
    packet.ip.tot_len = packet.udp.len;
    memcpy(&(packet.data), payload, sizeof(struct dhcpMessage));
    packet.udp.check = checksum(&packet, sizeof(struct udp_dhcp_packet));
    
    packet.ip.tot_len = htons(sizeof(struct udp_dhcp_packet));
    packet.ip.ihl = sizeof(packet.ip) >> 2;
    packet.ip.version = IPVERSION;
    packet.ip.ttl = IPDEFTTL;
    packet.ip.check = checksum(&(packet.ip), sizeof(packet.ip));

    result = sendto(
                 fd,
                 &packet,
                 sizeof(struct udp_dhcp_packet),
                 0,
                 (struct sockaddr *)&dest,
                 sizeof(dest)
             );
    if (result <= 0)
    {
        LOG_ERRO("write on socket failed: %s\n", strerror(errno));
    }
    close(fd);

    return result;
}

/* return -1 on errors that are fatal for the socket, -2 for those that aren't */
int raw_sock_recv(struct dhcpMessage *payload, int fd)
{
    struct udp_dhcp_packet packet;
    uint32  source, dest;
    uint16  check;
    int     bytes;

    memset(&packet, 0, sizeof(struct udp_dhcp_packet));
    bytes = read(fd, &packet, sizeof(struct udp_dhcp_packet));
    if (bytes < 0)
    {
        LOG_VERB("couldn't read on raw listening socket -- ignoring\n");
        usleep(500000); /* possible down interface, looping condition */
        return -1;
    }
    
    if (bytes < (int)(sizeof(struct iphdr) + sizeof(struct udphdr)))
    {
        LOG_VERB("message too short, ignoring\n");
        return -2;
    }
    
    if (bytes < ntohs(packet.ip.tot_len))
    {
        LOG_VERB("truncated packet\n");
        return -2;
    }
    
    /* ignore any extra garbage bytes */
    bytes = ntohs(packet.ip.tot_len);
    
    /* Make sure its the right packet for us, and that it passes sanity checks */
    if ((packet.ip.protocol != IPPROTO_UDP) ||
        (packet.ip.version != IPVERSION) ||
        (packet.ip.ihl != sizeof(packet.ip) >> 2) ||
        (packet.udp.dest != htons(CLIENT_PORT)) ||
        (bytes > (int)sizeof(struct udp_dhcp_packet)) ||
        (ntohs(packet.udp.len) != (short)(bytes - sizeof(packet.ip))))
    {
        //LOG_VERB("unrelated/bogus packet\n");
        return -2;
    }

    /* check IP checksum */
    check = packet.ip.check;
    packet.ip.check = 0;
    if (check != checksum(&(packet.ip), sizeof(packet.ip)))
    {
        LOG_VERB("bad IP header checksum, ignoring\n");
        return -1;
    }
    
    /* verify the UDP checksum by replacing the header with a psuedo header */
    source = packet.ip.saddr;
    dest = packet.ip.daddr;
    check = packet.udp.check;
    packet.udp.check = 0;
    memset(&packet.ip, 0, sizeof(packet.ip));

    packet.ip.protocol = IPPROTO_UDP;
    packet.ip.saddr = source;
    packet.ip.daddr = dest;
    packet.ip.tot_len = packet.udp.len; /* cheat on the psuedo-header */
    if (check && check != checksum(&packet, bytes))
    {
        LOG_ERRO("packet with bad UDP checksum received, ignoring\n");
        return -2;
    }
    
    memcpy(payload, &(packet.data), bytes - (sizeof(packet.ip) + sizeof(packet.udp)));
    
    if (ntohl(payload->cookie) != DHCP_MAGIC)
    {
        LOG_ERRO("received bogus message (bad magic) -- ignoring\n");
        return -2;
    }

    LOG_VERB("oooooh!!! got some!\n");
    return (bytes - (sizeof(packet.ip) + sizeof(packet.udp)));
}


int read_interface(
    char   *interface,
    int    *ifindex,
    uint32 *addr,
    uint8  *arp
)
{
    struct sockaddr_in *our_ip;
    struct ifreq ifr;
    int fd;

    memset(&ifr, 0, sizeof(struct ifreq));
    if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0)
    {
        ifr.ifr_addr.sa_family = AF_INET;
        strcpy(ifr.ifr_name, interface);

        if (addr)
        { 
            if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
            {
                our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
                *addr = our_ip->sin_addr.s_addr;
                LOG_VERB("%s (our ip) = %s\n", ifr.ifr_name, inet_ntoa(our_ip->sin_addr));
            }
            else
            {
                LOG_ERRO("SIOCGIFADDR failed, is the interface up and configured?: %s\n", 
                        strerror(errno));
                return -1;
            }
        }
        
        if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0)
        {
            LOG_VERB("adapter index %d\n", ifr.ifr_ifindex);
            *ifindex = ifr.ifr_ifindex;
        }
        else
        {
            LOG_ERRO("SIOCGIFINDEX failed!: %s\n", strerror(errno));
            return -1;
        }

        if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)
        {
            memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
            LOG_VERB("adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x\n",
                arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);
        }
        else
        {
            LOG_ERRO("SIOCGIFHWADDR failed!: %s\n", strerror(errno));
            return -1;
        }
    }
    else
    {
        LOG_ERRO("socket failed!: %s\n", strerror(errno));
        return -1;
    }

    close(fd);

    return 0;
}

void help(void)
{
    printf("Usage: dhcprobe [OPTION]...\n");
    printf("\n");
    printf("  -i INTERFACE    Assign a network interface.\n");
    printf("  -v              Enable verbose log.\n");
    printf("  -d              Enable dump log.\n");
    printf("  -h              Show this help.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    struct sockaddr_ll  sock;
    struct timeval  timeout;
    fd_set  flags;
    int     sock_fd;
    int     retval;

    struct dhcpMessage packet;
    uint8  *message = NULL;
    uint32  my_ip = 0;
    uint32  xid = 0;
    int     count = 10;
    int     len;
    int     ch;


    opterr = 0;
    while ((ch=getopt(argc, argv, "i:vdh")) != -1)
    {
        switch ( ch )
        {
            case 'i':
                client_config.interface = optarg;
                break;
            case 'v':
                g_verbose = BOOL_TRUE;
                break;
            case 'd':
                g_dump = BOOL_TRUE;
                break;
            case 'h':
            default:
                help();
                return 0;
        }
    }

    if (read_interface(
            client_config.interface,
            &client_config.ifindex, 
            NULL,
            client_config.arp) < 0)
    {
        LOG_ERRO("fail to get interface %s\n", client_config.interface);
        return -1;
    }

    if ( !client_config.clientid )
    {
        client_config.clientid = malloc(6 + 3);
        client_config.clientid[OPT_CODE] = DHCP_CLIENT_ID;
        client_config.clientid[OPT_LEN]  = 7;
        client_config.clientid[OPT_DATA] = 1;
        memcpy(client_config.clientid + 3, client_config.arp, 6);
    }


    LOG_VERB("opening raw socket on ifindex %d\n", client_config.ifindex);
    if ((sock_fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0)
    {
        LOG_ERRO("socket call failed: %s\n", strerror(errno));
        return -1;
    }
    
    sock.sll_family   = AF_PACKET;
    sock.sll_protocol = htons(ETH_P_IP);
    sock.sll_ifindex  = client_config.ifindex;
    if (bind(sock_fd, (struct sockaddr *) &sock, sizeof(sock)) < 0)
    {
        LOG_ERRO("bind call failed: %s\n", strerror(errno));
        close( sock_fd );
        return -1;
    }


    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;

    FD_ZERO( &flags );

    while (1)
    {
        FD_CLR(sock_fd, &flags);
        FD_SET(sock_fd, &flags);


        retval = select(sock_fd+1, &flags, NULL, NULL, &timeout);

        if (retval > 0)
        {
            if (FD_ISSET(sock_fd, &flags))
            {
                len = raw_sock_recv(&packet, sock_fd);

                if (len > 0)
                {
                    LOG_DUMP(&packet, len);

                    if (packet.xid != xid)
                    {
                        LOG_ERRO(
                            "ignoring XID 0x%08x <--> 0x%08x\n",
                            packet.xid,
                            xid
                        );
                        break;
                    }
                    
                    if ((message = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL)
                    {
                        LOG_ERRO("couldnt get option from packet -- ignoring\n");
                        break;
                    }

                    if (*message == DHCPOFFER)
                    {
                        uint8  *option;
                        char   *server_mac;
                        uint32  server_ip;

                        xid   = packet.xid;
                        my_ip = ntohl( packet.yiaddr );

                        LOG_INFO("\n");
                        if ((option = get_option(&packet, DHCP_SERVER_ID)))
                        {
                            BYTE_ARRAY_TO_UINT32(option, server_ip);

                            server_mac = find_arp( option );

                            if ( server_mac )
                            {
                                LOG_INFO(
                                    "server IP: %u.%u.%u.%u (%s)\n",
                                    ((server_ip >> 24) & 0xFF),
                                    ((server_ip >> 16) & 0xFF),
                                    ((server_ip >>  8) & 0xFF),
                                    ((server_ip      ) & 0xFF),
                                    server_mac
                                );
                            }
                            else
                            {
                                LOG_INFO(
                                    "server IP: %u.%u.%u.%u\n",
                                    ((server_ip >> 24) & 0xFF),
                                    ((server_ip >> 16) & 0xFF),
                                    ((server_ip >>  8) & 0xFF),
                                    ((server_ip      ) & 0xFF)
                                );
                            }
                        }
                        else
                        {
                            LOG_ERRO("No server ID in message\n");
                        }
                        LOG_INFO(
                            "client IP: %u.%u.%u.%u\n",
                            ((my_ip >> 24) & 0xFF),
                            ((my_ip >> 16) & 0xFF),
                            ((my_ip >>  8) & 0xFF),
                            ((my_ip      ) & 0xFF)
                        );
                        LOG_INFO("\n");

                        break;
                    }
                }
                //else
                //{
                //    LOG_ERRO("raw_sock_recv ... failed(%d)\n", len);
                //    break;
                //}
            }
        }
        else if (retval == 0)
        {
            timeout.tv_sec  = 1;
            timeout.tv_usec = 0;

            if (0 == xid)
            {
                xid   = random_xid();
                my_ip = 0;

                LOG_VERB("XID 0x%08x\n", xid);
                send_discover(xid, my_ip);
            }
            else
            {
                /* wait for 10 seconds */
                if (count-- <= 0)
                {
                    LOG_INFO("No any DHCP server was found\n");
                    break;
                }
            }
        }
        else
        {
            LOG_ERRO("select call failed: %s\n", strerror(errno));
            break;
        }
    }

    close( sock_fd );

    return 0;
}

