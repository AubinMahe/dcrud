#include <dcrud/IRepositoryFactory.h>
#include <util/types.h>
#include <util/check.h>
#include <coll/Map.h>
#include <coll/List.h>
#include <coll/Set.h>
#include <io/ByteBuffer.h>
#include "Cache.h"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <Ws2tcpip.h>
#  include <mswsock.h>
#elif __linux__
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netinet/ip.h>
#  include <arpa/inet.h>
#  include <string.h>
#  define SOCKET int
#  define INVALID_SOCKET (-1)
#endif
#include <stdio.h>

static byte     SIGNATURE[]     = { 'D','I','S','T','C','R','U','D' };
/*static unsigned FRM_HEADER_SIZE = sizeof( SIGNATURE ) + 4; / * SIGNATURE, Count */
static unsigned MSG_HEADER_SIZE = 4 + 4 + 4 + 4;           /* Size, Source, Instance, Class */

typedef struct Repositories_s {

   SOCKET             sckt;
   struct sockaddr_in target;
   collMap            repositories;
   ioByteBuffer       header;
   ioByteBuffer       payload;
   ioByteBuffer       frame;
   unsigned           messageCount;

} Repositories;

static int sourceComparator( int * * left, int * * right ) {
   return **left - **right;
}

#ifdef _WIN32
static void exitHook( void ) {
   WSACleanup();
}
#endif

#define RECV_IP_ADDR      "224.0.0.3"

dcrudIRepositoryFactory * dcrudIRepositoryFactory_Repositories(
   const char *   address,
   const char *   intrfc,
   unsigned short port    )
{
   int trueValue = 1;
   struct sockaddr_in local_sin;
   struct ip_mreq mreq;
#ifdef _WIN32
   WSADATA wsaData;
   check( 0 == WSAStartup( MAKEWORD( 2, 2 ), &wsaData ), __FILE__, __LINE__, "WSAStartup" );
   atexit( exitHook );
#endif
   Repositories * This = (Repositories*)malloc( sizeof( Repositories ) );
   This->sckt = socket( AF_INET, SOCK_DGRAM, 0 );
   check( This->sckt != INVALID_SOCKET, __FILE__, __LINE__, "socket" );
   check( 0 ==
      setsockopt( This->sckt, SOL_SOCKET, SO_REUSEADDR, (char*)&trueValue, sizeof( trueValue ) ),
      __FILE__, __LINE__, "setsockopt(SO_REUSEADDR)" );
   memset( &local_sin, 0, sizeof( local_sin ) );
   local_sin.sin_family = AF_INET;
   local_sin.sin_port = htons( port );
   local_sin.sin_addr.s_addr = htonl( INADDR_ANY );
   check( 0 == bind( This->sckt, (struct sockaddr *)&local_sin, sizeof( local_sin )),
      __FILE__, __LINE__, "bind(%s,%d)", address, 0 );
   memset( &mreq, 0, sizeof( mreq ));
   mreq.imr_multiaddr.s_addr = inet_addr( RECV_IP_ADDR );
   mreq.imr_interface.s_addr = INADDR_ANY;
   check( 0 == setsockopt( This->sckt, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof( mreq )),
      __FILE__, __LINE__, "setsockopt(IP_ADD_MEMBERSHIP,%s)", address );
   memset( &This->target, 0, sizeof( This->target ) );
   This->target.sin_family = AF_INET;
   This->target.sin_port = htons( port );
   This->target.sin_addr.s_addr = inet_addr( address );
   This->repositories = collMap_reserve((collComparator)sourceComparator );
   dcrudByteBuffer_init( &This->header, MSG_HEADER_SIZE );
   dcrudByteBuffer_init( &This->payload, 10*1024 );
   dcrudByteBuffer_init( &This->frame, 500*1024 );
   return (dcrudIRepositoryFactory *)This;
   (void)intrfc;
}

dcrudIRepository * dcrudIRepositoryFactory_getRepository(
   dcrudIRepositoryFactory * self,
   int                       sourceId,
   int                       producer,
   dcrudShareableFactory     factory  )
{
   Repositories * This = (Repositories *)self;
   dcrudIRepository * repo = dcrudCache_init( sourceId, producer, factory, self );
   collList repositories = collMap_get( This->repositories, &sourceId );
   if( repositories == NULL ) {
      int * key = (int *)malloc( sizeof( int ));
      repositories = collList_reserve( repositories );
      *key = sourceId;
      collMap_put( This->repositories, key, repositories );
   }
   collList_add( repositories, repo );
   return repo;
}

static void initFrame( Repositories * This ) {
   This->messageCount = 0;
   dcrudByteBuffer_clear( &This->frame );
   dcrudByteBuffer_put( &This->frame, SIGNATURE, sizeof( SIGNATURE ));
   dcrudByteBuffer_putInt( &This->frame, This->messageCount );
}

static void sendFrame( Repositories * This ) {
	size_t countBytes = (size_t)This->frame.position;
   This->frame.position = sizeof( SIGNATURE );
   dcrudByteBuffer_putInt( &This->frame, This->messageCount );
   check( countBytes == (size_t)
      sendto(
         This->sckt,
         (const char *)This->frame.bytes,
         countBytes,
         0,
         (struct sockaddr *)&This->target,
         sizeof( This->target )),
      __FILE__, __LINE__, "sendTo %d bytes sent to %s:%d",
      countBytes, inet_ntoa( This->target.sin_addr ), ntohs( This->target.sin_port ));
   printf( "%lu bytes sent to %s:%d\n",
      countBytes, inet_ntoa( This->target.sin_addr ), ntohs( This->target.sin_port ));
}

static void pushCreateOrUpdateItem( Repositories * This, dcrudShareable * item ) {
   unsigned int size;
   dcrudByteBuffer_clear( &This->payload );
   item->serialize( item, &This->payload );
   dcrudByteBuffer_flip( &This->payload );
   size = dcrudByteBuffer_remaining( &This->payload );
   dcrudByteBuffer_clear( &This->header );
   dcrudByteBuffer_putInt( &This->header, size );
   dcrudByteBuffer_putInt( &This->header, (unsigned)item->id.source );
   dcrudByteBuffer_putInt( &This->header, (unsigned)item->id.instance );
   dcrudByteBuffer_putInt( &This->header, (unsigned)item->classId );
   dcrudByteBuffer_flip( &This->header );
   if( dcrudByteBuffer_remaining( &This->frame ) < MSG_HEADER_SIZE + size ) {
      sendFrame( This );
      initFrame( This );
   }
   dcrudByteBuffer_putByteBuffer( &This->frame, &This->header );
   dcrudByteBuffer_putByteBuffer( &This->frame, &This->payload );
   ++This->messageCount;
}

static void pushDeleteItem( Repositories * This, dcrudShareable * item ) {
   dcrudByteBuffer_clear( &This->header );
   dcrudByteBuffer_putInt( &This->header, 0 );
   dcrudByteBuffer_putInt( &This->header, (unsigned)item->id.source );
   dcrudByteBuffer_putInt( &This->header, (unsigned)item->id.instance );
   dcrudByteBuffer_putInt( &This->header, (unsigned)item->classId );
   dcrudByteBuffer_flip( &This->header );
   if( dcrudByteBuffer_remaining( &This->frame ) < MSG_HEADER_SIZE ) {
      sendFrame( This );
      initFrame( This );
   }
   dcrudByteBuffer_putByteBuffer( &This->frame, &This->header );
   ++This->messageCount;
}

typedef struct pushCreateOrUpdateItemForeachCtx_s {
   
   bool           initDone;
   Repositories * repositories;

} pushCreateOrUpdateItemForeachCtx;

static bool pushCreateOrUpdateItemForeach( collForeach * context ) {
   dcrudShareable * item = (dcrudShareable *)context->item;
   pushCreateOrUpdateItemForeachCtx * ctx = (pushCreateOrUpdateItemForeachCtx *)context->user;
   if( !ctx->initDone ) {
      ctx->initDone = true;
      initFrame( ctx->repositories );
   }
   pushCreateOrUpdateItem( ctx->repositories, item );
   return true;
}

static bool pushDeleteItemForeach( collForeach * context ) {
   dcrudShareable * item = (dcrudShareable *)context->item;
   pushCreateOrUpdateItemForeachCtx * ctx = (pushCreateOrUpdateItemForeachCtx *)context->user;
   if( !ctx->initDone ) {
      ctx->initDone = true;
      initFrame( ctx->repositories );
   }
   pushDeleteItem( ctx->repositories, item );
   return true;
}

void Repositories_publish(
   dcrudIRepositoryFactory * self,
   collSet                   updated,
   collSet                   deleted )
{
   Repositories * This = (Repositories *)self;
   pushCreateOrUpdateItemForeachCtx ctx;
   ctx.initDone     = false;
   ctx.repositories = This;
   collSet_foreach( updated, pushCreateOrUpdateItemForeach, &ctx );
   collSet_foreach( deleted, pushDeleteItemForeach        , &ctx );
   if( ctx.initDone ) {
      sendFrame( This );
   }
}
