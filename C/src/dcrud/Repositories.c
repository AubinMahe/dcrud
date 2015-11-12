#include <dcrud/IRepositoryFactory.h>
#include <util/types.h>
#include <util/check.h>
#include <coll/Map.h>
#include <coll/List.h>
#include <coll/Set.h>
#include <io/ByteBuffer.h>
#include <io/socket.h>
#include "Cache.h"

#include <stdio.h>

static byte SIGNATURE[] = { 'D','I','S','T','C','R','U','D' };

typedef struct Repositories_s {

   SOCKET             sckt;
   struct sockaddr_in target;
   collMap            repositories;
   ioByteBuffer       header;
   ioByteBuffer       payload;
   ioByteBuffer       frame;
   unsigned           id;
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

dcrudIRepositoryFactory * dcrudIRepositoryFactory_Repositories(
   const char *   address,
   const char *   intrfc,
   unsigned short port,
   unsigned int   id    )
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
   local_sin.sin_family      = AF_INET;
   local_sin.sin_port        = htons( port );
   local_sin.sin_addr.s_addr = inet_addr( intrfc );
   check( 0 == bind( This->sckt, (struct sockaddr *)&local_sin, sizeof( local_sin )),
      __FILE__, __LINE__, "bind(%s,%d)", intrfc, port );
   memset( &mreq, 0, sizeof( mreq ));
   mreq.imr_multiaddr.s_addr = inet_addr( address );
   mreq.imr_interface.s_addr = inet_addr( intrfc );
   check( 0 == setsockopt( This->sckt, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof( mreq )),
      __FILE__, __LINE__, "setsockopt(IP_ADD_MEMBERSHIP,%s)", address );
   memset( &This->target, 0, sizeof( This->target ) );
   This->target.sin_family = AF_INET;
   This->target.sin_port = htons( port );
   This->target.sin_addr.s_addr = inet_addr( address );
   This->repositories = collMap_reserve((collComparator)sourceComparator );
   This->header       = ioByteBuffer_allocate(   2*1024 );
   This->payload      = ioByteBuffer_allocate(  10*1024 );
   This->frame        = ioByteBuffer_allocate( 500*1024 );
   This->id           = id;
   return (dcrudIRepositoryFactory *)This;
   (void)intrfc;
}

dcrudIRepository dcrudIRepositoryFactory_getRepository(
   dcrudIRepositoryFactory self,
   const char *            topic,
   dcrudShareableFactory   factory )
{
   Repositories * This = (Repositories *)self;
   dcrudIRepository repo = dcrudCache_init( topic, factory, self );
   collList repositories = collMap_get( This->repositories, topic );
   if( repositories == NULL ) {
      repositories = collList_reserve( repositories );
      collMap_put( This->repositories, topic, repositories );
   }
   collList_add( repositories, repo );
   return repo;
}

static void initFrame( Repositories * This ) {
   This->messageCount = 0;
   ioByteBuffer_clear ( This->frame );
   ioByteBuffer_put   ( This->frame, SIGNATURE, 0, sizeof( SIGNATURE ));
   ioByteBuffer_putInt( This->frame, This->id );
   ioByteBuffer_putInt( This->frame, This->messageCount );
}

static void sendFrame( Repositories * This ) {
   int countBytes = 0U;
	ioByteBuffer_putIntAt( This->frame, This->messageCount, sizeof( SIGNATURE ) + sizeof( int ));
   ioByteBuffer_flip( This->frame );
   ioByteBuffer_sendTo( This->frame, This->sckt, &This->target, &countBytes );
   printf( "%d bytes sent to %s:%d\n",
      countBytes, inet_ntoa( This->target.sin_addr ), ntohs( This->target.sin_port ));
}

static void sendFrameIfNeeded( Repositories * This ) {
   size_t frameR   = ioByteBuffer_remaining( This->frame   );
   size_t headerR  = ioByteBuffer_remaining( This->header  );
   size_t payloadR = ioByteBuffer_remaining( This->payload );
   if( frameR < ( headerR + payloadR )) {
      sendFrame( This );
      initFrame( This );
   }
}

static void pushCreateOrUpdateItem( Repositories * This, dcrudShareable item ) {
   size_t size;
   dcrudGUID id = dcrudShareable_getId( item );

   ioByteBuffer_clear ( This->payload );
   dcrudShareable_serialize( item, This->payload );
   ioByteBuffer_flip  ( This->payload );
   size = ioByteBuffer_remaining( This->payload );
   ioByteBuffer_clear ( This->header );
   ioByteBuffer_putInt( This->header, (unsigned int)size );
   dcrudGUID_serialize( id, This->header );
   ioByteBuffer_flip  ( This->header );
   sendFrameIfNeeded( This );
   ioByteBuffer_putBuffer( This->frame, This->header );
   ioByteBuffer_putBuffer( This->frame, This->payload );
   ++This->messageCount;
}

static void pushDeleteItem( Repositories * This, dcrudShareable item ) {
   dcrudGUID id = dcrudShareable_getId( item );

   ioByteBuffer_clear( This->header );
   ioByteBuffer_putInt( This->header, 0 );
   dcrudGUID_serialize( id, This->header );
   ioByteBuffer_flip( This->header );
   sendFrameIfNeeded( This );
   ioByteBuffer_putBuffer( This->frame, This->header );
   ++This->messageCount;
}

typedef struct pushCreateOrUpdateItemForeachCtx_s {

   bool           initDone;
   Repositories * repositories;

} pushCreateOrUpdateItemForeachCtx;

static bool pushCreateOrUpdateItemForeach( collForeach * context ) {
   dcrudShareable item = (dcrudShareable)context->item;
   pushCreateOrUpdateItemForeachCtx * ctx = (pushCreateOrUpdateItemForeachCtx *)context->user;
   if( !ctx->initDone ) {
      ctx->initDone = true;
      initFrame( ctx->repositories );
   }
   pushCreateOrUpdateItem( ctx->repositories, item );
   return true;
}

static bool pushDeleteItemForeach( collForeach * context ) {
   dcrudShareable item = (dcrudShareable)context->item;
   pushCreateOrUpdateItemForeachCtx * ctx = (pushCreateOrUpdateItemForeachCtx *)context->user;
   if( !ctx->initDone ) {
      ctx->initDone = true;
      initFrame( ctx->repositories );
   }
   pushDeleteItem( ctx->repositories, item );
   return true;
}

void Repositories_publish(
   dcrudIRepositoryFactory self,
   collSet                 updated,
   collSet                 deleted )
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
