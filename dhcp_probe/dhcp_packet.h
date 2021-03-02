#ifndef _DHCP_PACKET_H_
#define _DHCP_PACKET_H_

#include "dhcp_types.h"
#include <netinet/udp.h>
#include <netinet/ip.h>


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

/* DHCP protocol -- see RFC 2131 */
#define SERVER_PORT  67
#define CLIENT_PORT  68

/* miscellaneous defines */
#define MAC_BCAST_ADDR  (uint8 *)"\xff\xff\xff\xff\xff\xff"
#define OPT_CODE  0
#define OPT_LEN   1
#define OPT_DATA  2

#define DHCP_MAGIC  0x63825363


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

struct dhcpMessage {
    uint8   op;
    uint8   htype;
    uint8   hlen;
    uint8   hops;
    uint32  xid;
    uint16  secs;
    uint16  flags;
    uint32  ciaddr;
    uint32  yiaddr;
    uint32  siaddr;
    uint32  giaddr;
    uint8   chaddr[16];
    uint8   sname[64];
    uint8   file[128];
    uint32  cookie;
    uint8   options[308]; /* 312 - cookie */ 
};

struct udp_dhcp_packet {
    struct iphdr        ip;
    struct udphdr       udp;
    struct dhcpMessage  data;
};


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

uint16 checksum(void *addr, int count);

void init_packet(struct dhcpMessage *packet, char type);

int send_discover(uint32 xid, uint32 ip_addr);


#endif
