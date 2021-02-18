#define _GNU_SOURCE  // for strcasestr()
#include "sos_sip.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define IMS_SCSF_PORT (6060)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

typedef struct _tSipSession
{
    char  request[80];
    char  branch[17];
    char  tag[9];
    char  callId[33];
    unsigned short cSeq;
} tSipSession;


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

static uint8 _sendMsg[MAX_BUFFER_SIZE+1];
static int   _sendMsgLen;

static tSipSession _register;
static tSipSession _inviate;

int g_udp = -1;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

int sos_openUdp(void)
{
    struct sockaddr_in udpAddr;
    socklen_t udpAddrLen = sizeof(struct sockaddr_in);
    int retval;
    int fd;

    /* UDP socket */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("UDP socket");
        return -1;
    }

    /* Bind the UDP socket */
    bzero(&udpAddr, udpAddrLen);
    udpAddr.sin_family      = AF_INET;
    udpAddr.sin_port        = htons( SOS_PORT );
    udpAddr.sin_addr.s_addr = htonl( INADDR_ANY );

    retval = bind(fd, (struct sockaddr *)(&udpAddr), udpAddrLen);
    if (retval < 0)
    {
        perror("UDP bind");
        close( fd );
        return -1;
    }

    g_udp = fd;

    return g_udp;
}

void sos_closeUdp(void)
{
    if (g_udp > 0)
    {
        close( g_udp );
        g_udp = -1;
    }
}


void sos_regToIms(
    char *pDomain,
    char *pIpStr,
    unsigned short portNum,
    int expires
)
{
    struct sockaddr_in  toAddr;
    socklen_t toAddrLen = sizeof(struct sockaddr_in);
    char *pBuf = (char *)_sendMsg;
    int   len;


    bzero(&toAddr, toAddrLen);
    toAddr.sin_family      = AF_INET;
    toAddr.sin_port        = htons( portNum );
    toAddr.sin_addr.s_addr = inet_addr( pIpStr );

    sprintf(_register.request, "REGISTER");
    sos_genRandString(_register.branch, 16);
    _register.branch[16] = 0x00;
    sos_genRandNumber(_register.tag, 8);
    _register.tag[8] = 0x00;
    sos_genRandString(_register.callId, 32);
    _register.callId[32] = 0x00;
    sos_genRandByte(&(_register.cSeq), 2);


    memset(pBuf, 0x00, MAX_BUFFER_SIZE);

    len = sprintf(
              pBuf,
              "REGISTER sip:%s SIP/2.0\r\n",
              pDomain
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Via: SIP/2.0/UDP %s:%d;branch=%s;rport\r\n",
              SOS_IP,
              SOS_PORT,
              _register.branch
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "To: <sip:%s@%s>\r\n",
              SOS_ID,
              SOS_REALM
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "From: <sip:%s@%s>;tag=%s\r\n",
              SOS_ID,
              SOS_REALM,
              _register.tag
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Call-ID: %s\r\n",
              _register.callId
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "CSeq: %u REGISTER\r\n",
              _register.cSeq
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Contact: <sip:%s@%s:%d;transport=udp;sos>;+g.3gpp.smsip\r\n",
              SOS_ID,
              SOS_IP,
              SOS_PORT
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Authorization: Digest nonce=\"\",uri=\"sip:%s\",response=\"\",username=\"%s@%s\",algorithm=AKAv1-MD5,realm=\"%s\"\r\n",
              pDomain,
              SOS_ID,
              pDomain,
              pDomain
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Max-Forwards: 70\r\n"
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Expires: %d\r\n",
              expires
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "\r\n"
          );
    pBuf += len;

    _sendMsgLen = strlen( (char *)_sendMsg );


    LOG_INFO("SOS -- REGISTER --> %s:%u\n", pIpStr, portNum);

    #if (VERBOSE_SIP_MSG)
    printf("%s\n", (char *)_sendMsg);
    #endif

    sendto(
        g_udp,
        _sendMsg,
        _sendMsgLen,
        0,
        (struct sockaddr *)(&toAddr),
        toAddrLen
    );
}

void sos_replyStatus(
    struct sockaddr_in *pAddr,
    uint8              *pMsg,
    int                 msgLen,
    int                 code
)
{
    socklen_t addrLen = sizeof(struct sockaddr_in);
    char *pBuf = (char *)_sendMsg;
    int   len;

    char *pLine;


    memset(pBuf, 0x00, MAX_BUFFER_SIZE);

    switch ( code )
    {
        case 200:
            len = sprintf(pBuf, "SIP/2.0 200 OK\r\n");
            break;
        case 202:
            len = sprintf(pBuf, "SIP/2.0 202 Accepted\r\n");
            break;
        case 600:
        default:
            len = sprintf(pBuf, "SIP/2.0 600 ERROR\r\n");
    }
    pBuf += len;

    pLine = strtok((char *)pMsg, "\r\n");
    while ( pLine )
    {
        if ((0 == strncasecmp("Via:",     pLine, 4)) ||
            (0 == strncasecmp("From:",    pLine, 5)) ||
            (0 == strncasecmp("To:",      pLine, 3)) ||
            (0 == strncasecmp("Call-ID:", pLine, 8)) ||
            (0 == strncasecmp("CSeq:",    pLine, 5)))
        {
            len = sprintf(
                      pBuf,
                      "%s\r\n",
                      pLine
                  );
            pBuf += len;
        }

        pLine = strtok(NULL, "\r\n");
    }

    len = sprintf(
              pBuf,
              "Content-Length: 0\r\n"
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "\r\n"
          );
    pBuf += len;

    _sendMsgLen = strlen( (char *)_sendMsg );


    LOG_INFO(
        "SOS -- %d --> %s:%u\n",
        code,
        inet_ntoa(pAddr->sin_addr),
        ntohs(pAddr->sin_port)
    );

    #if (VERBOSE_SIP_MSG)
    printf("%s\n", (char *)_sendMsg);
    #endif

    sendto(
        g_udp,
        _sendMsg,
        _sendMsgLen,
        0,
        (struct sockaddr *)pAddr,
        addrLen
    );
}

void sos_sendInvite(
    char *pIpStr,
    unsigned short portNum
)
{
    struct sockaddr_in  toAddr;
    socklen_t toAddrLen = sizeof(struct sockaddr_in);
    char *pBuf = (char *)_sendMsg;
    int   len;


    bzero(&toAddr, toAddrLen);
    toAddr.sin_family      = AF_INET;
    toAddr.sin_port        = htons( portNum );
    toAddr.sin_addr.s_addr = inet_addr( pIpStr );

    sprintf(_inviate.request, "INVITE");
    sos_genRandString(_inviate.branch, 16);
    _inviate.branch[16] = 0x00;
    sos_genRandNumber(_inviate.tag, 8);
    _inviate.tag[8] = 0x00;
    sos_genRandString(_inviate.callId, 32);
    _inviate.callId[32] = 0x00;
    sos_genRandByte(&(_inviate.cSeq), 2);


    memset(pBuf, 0x00, MAX_BUFFER_SIZE);

    #if 0
    len = sprintf(
              pBuf,
              "INVITE tel:911 SIP/2.0\r\n"
          );
    pBuf += len;
    #else
    len = sprintf(
              pBuf,
              "INVITE urn:service:sos.police SIP/2.0\r\n"
          );
    pBuf += len;
    #endif
    len = sprintf(
              pBuf,
              "Via: SIP/2.0/UDP %s:%d;branch=%s;rport\r\n",
              SOS_IP,
              SOS_PORT,
              _inviate.branch
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Route: <sip:orig@scscf.%s:%d;lr>\r\n",
              g_imsDomain,
              IMS_SCSF_PORT
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "To: <sip:%s@%s>\r\n",
              SOS_ID,
              SOS_REALM
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "From: <sip:%s@%s>;tag=%s\r\n",
              SOS_ID,
              SOS_REALM,
              _inviate.tag
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Call-ID: %s\r\n",
              _inviate.callId
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "CSeq: %u INVITE\r\n",
              _inviate.cSeq
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Contact: <sip:%s@%s:%d;transport=udp>\r\n",
              SOS_ID,
              SOS_IP,
              SOS_PORT
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Content-Type: application/sdp\r\n"
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Content-Length: 0\r\n"
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Max-Forwards: 70\r\n"
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "\r\n"
          );
    pBuf += len;

    _sendMsgLen = strlen( (char *)_sendMsg );


    LOG_INFO("SOS -- INVITE --> %s:%u\n", pIpStr, portNum);

    #if (VERBOSE_SIP_MSG)
    printf("%s\n", (char *)_sendMsg);
    #endif

    sendto(
        g_udp,
        _sendMsg,
        _sendMsgLen,
        0,
        (struct sockaddr *)(&toAddr),
        toAddrLen
    );
}

void sos_sendAck(
    char *pIpStr,
    unsigned short portNum
)
{
    struct sockaddr_in  toAddr;
    socklen_t toAddrLen = sizeof(struct sockaddr_in);
    char *pBuf = (char *)_sendMsg;
    int   len;


    bzero(&toAddr, toAddrLen);
    toAddr.sin_family      = AF_INET;
    toAddr.sin_port        = htons( portNum );
    toAddr.sin_addr.s_addr = inet_addr( pIpStr );

    _inviate.cSeq++;


    memset(pBuf, 0x00, MAX_BUFFER_SIZE);

    len = sprintf(
              pBuf,
              "ACK tel:911 SIP/2.0\r\n"
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Via: SIP/2.0/UDP %s:%d;branch=%s;rport\r\n",
              SOS_IP,
              SOS_PORT,
              _inviate.branch
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Route: <sip:orig@scscf.%s:%d;lr>\r\n",
              g_imsDomain,
              IMS_SCSF_PORT
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "To: <sip:%s@%s>\r\n",
              SOS_ID,
              SOS_REALM
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "From: <sip:%s@%s>;tag=%s\r\n",
              SOS_ID,
              SOS_REALM,
              _inviate.tag
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Call-ID: %s\r\n",
              _inviate.callId
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "CSeq: %u INVITE\r\n",
              _inviate.cSeq
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Contact: <sip:%s@%s:%d;transport=udp>\r\n",
              SOS_ID,
              SOS_IP,
              SOS_PORT
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Content-Length: 0\r\n"
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Max-Forwards: 70\r\n"
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "\r\n"
          );
    pBuf += len;

    _sendMsgLen = strlen( (char *)_sendMsg );


    LOG_INFO("SOS -- ACK --> %s:%u\n", pIpStr, portNum);

    #if (VERBOSE_SIP_MSG)
    printf("%s\n", (char *)_sendMsg);
    #endif

    sendto(
        g_udp,
        _sendMsg,
        _sendMsgLen,
        0,
        (struct sockaddr *)(&toAddr),
        toAddrLen
    );
}

void sos_sendBye(
    char *pIpStr,
    unsigned short portNum
)
{
    struct sockaddr_in  toAddr;
    socklen_t toAddrLen = sizeof(struct sockaddr_in);
    char *pBuf = (char *)_sendMsg;
    int   len;


    bzero(&toAddr, toAddrLen);
    toAddr.sin_family      = AF_INET;
    toAddr.sin_port        = htons( portNum );
    toAddr.sin_addr.s_addr = inet_addr( pIpStr );

    _inviate.cSeq++;


    memset(pBuf, 0x00, MAX_BUFFER_SIZE);

    len = sprintf(
              pBuf,
              "BYE tel:911 SIP/2.0\r\n"
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Via: SIP/2.0/UDP %s:%d;branch=%s;rport\r\n",
              SOS_IP,
              SOS_PORT,
              _inviate.branch
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Route: <sip:orig@scscf.%s:%d;lr>\r\n",
              g_imsDomain,
              IMS_SCSF_PORT
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "To: <sip:%s@%s>\r\n",
              SOS_ID,
              SOS_REALM
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "From: <sip:%s@%s>;tag=%s\r\n",
              SOS_ID,
              SOS_REALM,
              _inviate.tag
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Call-ID: %s\r\n",
              _inviate.callId
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "CSeq: %u INVITE\r\n",
              _inviate.cSeq
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Contact: <sip:%s@%s:%d;transport=udp>\r\n",
              SOS_ID,
              SOS_IP,
              SOS_PORT
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Content-Length: 0\r\n"
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "Max-Forwards: 70\r\n"
          );
    pBuf += len;
    len = sprintf(
              pBuf,
              "\r\n"
          );
    pBuf += len;

    _sendMsgLen = strlen( (char *)_sendMsg );


    LOG_INFO("SOS -- BYE --> %s:%u\n", pIpStr, portNum);

    #if (VERBOSE_SIP_MSG)
    printf("%s\n", (char *)_sendMsg);
    #endif

    sendto(
        g_udp,
        _sendMsg,
        _sendMsgLen,
        0,
        (struct sockaddr *)(&toAddr),
        toAddrLen
    );
}


void sos_msgHdlr(struct sockaddr_in *pAddr, uint8 *pMsg, int msgLen)
{
    char   token[80];
    char  *ch;
    int    i;


    /* get the 1st token */
    for (i=0, ch=(char *)pMsg; (*ch) && (*ch != 0x20); i++, ch++)
    {
        token[i] = (*ch);
    }
    token[i] = 0x00;

    if (0 == strcasecmp("SIP/2.0", token))
    {
        /* ignore the 1st token */
        ch++;
        /* get the 2nd token */
        for (i=0; (*ch) && (*ch != 0x20); i++, ch++)
        {
            token[i] = (*ch);
        }
        token[i] = 0x00;

        LOG_INFO(
            "SOS <-- %s -- %s:%u\n",
            token,
            inet_ntoa( pAddr->sin_addr ),
            ntohs( pAddr->sin_port )
        );

        #if (VERBOSE_SIP_MSG)
        printf("%s\n", (char *)pMsg);
        #endif


    }
    else
    {
        LOG_INFO(
            "SOS <-- %s -- %s:%u\n",
            token,
            inet_ntoa( pAddr->sin_addr ),
            ntohs( pAddr->sin_port )
        );
        printf("%s\n", pMsg);

        sos_replyStatus(pAddr, pMsg, msgLen, 200);
    }
}

