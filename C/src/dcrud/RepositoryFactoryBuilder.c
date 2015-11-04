#include <dcrud/RepositoryFactoryBuilder.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef LINUX
#  include <sys/socket.h>
#  include <netdb.h>
#elif _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <Ws2tcpip.h>
#  include <mswsock.h>
#endif

typedef struct FactoryEntry_s {

   unsigned long             address;
   dcrudIRepositoryFactory * factory;

} FactoryEntry;

static int addressComparator( const unsigned long * left, const unsigned long * right ) {
   return (int)(*left - *right);
}

static collMap factories;

dcrudIRepositoryFactory * dcrudIRepositoryFactory_Repositories(
   unsigned long  ipV4MCastAddress,
   unsigned long  ipV4Intrfc,
   unsigned short port        );

dcrudIRepositoryFactory * dcrudRepositoryFactoryBuilder_join(
   const char *   address,
   const char *   intrfc,
   unsigned short port    )
{
   if( !factories ) {
      factories = collMap_reserve((collComparator)addressComparator );
   }
   unsigned long mcastAddr  = inet_addr( address );
   unsigned long intrfcAddr = inet_addr( intrfc );
   FactoryEntry * entry = collMap_get( factories, (collMapKey)mcastAddr );
   if( entry ) {
      return entry->factory;
   }
   dcrudIRepositoryFactory * factory =
      dcrudIRepositoryFactory_Repositories( mcastAddr, intrfcAddr, port );
   collMap_put( factories, (collMapKey)mcastAddr, factory );
   return factory;
}
