#include <dcrud/Networks.h>
#include "Network.h"
#include <stdio.h>
#include <sys/types.h>
#include <setjmp.h>
#ifdef __linux__
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#elif _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <Ws2tcpip.h>
#  include <mswsock.h>
#endif

static int addressComparator( const unsigned int * left, const unsigned int * right ) {
   return (int)(*left - *right);
}

static collMap groups;

dcrudIParticipant dcrudNetworks_join(
   const char *   address,
   const char *   intrfc,
   unsigned short port,
   byte           platformId,
   byte           execId     )
{
   unsigned int      mcastAddr;
   unsigned int *    key;
   dcrudIParticipant network;
   if( ! groups ) {
      groups = collMap_new((collComparator)addressComparator );
   }
   mcastAddr = (unsigned int)inet_addr( address );
   network   = collMap_get( groups, &mcastAddr );
   if( network ) {
      return network;
   }
   network = Network_new( address, intrfc, port, platformId, execId );
   key     = (unsigned int *)malloc( sizeof( unsigned int ));
   *key    = mcastAddr;
   collMap_put( groups, key, network, NULL );
   return network;
}

void dcrudNetworks_leave( dcrudIParticipant participant ) {
   unsigned int       key     = Network_getMCastAddress( participant );
   collMapPair previousPair;

   if( collMap_remove( groups, &key, &previousPair )) {
      free( previousPair.key );
      Network_delete( previousPair.value );
   }
}
