#include <io/InetSocketAddress.h>
#include <string.h>
#include <stdlib.h>

void ioInetSocketAddress_init( ioInetSocketAddress * This, const char * host, unsigned short port ) {
   strncpy( This->inetAddress, host, sizeof( This->inetAddress ));
   This->inetAddress[sizeof(This->inetAddress)-1] = '\0';
   This->port = port;
}

ioInetSocketAddress * ioInetSocketAddress_new( const char * host, unsigned short port ) {
   ioInetSocketAddress * This = (ioInetSocketAddress *)malloc( sizeof( ioInetSocketAddress ));
   ioInetSocketAddress_init( This, host, port );
   return This;
}

int ioInetSocketAddress_comparator( const ioInetSocketAddress * left, const ioInetSocketAddress * right ) {
   int diff = strcmp( left->inetAddress, right->inetAddress );
   if( diff == 0 ) {
      diff = left->port - right->port;
   }
   return diff;
}
