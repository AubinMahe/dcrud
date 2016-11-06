#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>

typedef struct ioInetSocketAddress_s {

   char           inetAddress[16];
   unsigned short port;

} ioInetSocketAddress;

utilStatus ioInetSocketAddress_new   ( ioInetSocketAddress ** This, const char *   host,
                                                                    unsigned short port );
utilStatus ioInetSocketAddress_delete( ioInetSocketAddress ** This );
utilStatus ioInetSocketAddress_init  ( ioInetSocketAddress *  This, const char *   host,
                                                                    unsigned short port );
int ioInetSocketAddress_comparator( const ioInetSocketAddress * left,
                                    const ioInetSocketAddress * right );

#ifdef __cplusplus
}
#endif
