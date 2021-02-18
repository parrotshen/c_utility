#include "sos_common.h"
#include "sos_sip.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#undef  SELECT_TIMEOUT
#define SELECT_TIMEOUT (5) // second(s)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

static uint8 _udpBuf[MAX_BUFFER_SIZE+1];
static int   _udpBufLen;

static int   _running = 1;

static char  _stdBuf[MAX_BUFFER_SIZE+1];
static int   _stdBufLen;

int g_stdin = 0;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

static void _signalHdlr(int sig)
{
    _running = 0;

    sos_closeUdp();
}

int main(int argc, char *argv[])
{
    #ifdef SELECT_TIMEOUT
    struct timeval timeout;
    #endif
    int   retval;

    struct sockaddr_in udpAddr;
    socklen_t udpAddrLen = sizeof(struct sockaddr_in);

    fd_set flags;


    if (argc > 3)
    {
        strcpy(g_imsDomain, argv[1]);
        strcpy(g_imsIp,     argv[2]);
        g_imsPort = atoi( argv[3] );
    }
    else
    {
        printf("Usage: sip_sos IMS_DOMAIN_NAME IMS_IP_ADDRESS IMS_PORT\n");
        printf("\n");
        printf("EX   : sip_sos ims.mnc001.mcc001.3gppnetwork.org 192.168.2.129 5060\n");
        printf("\n");
        return 0;
    }

    signal(SIGINT,  _signalHdlr);
    signal(SIGKILL, _signalHdlr);
    signal(SIGTERM, _signalHdlr);

    #ifdef SELECT_TIMEOUT
    timeout.tv_sec  = SELECT_TIMEOUT;
    timeout.tv_usec = 0;
    #endif


    /* open UDP socket for SIP messages */
    retval = sos_openUdp();
    if (retval < 0)
    {
        LOG_ERROR("sos_openUdp() ... failed\n");
        goto _EXIT;
    }


    LOG_INFO("\n");
    LOG_INFO("SOS ... running (port %d)\n", SOS_PORT);
    LOG_INFO("\n");


    //sos_regToIms(g_imsDomain, g_imsIp, g_imsPort, 3600);

    //sos_sendInvite(g_imsIp, g_imsPort);


    FD_ZERO(&flags);
    while ( _running )
    {
        FD_CLR(g_stdin, &flags);
        FD_CLR(g_udp, &flags);
        FD_SET(g_stdin, &flags);
        FD_SET(g_udp, &flags);

        #ifdef SELECT_TIMEOUT
        retval = select(g_udp+1, &flags, NULL, NULL, &timeout);
        #else
        retval = select(g_udp+1, &flags, NULL, NULL, NULL);
        #endif

        if (retval > 0)
        {
            if (FD_ISSET(g_stdin, &flags))
            {
                _stdBufLen = read(g_stdin, _stdBuf, MAX_BUFFER_SIZE);
                if (_stdBufLen > 0)
                {
                    REMOVE_CRLF( _stdBuf );

                    //printf("'%s'\n", _stdBuf);

                    if (0 == strcmp("reg", _stdBuf))
                    {
                        sos_regToIms(g_imsDomain, g_imsIp, g_imsPort, 3600);
                    }
                    else if (0 == strcmp("911", _stdBuf))
                    {
                        sos_sendInvite(g_imsIp, g_imsPort);
                    }
                    else if (0 == strcmp("ack", _stdBuf))
                    {
                        sos_sendAck(g_imsIp, g_imsPort);
                    }
                    else if (0 == strcmp("bye", _stdBuf))
                    {
                        sos_sendBye(g_imsIp, g_imsPort);
                    }
                    else if (0 == strcmp("quit", _stdBuf))
                    {
                        //sos_regToIms(g_imsDomain, g_imsIp, g_imsPort, 0);
                        _running = 0;
                        break;
                    }
                    else if (0 == strcmp("help", _stdBuf))
                    {
                        printf("\n");
                        printf("reg  - register to IMS server\n");
                        printf("911  - call 911 (send INVITE)\n");
                        printf("ack  - send ACK\n");
                        printf("bye  - send BYE\n");
                        printf("quit - exit\n");
                        printf("\n");
                    }
                }
            }

            if (FD_ISSET(g_udp, &flags))
            {
                _udpBufLen = recvfrom(
                                 g_udp,
                                 _udpBuf,
                                 MAX_BUFFER_SIZE,
                                 0,
                                 (struct sockaddr *)(&udpAddr),
                                 &udpAddrLen
                             );

                if (_udpBufLen > 2)
                {
                    _udpBuf[_udpBufLen] = 0x00;

                    sos_msgHdlr(&udpAddr, _udpBuf, _udpBufLen);
                }
            }
        }
        else if (retval == 0)
        {
            #ifdef SELECT_TIMEOUT
            timeout.tv_sec  = SELECT_TIMEOUT;
            timeout.tv_usec = 0;
            #endif
        }
        else
        {
            perror("select");
            break;
        }
    }


_EXIT:
    sos_closeUdp();

    LOG_INFO("\n");
    LOG_INFO("SOS ... terminated\n");
    LOG_INFO("\n");

    return 0;
}

