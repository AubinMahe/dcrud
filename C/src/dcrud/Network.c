#include "Network.h"
#include "Cache.h"

#include <util/check.h>

#include <coll/List.h>
#include <coll/MapFuncPtr.h>

#include <io/ByteBuffer.h>
#include <io/socket.h>

#include <os/mutex.h>
#include <os/System.h>

#include <dbg/Performance.h>
#include <dbg/Dump.h>

#include <stdio.h>

static const byte SIGNATURE[] = { 'D','I','S','T','C','R','U','D' };

#define SIZE_SIZE     4U
#define GUID_SIZE     (1U + 1U + 1U + 4U)
#define CLASS_ID_SIZE (1U + 1U + 1U + 1U)
#define HEADER_SIZE   (SIZE_SIZE + GUID_SIZE + CLASS_ID_SIZE)
#define PAYLOAD_SIZE  ( 64U*1024U)
#define FRAME_SIZE    (640U*1024U)

static int nextCacheID = 0;

typedef struct Network_s {

   osMutex            cachesMutex;
   dcrudICache        caches[256];
   dcrudIDispatcher   dispatcher;
   ioByteBuffer       header;
   ioByteBuffer       payload;
   osMutex            frameMutex;
   ioByteBuffer       frame;
   osMutex            factoriesMutex;
   collMapFuncPtr     factories;
   collMapVoidPtr     callbacks;
   struct sockaddr_in target;
   SOCKET             channel;
   byte               platformId;
   byte               execId;
   unsigned           itemCount;
   unsigned           callId;

} Network;

#ifdef _WIN32
static void exitHook( void ) {
   WSACleanup();
}
#endif

dcrudIParticipant Network_Network(
   const char *   address,
   const char *   intrfc,
   unsigned short port,
   byte           platformId,
   byte           execId     )
{
   int trueValue = 1;
   struct sockaddr_in local_sin;
   struct ip_mreq mreq;
#ifdef _WIN32
   WSADATA wsaData;
   if( ! check( 0 == WSAStartup( MAKEWORD( 2, 2 ), &wsaData ), __FILE__, __LINE__, "WSAStartup" )) {
      return NULL;
   }
   atexit( exitHook );
#endif
   Network * This = (Network *)malloc( sizeof( Network ));
   memset( This      , 0, sizeof( Network ));
   memset( &local_sin, 0, sizeof( local_sin ));
   memset( &mreq     , 0, sizeof( mreq ));
   This->header                 = ioByteBuffer_new( HEADER_SIZE );
   This->payload                = ioByteBuffer_new( PAYLOAD_SIZE );
   This->frame                  = ioByteBuffer_new( FRAME_SIZE );
   This->platformId             = platformId;
   This->execId                 = execId;
   This->target.sin_family      = AF_INET;
   This->target.sin_port        = htons( port );
   This->target.sin_addr.s_addr = inet_addr( address );
   This->channel                = socket( AF_INET, SOCK_DGRAM, 0 );
   check( This->channel != INVALID_SOCKET, __FILE__, __LINE__, "socket" );
   if( ! check( 0 ==
      setsockopt( This->channel, SOL_SOCKET, SO_REUSEADDR, (char*)&trueValue, sizeof( trueValue )),
      __FILE__, __LINE__, "setsockopt(SO_REUSEADDR)" ))
   {
      ioByteBuffer_delete( &This->header  );
      ioByteBuffer_delete( &This->payload );
      ioByteBuffer_delete( &This->frame   );
      return NULL;
   }
   local_sin.sin_family      = AF_INET;
   local_sin.sin_port        = htons( port );
   local_sin.sin_addr.s_addr = inet_addr( intrfc );
   if( ! check( 0 == bind( This->channel, (struct sockaddr *)&local_sin, sizeof( local_sin )),
      __FILE__, __LINE__, "bind(%s,%d)", intrfc, port ))
   {
      ioByteBuffer_delete( &This->header  );
      ioByteBuffer_delete( &This->payload );
      ioByteBuffer_delete( &This->frame   );
      return NULL;
   }
   mreq.imr_multiaddr.s_addr = inet_addr( address );
   mreq.imr_interface.s_addr = inet_addr( intrfc );
   if( ! check( 0 == setsockopt( This->channel, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof( mreq )),
      __FILE__, __LINE__, "setsockopt(IP_ADD_MEMBERSHIP,%s)", address ))
   {
      ioByteBuffer_delete( &This->header  );
      ioByteBuffer_delete( &This->payload );
      ioByteBuffer_delete( &This->frame   );
      return NULL;
   }
   osMutex_new( &This->factoriesMutex );
   osMutex_new( &This->cachesMutex );
   return (dcrudIParticipant)This;
}

void Network_delete( dcrudIParticipant * This ) {
   free( This );
   *This = NULL;
}

unsigned int Network_getMCastAddress( dcrudIParticipant self ) {
   Network * This = (Network *)self;
   return This->target.sin_addr.s_addr;
}

dcrudShareable Network_newInstance(
   dcrudIParticipant self,
   dcrudClassID *    classId,
   ioByteBuffer      frame   )
{
   Network *             This    = (Network *)self;
   dcrudShareable        item    = NULL;
   dcrudShareableFactory factory = NULL;
   osMutex_take( This->factoriesMutex );
   factory = (dcrudShareableFactory)collMapFuncPtr_get( This->factories, classId );
   if( factory ) {
      item = factory();
      if( item ) {
         if( IO_STATUS_NO_ERROR != dcrudShareable_serialize( item, frame )) {
#ifndef STATIC_ALLOCATION
            free( item );
#endif
            item = NULL;
         }
      }
   }
   osMutex_release( This->factoriesMutex );
   return item;
}

bool dcrudIParticipant_registerClass(
   dcrudIParticipant     self,
   dcrudClassID          id,
   dcrudShareableFactory factory )
{
   Network * This = (Network *)self;
   bool      known;

   osMutex_take( This->factoriesMutex );
   known = collMapFuncPtr_put( This->factories, id, (collMapFuncPtrValue)factory, NULL );
   osMutex_release( This->factoriesMutex );
   return known;
}

dcrudStatus dcrudIParticipant_createCache( dcrudIParticipant self, dcrudICache * target ) {
   Network *   This   = (Network *)self;
   dcrudStatus status = DCRUD_NO_ERROR;

   osMutex_take( This->cachesMutex );
   if( nextCacheID > 255 ) {
      *target = NULL;
      status  = DCRUD_TOO_MANY_CACHES;
   }
   else {
      This->caches[nextCacheID++] = dcrudCache_init( self, This->platformId, This->execId );
   }
   osMutex_release( This->factoriesMutex );
   return status;
}

dcrudICache dcrudIParticipant_getCache( dcrudIParticipant self, byte ID ) {
   Network *   This  = (Network *)self;
   dcrudICache cache = NULL;

   osMutex_take( This->cachesMutex );
   cache = This->caches[ID];
   osMutex_release( This->factoriesMutex );
   return cache;
}

dcrudIDispatcher dcrudIParticipant_getDispatcher( dcrudIParticipant self ) {
   Network * This  = (Network *)self;
   return This->dispatcher;
}

static void initFrame( Network * This, byte cacheId ) {
   This->itemCount = 0;
   ioByteBuffer_clear  ( This->frame );
   ioByteBuffer_put    ( This->frame, SIGNATURE, 0, sizeof( SIGNATURE ));
   ioByteBuffer_putByte( This->frame, This->platformId );
   ioByteBuffer_putByte( This->frame, This->execId );
   ioByteBuffer_putByte( This->frame, cacheId );/* cache 0 doesn't exists, it's a flag for operation */
   ioByteBuffer_mark   ( This->frame );
   ioByteBuffer_putInt ( This->frame, This->itemCount );
}

#ifdef LOW_LEVEL_IO_TRACE
static void LOW_LEVEL_IO_PRINT_SENT( ioByteBuffer frame, struct sockaddr_in target ) {
   ioByteBuffer_flip( frame );
   printf( "%d bytes sent to %s:%d\n",
      ioByteBuffer_remaining( frame ),
      inet_ntoa( target->sin_addr ),
      ntohs( target->sin_port ));
}
#else
#  define LOW_LEVEL_IO_PRINT_SENT(f,t)
#endif

      static void sendFrame( Network * This ) {
   if( This->itemCount > 0 ) {
      size_t position  = ioByteBuffer_getPosition( This->frame );
      ioByteBuffer_reset ( This->frame );
      ioByteBuffer_putInt( This->frame, This->itemCount );
      ioByteBuffer_setPosition( This->frame, position );
      ioByteBuffer_flip( This->frame );
      ioByteBuffer_send( This->frame, This->channel, &This->target );
      LOW_LEVEL_IO_PRINT_SENT( This->frame, This->target );
   }
   ioByteBuffer_clear( This->frame );
}

static void sendFrameIfNeeded( Network * This, byte cacheId ) {
   size_t frameR   = ioByteBuffer_remaining( This->frame   );
   size_t headerR  = ioByteBuffer_remaining( This->header  );
   size_t payloadR = ioByteBuffer_remaining( This->payload );
   if( frameR < ( headerR + payloadR )) {
      sendFrame( This );
      initFrame( This, cacheId );
   }
}

static void pushCreateOrUpdateItem( Network * This, byte cacheId, dcrudShareable item ) {
   size_t size;
   ioByteBuffer_clear   ( This->payload );
   dcrudShareable_serialize( item, This->payload );
   ioByteBuffer_flip     ( This->payload );
   size = ioByteBuffer_remaining( This->payload );
   ioByteBuffer_clear    ( This->header );
   ioByteBuffer_putInt   ( This->header, (unsigned int)size );
   dcrudGUID_serialize   ( dcrudShareable_getGUID   ( item ), This->header );
   dcrudClassID_serialize( dcrudShareable_getClassID( item ), This->header );
   ioByteBuffer_flip     ( This->header );
   sendFrameIfNeeded( This, cacheId );
   ioByteBuffer_putBuffer( This->frame, This->header );
   ioByteBuffer_putBuffer( This->frame, This->payload );
   ++This->itemCount;
}

static void pushDeleteItem( Network * This, byte cacheId, dcrudShareable item ) {
   ioByteBuffer_clear( This->header );
   ioByteBuffer_putInt( This->header, 0 );
   dcrudGUID_serialize( dcrudShareable_getGUID( item ), This->header );
   ioByteBuffer_flip( This->header );
   sendFrameIfNeeded( This, cacheId );
   ioByteBuffer_putBuffer( This->frame, This->header );
   ++This->itemCount;
}

typedef struct pushCreateOrUpdateItemForeachCtx_s {

   byte      cacheId;
   Network * network;

} pushCreateOrUpdateItemForeachCtx;

static bool pushCreateOrUpdateItemForeach( collForeach * context ) {
   dcrudShareable                     item = (dcrudShareable                    )context->item;
   pushCreateOrUpdateItemForeachCtx * ctx  = (pushCreateOrUpdateItemForeachCtx *)context->user;
   pushCreateOrUpdateItem( ctx->network, ctx->cacheId, item );
   return true;
}

static bool pushDeleteItemForeach( collForeach * context ) {
   dcrudShareable                     item = (dcrudShareable                    )context->item;
   pushCreateOrUpdateItemForeachCtx * ctx  = (pushCreateOrUpdateItemForeachCtx *)context->user;
   pushDeleteItem( ctx->network, ctx->cacheId, item );
   return true;
}

void Network_publish( dcrudIParticipant self, byte cacheId, collSet updated, collSet deleted ) {
   Network * This = (Network *)self;
   pushCreateOrUpdateItemForeachCtx ctx;

   ctx.network = This;
   ctx.cacheId = cacheId;

   osMutex_take( This->frameMutex );
   initFrame( This, cacheId );
   collSet_foreach( updated, pushCreateOrUpdateItemForeach, &ctx );
   collSet_foreach( deleted, pushDeleteItemForeach        , &ctx );
   sendFrame( This );
   osMutex_release( This->frameMutex );
}

static bool serializeArgument( collMapVoidPtrForeach * context ) {
   const char * name    = (const char *)context->key;
   Network *    network = (Network *   )context->user;
   ioByteBuffer_putString( network->frame, name );
   if( 0 == strcmp( name, "@queue" )) {
      byte * value = (byte *)context->value;
      ioByteBuffer_putByte( network->frame, *value );
   }
   else if( 0 == strcmp( name, "@mode" )) {
      byte * value = (byte *)context->value;
      ioByteBuffer_putByte( network->frame, *value );
   }
   else {
      dcrudShareable item = (dcrudShareable)context->value;
      dcrudClassID_serialize( dcrudShareable_getClassID( item ), network->frame );
      dcrudShareable_serialize( item, network->frame );
   }
   return true;
}

static void sendCall(
   Network *      This,
   const char *   intrfcName,
   const char *   opName,
   collMapVoidPtr in,
   unsigned       callId )
{
   osMutex_take( This->frameMutex );
   ioByteBuffer_clear    ( This->frame );
   ioByteBuffer_put      ( This->frame, SIGNATURE, 0, sizeof( SIGNATURE ));
   ioByteBuffer_putByte  ( This->frame, This->platformId );
   ioByteBuffer_putByte  ( This->frame, This->execId );
   ioByteBuffer_putByte  ( This->frame, 0 ); /* cache 0 doesn't exists, it's a flag for operation */
   ioByteBuffer_putInt   ( This->frame, collMapVoidPtr_size( in ));
   ioByteBuffer_putString( This->frame, intrfcName );
   ioByteBuffer_putString( This->frame, opName );
   ioByteBuffer_putInt   ( This->frame, callId );
   collMapVoidPtr_foreach( in, serializeArgument, This );
   ioByteBuffer_flip     ( This->frame );
   ioByteBuffer_send     ( This->frame, This->channel, &This->target );
   LOW_LEVEL_IO_PRINT_SENT( This->frame, This->target );
   osMutex_release( This->frameMutex );
}

unsigned int call(
   dcrudIParticipant self,
   const char *      intrfcName,
   const char *      opName,
   collMapVoidPtr    in,
   dcrudICallback    callback )
{
   Network * This = (Network *)self;
   sendCall( This, intrfcName, opName, in, This->callId );
   if( callback ) {
      collMapVoidPtr_put( This->callbacks, &This->callId, callback, NULL );
      return This->callId++;
   }
   return 0;
}

void dcrudIParticipant_run( dcrudIParticipant self ) {
   Network * This = (Network *)self;
   byte      signa[sizeof( SIGNATURE )];
   uint64_t  atStart;
   for( atStart = 0;;) {
      ioByteBuffer frame = ioByteBuffer_new( 64*1024 );
      if( atStart > 0 ) {
         Performance_record( "network", System_nanotime() - atStart );
      }
      ioByteBuffer_receive( frame, This->channel );
      atStart = System_nanotime();
      ioByteBuffer_flip( frame );
      dump( stderr, ioByteBuffer_getBytes( frame ), ioByteBuffer_getPosition( frame ));
      ioByteBuffer_get( frame, signa, 0, sizeof( signa ));
   }
}
