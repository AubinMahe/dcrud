#include "Network.h"
#include "Cache.h"
#include "Dispatcher.h"
#include "Shareable_private.h"

#include <util/check.h>

#include <coll/List.h>
#include <coll/MapFuncPtr.h>

#include <io/ByteBuffer.h>
#include <io/socket.h>

#include <os/System.h>

#include <dbg/Performance.h>
#include <dbg/Dump.h>
#include <os/Mutex.h>

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
   collMap            factories;
   collMap            callbacks;
   struct sockaddr_in target;
   SOCKET             channel;
   byte               platformId;
   byte               execId;
   unsigned int       itemCount;
   int                callId;

} Network;

#ifdef _WIN32
static void exitHook( void ) {
   WSACleanup();
}
#endif

dcrudIParticipant Network_new(
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
   This->dispatcher = dcrudDispatcher_new((dcrudIParticipant)This );
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

typedef struct Factory_s {

   size_t                     size;
   dcrudShareable_Initialize  initialize;
   dcrudShareable_Set         set;
   dcrudShareable_Serialize   serialize;
   dcrudShareable_Unserialize unserialize;

} Factory;

bool dcrudIParticipant_registerClass(
   dcrudIParticipant          self,
   dcrudClassID               id,
   size_t                     size,
   dcrudShareable_Initialize  initialize,
   dcrudShareable_Set         set,
   dcrudShareable_Serialize   serialize,
   dcrudShareable_Unserialize unserialize )
{
   Network *   This = (Network *)self;
   bool        known;
   Factory *   factory = (Factory *)malloc( sizeof( Factory ));
   collMapPair previous;

   factory->size        = size;
   factory->initialize  = initialize;
   factory->set         = set;
   factory->serialize   = serialize;
   factory->unserialize = unserialize;
   osMutex_take( This->factoriesMutex );
   known = collMap_put( This->factories, id, (collMapValue)factory, &previous );
   if( known ) {
      free( previous.value );
   }
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
      This->caches[nextCacheID++] = dcrudCache_new( self, This->platformId, This->execId );
   }
   osMutex_release( This->cachesMutex );
   return status;
}

dcrudICache dcrudIParticipant_getCache( dcrudIParticipant self, byte ID ) {
   Network *   This  = (Network *)self;
   dcrudICache cache = NULL;

   osMutex_take( This->cachesMutex );
   cache = This->caches[ID];
   osMutex_release( This->cachesMutex );
   return cache;
}

dcrudIDispatcher dcrudIParticipant_getDispatcher( dcrudIParticipant self ) {
   Network * This  = (Network *)self;
   return This->dispatcher;
}

dcrudShareable dcrudIParticipant_createShareable( dcrudIParticipant self, dcrudClassID classID ) {
   Network *            This    = (Network *)self;
   dcrudShareableImpl * item    = NULL;
   Factory *            factory = NULL;

   osMutex_take( This->factoriesMutex );
   factory = (Factory *)collMap_get( This->factories, classID );
   osMutex_release( This->factoriesMutex );
   if( factory ) {
      item = (dcrudShareableImpl *)malloc( sizeof( dcrudShareableImpl ) + factory->size );
      item->classID = classID;
      if( !factory->initialize((dcrudShareable)item )) {
         free( item );
         item = NULL;
      }
   }
   return (dcrudShareable)item;
}

dcrudShareable Network_newInstance( dcrudIParticipant self, ioByteBuffer frame ) {
   Network *            This    = (Network *)self;
   dcrudShareableImpl * item    = NULL;
   Factory *            factory = NULL;
   dcrudClassID         classID;

   dcrudClassID_unserialize( frame, &classID );
   osMutex_take( This->factoriesMutex );
   factory = (Factory *)collMap_get( This->factories, classID );
   osMutex_release( This->factoriesMutex );
   if( factory ) {
      item = (dcrudShareableImpl *)malloc( sizeof( dcrudShareableImpl ) + factory->size );
      item->classID     = classID;
      item->serialize   = factory->serialize;
      item->unserialize = factory->unserialize;
      if( IO_STATUS_NO_ERROR != factory->unserialize((dcrudShareable)item, frame )) {
         free( item );
         item = NULL;
      }
   }
   return (dcrudShareable)item;
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
   dcrudShareableImpl * concrete = (dcrudShareableImpl *)item;
   size_t size;
   ioByteBuffer_clear    ( This->payload );
   concrete->serialize( item, This->payload );
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

static bool serializeArgument( collForeach * context ) {
   Network *            network = (Network *           )context->user;
   collMapPair * pair    = (collMapPair *)context->item;
   const char *         name    = (const char *        )pair->key;
   ioByteBuffer_putString( network->frame, name );
   if( 0 == strcmp( name, "@queue" )) {
      byte * value = (byte *)pair->value;
      ioByteBuffer_putByte( network->frame, *value );
   }
   else if( 0 == strcmp( name, "@mode" )) {
      byte * value = (byte *)pair->value;
      ioByteBuffer_putByte( network->frame, *value );
   }
   else {
      dcrudShareableImpl * item = (dcrudShareableImpl *)pair->value;
      dcrudClassID_serialize( item->classID, network->frame );
      item->serialize((dcrudShareable)item, network->frame );
   }
   return true;
}

static void sendCall(
   Network *    This,
   const char * intrfcName,
   const char * opName,
   collMap      in,
   int          callId )
{
   osMutex_take( This->frameMutex );
   ioByteBuffer_clear    ( This->frame );
   ioByteBuffer_put      ( This->frame, SIGNATURE, 0, sizeof( SIGNATURE ));
   ioByteBuffer_putByte  ( This->frame, This->platformId );
   ioByteBuffer_putByte  ( This->frame, This->execId );
   ioByteBuffer_putByte  ( This->frame, 0 ); /* cache 0 doesn't exists, it's a flag for operation */
   ioByteBuffer_putInt   ( This->frame, collMap_size( in ));
   ioByteBuffer_putString( This->frame, intrfcName );
   ioByteBuffer_putString( This->frame, opName );
   ioByteBuffer_putInt   ( This->frame, (unsigned int)callId );
   collMap_foreach( in, serializeArgument, This );
   ioByteBuffer_flip     ( This->frame );
   ioByteBuffer_send     ( This->frame, This->channel, &This->target );
   LOW_LEVEL_IO_PRINT_SENT( This->frame, This->target );
   osMutex_release( This->frameMutex );
}

int Network_call(
   dcrudIParticipant self,
   const char *      intrfcName,
   const char *      opName,
   collMap           in,
   dcrudICallback    callback )
{
   Network * This = (Network *)self;
   sendCall( This, intrfcName, opName, in, This->callId );
   if( callback ) {
      collMap_put( This->callbacks, &This->callId, callback, NULL );
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
         dbgPerformance_record( "network", osSystem_nanotime() - atStart );
      }
      ioByteBuffer_receive( frame, This->channel );
      atStart = osSystem_nanotime();
      ioByteBuffer_flip( frame );
      dump( stderr, ioByteBuffer_getBytes( frame ), ioByteBuffer_getPosition( frame ));
      ioByteBuffer_get( frame, signa, 0, sizeof( signa ));
      if( 0 == strcmp((const char *)signa, (const char *)SIGNATURE )) {
         byte         platformId = 0;
         byte         execId     = 0;
         byte         cacheId    = 0;
         unsigned int count      = 0;
         collMap      arguments  = collMap_new((collComparator)strcmp );

         ioByteBuffer_getByte( frame, &platformId );
         ioByteBuffer_getByte( frame, &execId     );
         ioByteBuffer_getByte( frame, &cacheId    );
         ioByteBuffer_getInt ( frame, &count      );
         if( cacheId == 0 ) { /* cache 0 doesn't exists, it's a flag for operation */
            char          intrfcName[1000];
            char          opName[1000];
            int           callId;
            unsigned int  i;
            byte          queueNdx = DCRUD_DEFAULT_QUEUE;
            dcrudCallMode callMode = DCRUD_ASYNCHRONOUS_DEFERRED;

            ioByteBuffer_getString( frame, intrfcName, sizeof( intrfcName ));
            ioByteBuffer_getString( frame, opName    , sizeof( opName     ));
            ioByteBuffer_getInt   ( frame, (unsigned int *)&callId );
            for( i = 0; i < count; ++i ) {
               char argName[1000];
               ioByteBuffer_getString( frame, argName, sizeof( argName ));
               if( 0 == strcmp( argName, "@queue" )) {
                  ioByteBuffer_getByte( frame, &queueNdx );
               }
               else if( 0 == strcmp( argName, "@mode" )) {
                  ioByteBuffer_getByte( frame, (byte *)&callMode );
               }
               else {
                  dcrudShareable item = Network_newInstance( self, frame );
                  collMap_put( arguments, argName, item, NULL );
               }
            }
            if( callId > 0 ) {
               collMap out = collMap_new((collComparator)strcmp );
               dcrudIDispatcher_execute(
                  This->dispatcher, intrfcName, opName, arguments, out, queueNdx, callMode );
               if( collMap_size( out ) > 0 ) {
                  sendCall( This, intrfcName, opName, out, -callId );
               }
            }
            else if( callId < 0 ) {
               int            key      = -callId;
               dcrudICallback callback = collMap_get( This->callbacks, &key );
               if( callback == NULL ) {
                  fprintf( stderr, "Unknown Callback received: %s.%s, id: %d\n",
                     intrfcName, opName, -callId );
               }
               else {
                  dcrudICallback_callback( callback, intrfcName, opName, arguments );
               }
            }
         }
         else {
            unsigned int  i;
            for( i = 0; i < count; ++i ) {
               unsigned size;
               int      c;

               ioByteBuffer_getInt( frame, &size );
               if( size == 0 ) {
                  dcrudGUID id;
                  osMutex_take( This->cachesMutex );
                  dcrudGUID_unserialize( frame, &id );
                  for( c = 0; c < 256; ++c ) {
                     if( This->caches[c] ) {
                        dcrudCache_deleteFromNetwork( This->caches[c], &id );
                     }
                     else {
                        break;
                     }
                  }
                  osMutex_release( This->cachesMutex );
               }
               else {
                  osMutex_take( This->cachesMutex );
                  for( c = 0; c < 256; ++c ) {
                     if( This->caches[c] ) {
                        if( ! dcrudCache_matches((dcrudICache)This->caches[c], platformId, execId, cacheId )) {
                           dcrudCache_updateFromNetwork(
                              This->caches[c],
                              ioByteBuffer_copy( frame, GUID_SIZE + CLASS_ID_SIZE + size ));
                        }
                     }
                     else {
                        break;
                     }
                  }
                  osMutex_release( This->cachesMutex );
                  ioByteBuffer_setPosition( frame,
                     ioByteBuffer_getPosition( frame ) + GUID_SIZE + CLASS_ID_SIZE + size );
               }
            }
         }
         if( ioByteBuffer_remaining( frame )) {
            fprintf( stderr, "Frame decoding failed, %lu bytes remaining!\n",
               ioByteBuffer_remaining( frame ));
         }
      }
      else {
         fprintf( stderr, "Garbage received, %lu bytes discarded!\n",
            ioByteBuffer_getLimit( frame ));
      }

   }
}
