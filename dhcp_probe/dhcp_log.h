#ifndef _DHCP_LOG_H_
#define _DHCP_LOG_H_

#include "dhcp_types.h"


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define LOG_TRAC()        dhcp_print("TRAC", "%s:%d\n", __FUNCTION__, __LINE__)
#define LOG_INFO( a... )  dhcp_print("INFO", ##a)
#define LOG_WARN( a... )  dhcp_print("WARN", ##a)
#define LOG_ERRO( a... )  dhcp_print("ERRO", ##a)
#define LOG_VERB( a... ) \
    if ( g_verbose ) \
    { \
        dhcp_print("VERB", ##a ); \
    }
#define LOG_DUMP(addr, size) \
    if ( g_dump ) \
    { \
        dhcp_dump(#addr, addr, size); \
    }


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

extern bool g_verbose;
extern bool g_dump;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

void dhcp_dump(char *name, const void *addr, int len);

void dhcp_print(
    const char *prefix,
    const char *fmt,
    ...
) __attribute__ ((format (printf, 2, 3)));


#endif
