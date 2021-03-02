#ifndef _DHCP_MAIN_H_
#define _DHCP_MAIN_H_

#include "dhcp_types.h"
#include "dhcp_packet.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

struct client_config_t {
    char  *interface;  /* The name of the interface to use */
    uint8 *clientid;   /* Optional client id to use */
    char  *hostname;   /* Optional hostname to use */
    int    ifindex;    /* Index number of the interface to use */
    uint8  arp[6];     /* Our arp address */
};


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

extern struct client_config_t client_config;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

int raw_sock_send(
    struct dhcpMessage *payload,
    uint32  source_ip,
    int     source_port,
    uint32  dest_ip,
    int     dest_port,
    uint8  *dest_arp,
    int     ifindex
);

int raw_sock_recv(struct dhcpMessage *payload, int fd);


#endif
