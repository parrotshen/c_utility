#ifndef _DHCP_OPTIONS_H_
#define _DHCP_OPTIONS_H_

#include "dhcp_types.h"
#include "dhcp_packet.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define OPTION_REQ   0x10 /* have the client request this option */
#define OPTION_LIST  0x20 /* There can be a list of 1 or more of these */

#define TYPE_MASK    0x0F

/* DHCP message types */
#define DHCPDISCOVER  (1)
#define DHCPOFFER     (2)
#define DHCPREQUEST   (3)
#define DHCPDECLINE   (4)
#define DHCPACK       (5)
#define DHCPNAK       (6)
#define DHCPRELEASE   (7)
#define DHCPINFORM    (8)

/* DHCP option codes (partial list) */
#define DHCP_PADDING        0x00
#define DHCP_SUBNET         0x01
#define DHCP_TIME_OFFSET    0x02
#define DHCP_ROUTER         0x03
#define DHCP_TIME_SERVER    0x04
#define DHCP_NAME_SERVER    0x05
#define DHCP_DNS_SERVER     0x06
#define DHCP_LOG_SERVER     0x07
#define DHCP_COOKIE_SERVER  0x08
#define DHCP_LPR_SERVER     0x09
#define DHCP_HOST_NAME      0x0c
#define DHCP_BOOT_SIZE      0x0d
#define DHCP_DOMAIN_NAME    0x0f
#define DHCP_SWAP_SERVER    0x10
#define DHCP_ROOT_PATH      0x11
#define DHCP_IP_TTL         0x17
#define DHCP_MTU            0x1a
#define DHCP_BROADCAST      0x1c
#define DHCP_NTP_SERVER     0x2a
#define DHCP_WINS_SERVER    0x2c
#define DHCP_REQUESTED_IP   0x32
#define DHCP_LEASE_TIME     0x33
#define DHCP_OPTION_OVER    0x34
#define DHCP_MESSAGE_TYPE   0x35
#define DHCP_SERVER_ID      0x36
#define DHCP_PARAM_REQ      0x37
#define DHCP_MESSAGE        0x38
#define DHCP_MAX_SIZE       0x39
#define DHCP_T1             0x3a
#define DHCP_T2             0x3b
#define DHCP_VENDOR         0x3c
#define DHCP_CLIENT_ID      0x3d
#define DHCP_END            0xFF


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

enum {
    OPTION_IP=1,
    OPTION_IP_PAIR,
    OPTION_STRING,
    OPTION_BOOLEAN,
    OPTION_U8,
    OPTION_U16,
    OPTION_S16,
    OPTION_U32,
    OPTION_S32
};

struct dhcp_option {
    char   name[10];
    char   flags;
    uint8  code;
};


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

extern struct dhcp_option options[];
extern int option_lengths[];


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

uint8 *get_option(struct dhcpMessage *packet, int code);

int end_option(uint8 *optionptr) ;

int add_option_string(uint8 *optionptr, uint8 *string);

int add_simple_option(uint8 *optionptr, uint8 code, uint32 data);


#endif
