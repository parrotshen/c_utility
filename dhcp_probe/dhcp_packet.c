#include "dhcp_packet.h"
#include "dhcp_options.h"
#include "dhcp_main.h"
#include "dhcp_log.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define VERSION  "0.9.8-pre"

#define BOOTREQUEST   1
#define BOOTREPLY     2

#define ETH_10MB      1
#define ETH_10MB_LEN  6


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

uint16 checksum(void *addr, int count)
{
    /* Compute Internet Checksum for "count" bytes
     *         beginning at location "addr".
     */
    register int sum = 0;
    uint16 *source = (uint16 *) addr;

    while (count > 1)  {
        /*  This is the inner loop */
        sum += *source++;
        count -= 2;
    }

    /*  Add left-over byte, if any */
    if (count > 0) {
        /* Make sure that the left-over byte is added correctly both
         * with little and big endian hosts */
        uint16 tmp = 0;
        *(uint8 *) (&tmp) = * (uint8 *) source;
        sum += tmp;
    }
    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

void init_header(struct dhcpMessage *packet, char type)
{
    memset(packet, 0, sizeof(struct dhcpMessage));

    switch (type)
    {
        case DHCPDISCOVER:
        case DHCPREQUEST:
        case DHCPRELEASE:
        case DHCPINFORM:
            packet->op = BOOTREQUEST;
            break;
        case DHCPOFFER:
        case DHCPACK:
        case DHCPNAK:
            packet->op = BOOTREPLY;
    }
    packet->htype = ETH_10MB;
    packet->hlen = ETH_10MB_LEN;
    packet->cookie = htonl(DHCP_MAGIC);
    packet->options[0] = DHCP_END;

    add_simple_option(packet->options, DHCP_MESSAGE_TYPE, type);
}

/* initialize a packet with the proper defaults */
void init_packet(struct dhcpMessage *packet, char type)
{
    struct vendor  {
        char vendor, length;
        char str[sizeof("udhcp "VERSION)];
    } vendor_id = { DHCP_VENDOR,  sizeof("udhcp "VERSION) - 1, "udhcp "VERSION};
    
    init_header(packet, type);
    memcpy(packet->chaddr, client_config.arp, 6);
    add_option_string(packet->options, client_config.clientid);
    if (client_config.hostname) add_option_string(packet->options, (uint8 *)client_config.hostname);
    add_option_string(packet->options, (uint8 *)&vendor_id);
}


/* Add a paramater request list for stubborn DHCP servers. Pull the data
 * from the struct in options.c. Don't do bounds checking here because it
 * goes towards the head of the packet. */
void add_requests(struct dhcpMessage *packet)
{
    int end = end_option(packet->options);
    int i, len = 0;

    packet->options[end + OPT_CODE] = DHCP_PARAM_REQ;
    for (i = 0; options[i].code; i++)
    {
        if (options[i].flags & OPTION_REQ)
        {
            packet->options[end + OPT_DATA + len++] = options[i].code;
        }
    }
    packet->options[end + OPT_LEN] = len;
    packet->options[end + OPT_DATA + len] = DHCP_END;
}


/* Broadcast a DHCP discover packet to the network, with an optionally requested IP */
int send_discover(uint32 xid, uint32 ip_addr)
{
    struct dhcpMessage packet;
    int result;

    init_packet(&packet, DHCPDISCOVER);

    packet.xid = xid;

    if (ip_addr)
    {
        add_simple_option(packet.options, DHCP_REQUESTED_IP, ip_addr);
    }

    add_requests( &packet );

    LOG_VERB("sending discover ...\n");
    result = raw_sock_send(
                 &packet,
                 INADDR_ANY,
                 CLIENT_PORT,
                 INADDR_BROADCAST, 
                 SERVER_PORT,
                 MAC_BCAST_ADDR,
                 client_config.ifindex
             );

    return result;
}

