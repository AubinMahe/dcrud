#include <dcrud/RepositoryFactoryBuilder.h>
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

static collMap factories;

dcrudIRepositoryFactory dcrudIRepositoryFactory_Repositories(
   const char *   address,
   const char *   intrfc,
   unsigned short port,
   unsigned int   id );

dcrudIRepositoryFactory dcrudRepositoryFactoryBuilder_join(
   const char *   address,
   const char *   intrfc,
   unsigned short port,
   unsigned int   id      )
{
   unsigned int mcastAddr;
   unsigned int * key;
   dcrudIRepositoryFactory factory;
   if( !factories ) {
      factories = collMap_reserve((collComparator)addressComparator );
   }
   mcastAddr = (unsigned int)inet_addr( address );
   factory = collMap_get( factories, &mcastAddr );
   if( factory ) {
      return factory;
   }
   factory = dcrudIRepositoryFactory_Repositories( address, intrfc, port, id );
   key = (unsigned int *)malloc( sizeof( unsigned int ));
   *key = mcastAddr;
   collMap_put( factories, key, factory );
   return factory;
}
