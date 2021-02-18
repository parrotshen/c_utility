#ifndef _SOS_SIP_H_
#define _SOS_SIP_H_

#include "sos_common.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define MAX_BUFFER_SIZE (4095)

#define SOS_ID    "alice"
#define SOS_REALM "ims.mnc005.mcc450.3gppnetwork.org"
#define SOS_IP    "192.168.2.100"
#define SOS_PORT  (55688)

#define VERBOSE_SIP_MSG (0)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

extern int g_udp;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

int sos_openUdp(void);
void sos_closeUdp(void);


void sos_regToIms(
    char *pDomain,
    char *pIpStr,
    unsigned short portNum,
    int expires
);

void sos_replyStatus(
    struct sockaddr_in *pAddr,
    uint8              *pMsg,
    int                 msgLen,
    int                 code
);

void sos_sendInvite(
    char *pIpStr,
    unsigned short portNum
);

void sos_sendAck(
    char *pIpStr,
    unsigned short portNum
);

void sos_sendBye(
    char *pIpStr,
    unsigned short portNum
);


void sos_msgHdlr(struct sockaddr_in *pAddr, uint8 *pMsg, int msgLen);


#endif //_SOS_SIP_H_
