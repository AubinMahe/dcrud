#include "ParticipantImpl.h"

#include "Cache.h"
#include "Dispatcher.h"
#include "Shareable_private.h"

#include <coll/List.h>
#include <coll/MapFuncPtr.h>

#include <os/System.h>

#include <dbg/Performance.h>
#include <dbg/Dump.h>
#include <os/Mutex.h>

#include <stdio.h>
#include <util/CheckSysCall.h>

const byte SIGNATURE[4] = { 'H','P','M','S' };

static unsigned int nextCacheID = 0;

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
   if( OS_STATUS_NO_ERROR != osMutex_new( &This->cachesMutex )) {
      perror( "Unable to create mutex\n" );
      return DCRUD_INIT_FAILED;
   }
   This->header  = ioByteBuffer_new( HEADER_SIZE );
   This->payload = ioByteBuffer_new( PAYLOAD_SIZE );
   This->message = ioByteBuffer_new( 64*1024 );/* UDP MAX packet size */
   if( OS_STATUS_NO_ERROR != osMutex_new( &This->classesMutex )) {
      perror( "Unable to create mutex\n" );
      return DCRUD_INIT_FAILED;
   }
   if( OS_STATUS_NO_ERROR != osMutex_new( &This->outMutex )) {
      perror( "Unable to create mutex\n" );
      return DCRUD_INIT_FAILED;
   }
   This->classes                = collMap_new((collComparator)dcrudClassID_compareTo );
   This->callbacks              = collMap_new((collComparator)IntegerCompare );
   This->publisherId            = publisherId;
   This->target.sin_family      = AF_INET;
   This->target.sin_port        = htons( port );
   This->target.sin_addr.s_addr = inet_addr( address );
   This->out                    = socket( AF_INET, SOCK_DGRAM, 0 );
   if( ! utilCheckSysCall( This->out != INVALID_SOCKET, __FILE__, __LINE__, "socket" )) {
      return DCRUD_INIT_FAILED;
   }
   utilCheckSysCall( This->out != INVALID_SOCKET, __FILE__, __LINE__, "socket" );
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
   This->dispatcher = dcrudIDispatcher_new( This );
   return DCRUD_NO_ERROR;
}

static void deleteRegisteredClasses( collForeach * context ) {
   collMapPair * pair = UTIL_CAST( collMapPair *, context->item );
   free( pair->value );
}

void ParticipantImpl_delete( ParticipantImpl * * self ) {
   ParticipantImpl * This = (ParticipantImpl *)*self;
   if( This ) {
      unsigned int i;
      for( i = 0; i < nextCacheID; ++i ) {
         if( This->caches[i] ) {
            dcrudCache_delete( &(This->caches[i] ));
         }
         else {
            break;
         }
      }
      collMap_foreach( This->classes, (collForeachFunction)deleteRegisteredClasses, NULL );
      osMutex_delete         ( &This->cachesMutex  );
      ioByteBuffer_delete    ( &This->header       );
      ioByteBuffer_delete    ( &This->payload      );
      ioByteBuffer_delete    ( &This->message      );
      osMutex_delete         ( &This->classesMutex );
      collMap_delete         ( &This->classes      );
      collMap_delete         ( &This->callbacks    );
      osMutex_delete         ( &This->outMutex     );
      close                  (  This->out          );
      dcrudIDispatcher_delete( &This->dispatcher   );
      free( This );
      *self = NULL;
   }
}

unsigned int ParticipantImpl_getMCastAddress( ParticipantImpl * This ) {
   return This->target.sin_addr.s_addr;
}

bool dcrudIParticipant_registerClass(
   dcrudIParticipant          self,
   dcrudClassID               id,
   size_t                     size,
   dcrudShareable_Initialize  initialize,
   dcrudShareable_Set         set,
   dcrudShareable_Serialize   serialize,
   dcrudShareable_Unserialize unserialize )
{
   ParticipantImpl *     This     = (ParticipantImpl *)self;
   dcrudShareableClass * factory  = (dcrudShareableClass *)malloc( sizeof( dcrudShareableClass ));
   collMapPair           previous;
   bool                  known;

   factory->size        = size;
   factory->initialize  = initialize;
   factory->set         = set;
   factory->serialize   = serialize;
   factory->unserialize = unserialize;
   osMutex_take( This->classesMutex );
   known = collMap_put( This->classes, id, (collMapValue)factory, &previous );
   if( known ) {
      free( previous.value );
   }
   osMutex_release( This->classesMutex );
   return known;
}

dcrudStatus dcrudIParticipant_createCache( dcrudIParticipant self, dcrudICache * target ) {
   ParticipantImpl *   This   = (ParticipantImpl *)self;
   dcrudStatus status = DCRUD_NO_ERROR;

   osMutex_take( This->cachesMutex );
   if( nextCacheID > 255 ) {
      *target = NULL;
      status  = DCRUD_TOO_MANY_CACHES;
   }
   else {
      *target =
      This->caches[nextCacheID++] = dcrudCache_new( This );
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
   ParticipantImpl *             This  = (ParticipantImpl *)self;
   dcrudShareable        item  = NULL;
   dcrudShareableClass * clazz = NULL;

   osMutex_take( This->classesMutex );
   clazz = (dcrudShareableClass *)collMap_get( This->classes, classID );
   osMutex_release( This->classesMutex );
   if( clazz ) {
      item = dcrudShareable_new( clazz, classID );
      if( !clazz->initialize((dcrudShareable)item )) {
         dcrudShareable_delete( &item );
      }
   }
   else {
      fprintf( stderr, "%s:%d: No class found!\n", __FILE__, __LINE__ );
   }
   return item;
}

dcrudShareable ParticipantImpl_newInstance( ParticipantImpl * This, ioByteBuffer frame ) {
   dcrudShareable        item  = NULL;
   dcrudShareableClass * clazz = NULL;
   dcrudClassID          classID;

   dcrudClassID_unserialize( frame, &classID );
   osMutex_take( This->classesMutex );
   clazz = (dcrudShareableClass *)collMap_get( This->classes, classID );
   osMutex_release( This->classesMutex );
   if( clazz ) {
      item = dcrudShareable_new( clazz, classID );
      if( IO_STATUS_NO_ERROR != clazz->unserialize( item, frame )) {
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
   ioByteBuffer_put      ( This->header, SIGNATURE, 0, sizeof( SIGNATURE ));
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
   ioByteBuffer_put    ( This->header, SIGNATURE, 0, sizeof( SIGNATURE ));
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

static bool serializeArgument( collForeach * context ) {
   ParticipantImpl * This = (ParticipantImpl *)context->user;
   collMapPair *     pair = (collMapPair *    )context->item;
   const char *      name = (const char *     )pair->key;

   ioByteBuffer_clear( This->payload );
   ioByteBuffer_putString( This->message, name );
   if( 0 == strcmp( name, "@queue" )) {
      byte * value = (byte *)pair->value;
      ioByteBuffer_putByte( This->payload, *value );
   }
   else if( 0 == strcmp( name, "@mode" )) {
      byte * value = (byte *)pair->value;
      ioByteBuffer_putByte( This->payload, *value );
   }
   else {
      dcrudShareableImpl * item = (dcrudShareableImpl *)pair->value;
      dcrudClassID_serialize( item->classID, This->payload );
      item->serialize((dcrudShareable)item, This->payload );
   }
   ioByteBuffer_flip( This->payload );
   ioByteBuffer_putBuffer( This->message, This->payload );
   return true;
}

void ParticipantImpl_sendCall(
   ParticipantImpl * This,
   const char *      intrfcName,
   const char *      opName,
   collMap           in,
   int               callId )
{
   osMutex_take( This->outMutex );
   ioByteBuffer_clear    ( This->message );
   ioByteBuffer_clear    ( This->header );
   ioByteBuffer_put      ( This->header, SIGNATURE, 0, sizeof( SIGNATURE ));
   ioByteBuffer_putByte  ( This->header, FRAMETYPE_OPERATION );
   ioByteBuffer_putInt   ( This->header, collMap_size( in ));
   ioByteBuffer_flip     ( This->header );
   ioByteBuffer_putBuffer( This->message, This->header );
   ioByteBuffer_putString( This->message, intrfcName );
   ioByteBuffer_putString( This->message, opName );
   ioByteBuffer_putInt   ( This->message, (unsigned int)callId );
   collMap_foreach( in, serializeArgument, This );
   ioByteBuffer_flip     ( This->message );
   ioByteBuffer_send     ( This->message, This->out, &This->target );
   osMutex_release( This->outMutex );
}

int ParticipantImpl_call(
   ParticipantImpl * This,
   const char *      intrfcName,
   const char *      opName,
   collMap           in,
   dcrudICallback    callback )
{
   ParticipantImpl_sendCall( This, intrfcName, opName, in, This->callId );
   if( callback ) {
      collMap_put( This->callbacks, &This->callId, callback, NULL );
      return This->callId++;
   }
   return 0;
}
