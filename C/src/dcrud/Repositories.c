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
#endif

static byte     SIGNATURE[]     = { 'D','I','S','T','C','R','U','D' };
//static unsigned FRM_HEADER_SIZE = sizeof( SIGNATURE ) + 4; // SIGNATURE, Count
static unsigned MSG_HEADER_SIZE = 4 + 4 + 4 + 4;           // Size, Source, Instance, Class

typedef struct Repositories_s {

   SOCKET             sckt;
   unsigned short     port;
   struct sockaddr_in target;
   collMap            repositories;
   ioByteBuffer       header;
   ioByteBuffer       payload;
   ioByteBuffer       frame;
   unsigned           messageCount;

} Repositories;

static int sourceComparator( int * left, int * right ) {
   return *left - *right;
}

dcrudIRepositoryFactory * dcrudIRepositoryFactory_Repositories(
   unsigned long  ipV4MCastAddress,
   unsigned long  ipV4Intrfc,
   unsigned short port    )
{
   Repositories * This = (Repositories*)malloc( sizeof( Repositories ));
   This->sckt = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
   check( This->sckt != INVALID_SOCKET, __FILE__, __LINE__, "socket" );
   int trueValue = 1;
   check( 0 ==
      setsockopt( This->sckt, SOL_SOCKET, SO_REUSEADDR, (char*)&trueValue, sizeof( trueValue )),
      __FILE__, __LINE__, "setsockopt(SO_REUSEADDR)" );
   struct sockaddr_in bindAddr;
   memset( &bindAddr, 0, sizeof( bindAddr ));
   memset( &This->target, 0, sizeof( This->target ) );
   bindAddr.sin_family = AF_INET;
   bindAddr.sin_addr.s_addr = INADDR_ANY;
   bindAddr.sin_port = htons( port );
   This->target.sin_family = AF_INET;
   This->target.sin_addr.s_addr = ipV4MCastAddress;
   This->target.sin_port = bindAddr.sin_port;
   check( 0 ==
      bind( This->sckt, (SOCKADDR *)&bindAddr, sizeof( bindAddr )),
      __FILE__, __LINE__, "bind(%s,%d)", inet_ntoa( bindAddr.sin_addr ), port );
   struct ip_mreq_source imr;
   memset( &imr, 0, sizeof( imr ));
   imr.imr_multiaddr.s_addr = ipV4MCastAddress;
   imr.imr_sourceaddr.s_addr = ipV4Intrfc;
   imr.imr_interface.s_addr = ipV4Intrfc;
   check( 0 ==
      setsockopt( This->sckt, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (char *)&imr, sizeof( imr )),
      __FILE__, __LINE__, "setsockopt(IP_ADD_SOURCE_MEMBERSHIP)" );
   This->port         = port;
   This->repositories = collMap_reserve((collComparator)sourceComparator );
   dcrudByteBuffer_init( &This->header, MSG_HEADER_SIZE );
   dcrudByteBuffer_init( &This->payload, 10*1024 );
   dcrudByteBuffer_init( &This->frame, 500*1024 );
   return (dcrudIRepositoryFactory *)This;
}

dcrudIRepository * dcrudIRepositoryFactory_getRepository(
   dcrudIRepositoryFactory * self,
   int                       sourceId,
   int                       producer,
   dcrudShareableFactory     factory  )
{
   Repositories * This = (Repositories *)self;
   dcrudIRepository * repo = dcrudCache_init( sourceId, producer, factory, self );
   collList repositories = collMap_get( This->repositories, (void*)sourceId );
   if( repositories == NULL ) {
      repositories = collList_reserve( repositories );
      collMap_put( This->repositories, (void*)sourceId, repositories );
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
   unsigned r = This->frame.position;
   This->frame.position = sizeof( SIGNATURE );
   dcrudByteBuffer_putInt( &This->frame, This->messageCount );
   This->frame.position = r;
   dcrudByteBuffer_flip( &This->frame );
   sendto(
      This->sckt,
      (const char *)This->frame.bytes,
      (int)This->frame.position,
      0,
      (struct sockaddr *)&This->target,
      sizeof( This->target ));
}

static void pushCreateOrUpdateItem( Repositories * This, dcrudShareable * item ) {
   dcrudByteBuffer_clear( &This->payload );
   item->serialize( item, &This->payload );
   dcrudByteBuffer_flip( &This->payload );
   unsigned int size = dcrudByteBuffer_remaining( &This->payload );
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

typedef struct {
   
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
   pushCreateOrUpdateItemForeachCtx ctx = { false, This };
   collSet_foreach( updated, pushCreateOrUpdateItemForeach, &ctx );
   collSet_foreach( deleted, pushDeleteItemForeach        , &ctx );
   if( ctx.initDone ) {
      sendFrame( This );
   }
}
