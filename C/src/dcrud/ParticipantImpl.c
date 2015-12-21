#include "ParticipantImpl.h"

#include "Cache.h"
#include "Dispatcher.h"
#include "Shareable_private.h"
#include "NetworkReceiver.h"

#include <coll/List.h>
#include <coll/MapFuncPtr.h>

#include <os/System.h>

#include <dbg/Performance.h>
#include <dbg/Dump.h>
#include <os/Mutex.h>

#include <stdio.h>
#include <util/CheckSysCall.h>

const byte DCRUD_SIGNATURE[DCRUD_SIGNATURE_SIZE] = { 'D','C','R','U', 'D' };

#ifdef _WIN32
static void exitHook( void ) {
   WSACleanup();
}
#else
#  include <unistd.h>
#endif

static int IntegerCompare( int * left, int * right ) {
   return *left - *right;
}

dcrudStatus ParticipantImpl_new(
   unsigned short      publisherId,
   const char *        address,
   unsigned short      port,
   const char *        intrfc,
   ParticipantImpl * * target  )
{
   int               trueValue = 1;
   struct in_addr    lIntrfc;
   ParticipantImpl * This;
#ifdef _WIN32
   WSADATA wsaData;
   if( ! utilCheckSysCall( 0 ==
      WSAStartup( MAKEWORD( 2, 2 ), &wsaData ),
      __FILE__, __LINE__, "WSAStartup" ))
   {
      *target = NULL;
      return DCRUD_INIT_FAILED;
   }
   atexit( exitHook );
#endif
   This = (ParticipantImpl *)malloc( sizeof( ParticipantImpl ));
   memset( This, 0, sizeof( ParticipantImpl ));
   *target = This;
   memset( &lIntrfc, 0, sizeof( lIntrfc ));
   if( osMutex_new( &This->cachesMutex )) {
      perror( "Unable to create mutex\n" );
      return DCRUD_INIT_FAILED;
   }
   This->header  = ioByteBuffer_new( HEADER_SIZE );
   This->payload = ioByteBuffer_new( PAYLOAD_SIZE );
   This->message = ioByteBuffer_new( 64*1024 );/* UDP MAX packet size */
   if( osMutex_new( &This->factoriesMutex )) {
      perror( "Unable to create mutex\n" );
      return DCRUD_INIT_FAILED;
   }
   if( osMutex_new( &This->publishersMutex )) {
      perror( "Unable to create mutex\n" );
      return DCRUD_INIT_FAILED;
   }
   if( osMutex_new( &This->outMutex )) {
      perror( "Unable to create mutex\n" );
      return DCRUD_INIT_FAILED;
   }
   This->factories              = collMap_new((collComparator)dcrudClassID_compareTo );
   This->publishers             = collMap_new((collComparator)dcrudClassID_compareTo );
   This->callbacks              = collMap_new((collComparator)IntegerCompare );
   This->publisherId            = publisherId;
   This->target.sin_family      = AF_INET;
   This->target.sin_port        = htons( port );
   This->target.sin_addr.s_addr = inet_addr( address );
   This->out                    = socket( AF_INET, SOCK_DGRAM, 0 );
   if( ! utilCheckSysCall( This->out != INVALID_SOCKET, __FILE__, __LINE__, "socket" )) {
      return DCRUD_INIT_FAILED;
   }
   if( ! utilCheckSysCall( 0 ==
      setsockopt( This->out, SOL_SOCKET, SO_REUSEADDR, (char*)&trueValue, sizeof( trueValue )),
      __FILE__, __LINE__, "setsockopt(SO_REUSEADDR)" ))
   {
      return DCRUD_INIT_FAILED;
   }
   lIntrfc.s_addr = inet_addr( intrfc );
   if( ! utilCheckSysCall( 0 ==
      setsockopt( This->out, IPPROTO_IP, IP_MULTICAST_IF, (char *)&lIntrfc, sizeof( lIntrfc )),
      __FILE__, __LINE__, "setsockopt(IP_MULTICAST_IF,%s)", intrfc ))
   {
      return DCRUD_INIT_FAILED;
   }
   printf( "Sending to %s:%d via interface %s\n", address, port, intrfc );
   This->dispatcher = dcrudIDispatcher_new( This );
   This->caches[This->nextCacheID++] = dcrudCache_new( This );
   return DCRUD_NO_ERROR;
}

static struct NetworkReceiver_s * s_receivers[65536];
static unsigned short             s_receiversCount;

void dcrudIParticipant_listen(
   dcrudIParticipant  self,
   const char *       intrfc,
   dcrudCounterpart * others[] )
{
   ParticipantImpl * This = (ParticipantImpl *)self;
   unsigned int      i;

   for( i = 0; others[i]; ++i ) {
      s_receivers[s_receiversCount++] =
         createNetworkReceiver( This, others[i]->mcastAddr, others[i]->port, intrfc );
   }
}

void dcrudIParticipant_delete( dcrudIParticipant * self ) {
   ParticipantImpl * This = (ParticipantImpl *)*self;
   if( This ) {
      unsigned int i;
      for( i = 0; ; ++i ) {
         if( s_receivers[i] ) {
            deleteNetworkReceiver( s_receivers[i] );
         }
         else {
            break;
         }
      }
      for( i = 0; i < This->nextCacheID; ++i ) {
         if( This->caches[i] ) {
            dcrudCache_delete( &(This->caches[i] ));
         }
         else {
            break;
         }
      }
      osMutex_delete     ( &This->cachesMutex    );
      ioByteBuffer_delete( &This->header         );
      ioByteBuffer_delete( &This->payload        );
      ioByteBuffer_delete( &This->message        );
      osMutex_delete     ( &This->factoriesMutex );
      collMap_delete     ( &This->factories      );
      collMap_delete     ( &This->callbacks      );
      osMutex_delete     ( &This->outMutex       );
#ifdef WIN32
      closesocket        (  This->out            );
#else
      close              (  This->out            );
#endif
      dcrudIDispatcher_delete( &This->dispatcher );
      free( This );
      *self = NULL;
   }
}

unsigned int ParticipantImpl_getMCastAddress( ParticipantImpl * This ) {
   return This->target.sin_addr.s_addr;
}

bool dcrudIParticipant_registerFactory( dcrudIParticipant self, dcrudIFactory * factory ) {
   ParticipantImpl * This = (ParticipantImpl *)self;
   collMapPair       previous;
   bool              known;

   osMutex_take( This->factoriesMutex );
   known = collMap_put( This->factories, factory->classID, (collMapValue)factory, &previous );
   if( known ) {
      free( previous.value );
   }
   osMutex_release( This->factoriesMutex );
   return known;
}

bool dcrudIParticipant_registerPublisher(
   dcrudIParticipant self,
   dcrudClassID      id,
   dcrudICRUDSrvc *  publisher )
{
   ParticipantImpl * This = (ParticipantImpl *)self;
   collMapPair       previous;
   bool              known;

   osMutex_take( This->publishersMutex );
   known = collMap_put( This->publishers, id, (collMapValue)publisher, &previous );
   if( known ) {
      free( previous.value );
   }
   osMutex_release( This->publishersMutex );
   return known;
}

dcrudICache dcrudIParticipant_getDefaultCache( dcrudIParticipant self ) {
   ParticipantImpl *   This  = (ParticipantImpl *)self;
   dcrudICache cache = NULL;

   osMutex_take( This->cachesMutex );
   cache = This->caches[0];
   osMutex_release( This->cachesMutex );
   return cache;
}

dcrudStatus dcrudIParticipant_createCache( dcrudIParticipant self, dcrudICache * target ) {
   ParticipantImpl *   This   = (ParticipantImpl *)self;
   dcrudStatus status = DCRUD_NO_ERROR;

   osMutex_take( This->cachesMutex );
   if( This->nextCacheID > 255 ) {
      *target = NULL;
      status  = DCRUD_TOO_MANY_CACHES;
   }
   else {
      *target = This->caches[This->nextCacheID++] = dcrudCache_new( This );
   }
   osMutex_release( This->cachesMutex );
   return status;
}

dcrudICache dcrudIParticipant_getCache( dcrudIParticipant self, byte ID ) {
   ParticipantImpl *   This  = (ParticipantImpl *)self;
   dcrudICache cache = NULL;

   osMutex_take( This->cachesMutex );
   cache = This->caches[ID];
   osMutex_release( This->cachesMutex );
   return cache;
}

dcrudIDispatcher dcrudIParticipant_getDispatcher( dcrudIParticipant self ) {
   ParticipantImpl * This  = (ParticipantImpl *)self;
   return This->dispatcher;
}

dcrudShareable dcrudIParticipant_createShareable( dcrudIParticipant self, dcrudClassID classID ) {
   ParticipantImpl * This    = (ParticipantImpl *)self;
   dcrudShareable    item    = NULL;
   dcrudIFactory *   factory = NULL;

   osMutex_take( This->factoriesMutex );
   factory = (dcrudIFactory *)collMap_get( This->factories, classID );
   osMutex_release( This->factoriesMutex );
   if( factory ) {
      item = dcrudShareable_new( factory, classID );
      if( !factory->initialize((dcrudShareable)item )) {
         dcrudShareable_delete( &item );
      }
   }
   else {
      char buffer[1024];
      dcrudClassID_toString( classID, buffer, sizeof( buffer ));
      fprintf( stderr, "%s:%d: No class '%s' found!\n", __FILE__, __LINE__, buffer );
   }
   return item;
}

dcrudShareable ParticipantImpl_newInstance( ParticipantImpl * This, ioByteBuffer frame ) {
   dcrudShareable  item    = NULL;
   dcrudIFactory * factory = NULL;
   dcrudClassID    classID = NULL;

   dcrudClassID_unserialize( frame, &classID );
   osMutex_take( This->factoriesMutex );
   factory = (dcrudIFactory *)collMap_get( This->factories, classID );
   osMutex_release( This->factoriesMutex );
   if( factory ) {
      dcrudShareableData data;
      item = dcrudShareable_new( factory, classID );
      data = dcrudShareable_getUserData( item );
      if( IO_STATUS_NO_ERROR != factory->unserialize( data, frame )) {
         dcrudShareable_delete( &item );
      }
   }
   return (dcrudShareable)item;
}

static bool pushCreateOrUpdateItem( collForeach * context ) {
   ParticipantImpl *    This = (ParticipantImpl *   )context->user;
   dcrudShareableImpl * item = (dcrudShareableImpl *)context->item;
   dcrudShareableData   data = dcrudShareable_getUserData((dcrudShareable)item );
   unsigned int         size;

   ioByteBuffer_clear    ( This->payload );
   item->serialize( data,  This->payload );
   ioByteBuffer_flip     ( This->payload );
   size = ioByteBuffer_remaining( This->payload );
   ioByteBuffer_clear    ( This->header );
   ioByteBuffer_put      ( This->header, DCRUD_SIGNATURE, 0, DCRUD_SIGNATURE_SIZE );
   ioByteBuffer_putByte  ( This->header, FRAMETYPE_DATA_CREATE_OR_UPDATE );
   ioByteBuffer_putInt   ( This->header, size );
   dcrudGUID_serialize   ( item->id     , This->header );
   dcrudClassID_serialize( item->classID, This->header );
   ioByteBuffer_flip     ( This->header );
   ioByteBuffer_clear    ( This->message );
   ioByteBuffer_putBuffer( This->message, This->header );
   ioByteBuffer_putBuffer( This->message, This->payload );
   ioByteBuffer_flip     ( This->message );
   /*
   dbgDump(
      stderr, ioByteBuffer_getBytes( This->message ), ioByteBuffer_remaining( This->message ));
   */
   ioByteBuffer_send     ( This->message, This->out, &This->target );
   return true;
}

void ParticipantImpl_publishUpdated( ParticipantImpl * This, collSet updated ) {
   osMutex_take( This->outMutex );
   collSet_foreach( updated, pushCreateOrUpdateItem, This );
   osMutex_release( This->outMutex );
}

static bool pushDeleteItem( collForeach * context ) {
   dcrudShareableImpl * item = (dcrudShareableImpl *)context->item;
   ParticipantImpl *    This = (ParticipantImpl *   )context->user;

   ioByteBuffer_clear  ( This->header );
   ioByteBuffer_put    ( This->header, DCRUD_SIGNATURE, 0, DCRUD_SIGNATURE_SIZE );
   ioByteBuffer_putByte( This->header, FRAMETYPE_DATA_DELETE );
   dcrudGUID_serialize( dcrudShareable_getGUID((dcrudShareable)item ), This->header );
   ioByteBuffer_flip   ( This->header );
   ioByteBuffer_send   ( This->header, This->out, &This->target );

   return true;
}

void ParticipantImpl_publishDeleted( ParticipantImpl * This, collSet deleted ) {
   osMutex_take( This->outMutex );
   collSet_foreach( deleted, pushDeleteItem, This );
   osMutex_release( This->outMutex );
}

void ParticipantImpl_sendCall(
   ParticipantImpl * This,
   const char *      intrfcName,
   const char *      opName,
   dcrudArguments    args,
   int               callId )
{
   osMutex_take( This->outMutex );
   ioByteBuffer_clear    ( This->message );
   ioByteBuffer_clear    ( This->header );
   ioByteBuffer_put      ( This->header, DCRUD_SIGNATURE, 0, DCRUD_SIGNATURE_SIZE );
   ioByteBuffer_putByte  ( This->header, FRAMETYPE_OPERATION );
   ioByteBuffer_putByte  ( This->header, (byte)( args ? dcrudArguments_getCount( args ) : 0 ));
   ioByteBuffer_flip     ( This->header );
   ioByteBuffer_putBuffer( This->message, This->header );
   ioByteBuffer_putString( This->message, intrfcName );
   ioByteBuffer_putString( This->message, opName );
   ioByteBuffer_putInt   ( This->message, (unsigned int)callId );
   if( args ) {
      dcrudArguments_serialize( args, This->message );
   }
   ioByteBuffer_flip     ( This->message );
   ioByteBuffer_send     ( This->message, This->out, &This->target );
   osMutex_release( This->outMutex );
}

void ParticipantImpl_call(
   ParticipantImpl * This,
   const char *      intrfcName,
   const char *      opName,
   dcrudArguments    args,
   dcrudICallback    callback )
{
   if( callback ) {
      ParticipantImpl_sendCall( This, intrfcName, opName, args, This->callId );
      collMap_put( This->callbacks, &This->callId, callback, NULL );
      This->callId++;
   }
   else {
      ParticipantImpl_sendCall( This, intrfcName, opName, args, 0 );
   }
}

bool ParticipantImpl_callback(
   ParticipantImpl * This,
   const char *      intrfcName,
   const char *      opName,
   dcrudArguments    args,
   int               callId )
{
   dcrudICallback callback = collMap_get( This->callbacks, &callId );
   if( callback == NULL ) {
      fprintf( stderr, "%s:%d:Unknown Callback received: %s.%s, id: %d\n",
         __FILE__, __LINE__, intrfcName, opName, -callId );
      return false;
   }
   dcrudICallback_callback( callback, intrfcName, opName, args );
   return true;
}
