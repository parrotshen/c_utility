#include "dhcp_log.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define DHCP_LOG_BUFFER  (255)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

bool g_verbose = BOOL_FALSE;
bool g_dump = BOOL_FALSE;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void dhcp_dump(char *name, const void *addr, int len)
{
    uint8 *byte = (uint8 *)addr;
    int    i;

    if (byte == NULL)
    {
        printf("[DUMP] NULL\n");
        printf("\n");
        return;
    }

    printf("[DUMP] %s\n", name);
    printf(" %d bytes\n", len);
    for (i=0; i<len; i++)
    {
        if ((i != 0) && ((i % 16) == 0))
        {
            printf("\n");
        }

        printf(" %02X", byte[i]);
    }
    printf("\n");
    printf("[DUMP]\n");
    printf("\n");
}

void dhcp_print(const char *prefix, const char *fmt, ...)
{
    char  msg[DHCP_LOG_BUFFER+1];
    int   msgLen = 0;
    va_list args;


    /* append prefix to log message */
    msgLen = sprintf(msg, "[%s] ", prefix);

    va_start(args, fmt);

    /* log message copy */
    msgLen += vsnprintf((msg + msgLen),
                        (DHCP_LOG_BUFFER - msgLen),
                        fmt,
                        args);

    va_end(args);

    printf("%s", msg);
}


