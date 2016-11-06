#include <io/InetSocketAddress.h>
#include "poolSizes.h"

#include <util/Pool.h>

#include <string.h>
#include <stdlib.h>

UTIL_POOL_DECLARE( ioInetSocketAddress )

utilStatus ioInetSocketAddress_init( ioInetSocketAddress * This, const char * host, unsigned short port ) {
   strncpy( This->inetAddress, host, sizeof( This->inetAddress ));
   This->inetAddress[sizeof(This->inetAddress)-1] = '\0';
   This->port = port;
   return UTIL_STATUS_NO_ERROR;
}

utilStatus ioInetSocketAddress_new(
   ioInetSocketAddress ** self,
   const char *           host,
   unsigned short         port )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   ioInetSocketAddress * This = NULL;
   UTIL_ALLOCATE( ioInetSocketAddress );
   if( UTIL_STATUS_NO_ERROR == status ) {
      ioInetSocketAddress_init( This, host, port );
   }
   return status;
}

utilStatus ioInetSocketAddress_delete( ioInetSocketAddress ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   UTIL_RELEASE( ioInetSocketAddress );
   return status;
}

int ioInetSocketAddress_comparator( const ioInetSocketAddress * left, const ioInetSocketAddress * right ) {
   int diff = strcmp( left->inetAddress, right->inetAddress );
   if( diff == 0 ) {
      diff = left->port - right->port;
   }
   return diff;
}
