#include "ParticipantImpl.h"

#include "magic.h"
#include "poolSizes.h"
#include "Cache.h"
#include "Dispatcher.h"
#include "Shareable_private.h"
#include "NetworkReceiver.h"
#include "IProtocol.h"

#include <coll/List.h>
#include <coll/MapFuncPtr.h>

#include <os/System.h>
#include <os/Mutex.h>

#include <util/Pool.h>
#include <util/Dump.h>
#include <util/Performance.h>

#include <string.h>

UTIL_DEFINE_SAFE_CAST( dcrudIParticipant     )
UTIL_POOL_DECLARE    ( dcrudIParticipantImpl )

#define UDP_MAX_PACKET_SIZE 64*1024

const byte DCRUD_SIGNATURE[DCRUD_SIGNATURE_SIZE] = { 'D','C','R','U', 'D' };

#ifdef _WIN32
static void exitHook( void ) {
   WSACleanup();
}
#else
#  include <unistd.h>
#endif

utilStatus dcrudIParticipantImpl_new(
   dcrudIParticipantImpl **    target,
   unsigned int                publisherId,
   const ioInetSocketAddress * addr,
   const char *                intrfc  )
{
   int trueValue = 1;
   struct in_addr lIntrfc;
   dcrudIParticipantImpl * This;
   utilStatus status;
#ifdef _WIN32
   WSADATA wsaData;
   if( ! utilCheckSysCall( 0 ==
      WSAStartup( MAKEWORD( 2, 2 ), &wsaData ),
      __FILE__, __LINE__, "WSAStartup" ))
   {
      *target = NULL;
      return UTIL_STATUS_INIT_FAILED;
   }
   atexit( exitHook );
#endif
   if( NULL == target ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   dcrudIParticipant self;
   UTIL_ALLOCATE_ADT( dcrudIParticipant, &self, This );
   if( UTIL_STATUS_NO_ERROR != status ) {
      return status;
   }
   *target = This;
   memset( &lIntrfc, 0, sizeof( lIntrfc ));
   This->magic   = dcrudIParticipantImplMAGIC;
   This->header  = NULL;
   This->payload = NULL;
   This->message = NULL;
   CHK(__FILE__,__LINE__,osMutex_new( &This->cachesMutex ));
   CHK(__FILE__,__LINE__,ioByteBuffer_new( &This->header , HEADER_SIZE ));
   CHK(__FILE__,__LINE__,ioByteBuffer_new( &This->payload, PAYLOAD_SIZE ));
   CHK(__FILE__,__LINE__,ioByteBuffer_new( &This->message, UDP_MAX_PACKET_SIZE ));
   CHK(__FILE__,__LINE__,osMutex_new( &This->factoriesMutex ));
   CHK(__FILE__,__LINE__,osMutex_new( &This->publishersMutex ));
   CHK(__FILE__,__LINE__,osMutex_new( &This->outMutex ));
   CHK(__FILE__,__LINE__,collMap_new( &This->factories , (collComparator)dcrudClassID_compareTo ));
   CHK(__FILE__,__LINE__,collMap_new( &This->publishers, (collComparator)dcrudClassID_compareTo ));
   CHK(__FILE__,__LINE__,collMap_new( &This->callbacks , (collComparator)collIntCompare         ));
   This->publisherId = publisherId;
   This->target.sin_family = AF_INET;
   This->target.sin_port = htons( addr->port );
   This->target.sin_addr.s_addr = inet_addr( addr->inetAddress );
   This->out = socket( AF_INET, SOCK_DGRAM, 0 );
   if( This->out == INVALID_SOCKET ) {
      return UTIL_STATUS_STD_API_ERROR;
   }
   if( setsockopt( This->out, SOL_SOCKET, SO_REUSEADDR, (char*)&trueValue, sizeof( trueValue ))) {
      return UTIL_STATUS_STD_API_ERROR;
   }
   lIntrfc.s_addr = inet_addr( intrfc );
   if( setsockopt( This->out, IPPROTO_IP, IP_MULTICAST_IF, (char *)&lIntrfc, sizeof( lIntrfc ))) {
      return UTIL_STATUS_STD_API_ERROR;
   }
   CHK(__FILE__,__LINE__,dcrudIDispatcher_new( &This->dispatcher, This ));
   CHK(__FILE__,__LINE__,dcrudCache_new( &This->caches[This->cacheCount++], This ));
   CHK(__FILE__,__LINE__,collList_new( &(This->receivers)));
   return UTIL_STATUS_NO_ERROR;
}

typedef struct createReceiverParams_s {

   dcrudIParticipantImpl * participant;
   char                    networkInterface[NI_MAXHOST];

} createReceiverParams;

static utilStatus createReceiver( collForeach * context ) {
   const ioInetSocketAddress * addr   = (const ioInetSocketAddress *)context->value;
   createReceiverParams *      p      = (createReceiverParams *     )context->user;
   dcrudNetworkReceiver        rcvr   = NULL;
   utilStatus                  status =
      dcrudNetworkReceiver_new( &rcvr, p->participant, addr, p->networkInterface );
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = collList_add( p->participant->receivers, rcvr );
   }
   return status;
}

utilStatus dcrudIParticipant_listen(
   dcrudIParticipant self,
   dcrudIRegistry    registry,
   const char *      networkInterface )
{
   utilStatus              status = UTIL_STATUS_NO_ERROR;
   dcrudIParticipantImpl * This   = dcrudIParticipant_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      if( NULL == networkInterface ) {
         status = UTIL_STATUS_NULL_ARGUMENT;
      }
      else {
         collSet participants = NULL;
         status = dcrudIRegistry_getParticipants( registry, &participants );
         if( UTIL_STATUS_NO_ERROR == status ) {
            createReceiverParams p;
            p.participant = This;
            strncpy( p.networkInterface, networkInterface, NI_MAXHOST );
            if( UTIL_STATUS_NO_ERROR == status ) {
               status = collSet_foreach( participants, createReceiver, &p );
            }
         }
      }
   }
   return status;
}

utilStatus dcrudIParticipantImpl_delete( dcrudIParticipantImpl ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      unsigned int i;
      unsigned int count = 0;
      dcrudIParticipantImpl * This = *self;

      collList_size( This->receivers, &count );
      for( i = 0; i < count; ++i ) {
         dcrudNetworkReceiver item = NULL;
         if( collList_get( This->receivers, i, (collListItem*)&item ) == UTIL_STATUS_NO_ERROR ) {
            dcrudNetworkReceiver_delete( &item );
         }
      }
      for( i = 0; i < This->cacheCount; ++i ) {
         dcrudCache_delete( &(This->caches[i] ));
      }
      ioByteBuffer_delete    ( &This->header          );
      ioByteBuffer_delete    ( &This->payload         );
      ioByteBuffer_delete    ( &This->message         );
      osMutex_delete         ( &This->cachesMutex     );
      osMutex_delete         ( &This->factoriesMutex  );
      osMutex_delete         ( &This->publishersMutex );
      osMutex_delete         ( &This->outMutex        );
      collMap_delete         ( &This->factories       );
      collMap_delete         ( &This->publishers      );
      collMap_delete         ( &This->callbacks       );
      collList_delete        ( &This->receivers       );
      closesocket            (  This->out             );
      dcrudIDispatcher_delete( &This->dispatcher      );
      UTIL_RELEASE( dcrudIParticipantImpl )
   }
   return status;
}

utilStatus dcrudIParticipantImpl_getMCastAddress(
   dcrudIParticipantImpl * This,
   unsigned int *          result )
{
   *result = This->target.sin_addr.s_addr;
   return UTIL_STATUS_NO_ERROR;
}

utilStatus dcrudIParticipant_registerLocalFactory(
   dcrudIParticipant   self,
   dcrudLocalFactory * factory )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if(   NULL == factory
      || NULL == factory->allocateUserData
      || NULL == factory->serialize
      || NULL == factory->unserialize
      || NULL == factory->releaseUserData )
   {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   dcrudIParticipantImpl * This = dcrudIParticipant_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      CHK(__FILE__,__LINE__,osMutex_take( This->factoriesMutex ))
      status = collMap_put( This->factories, factory->classID, factory, NULL );
      osMutex_release( This->factoriesMutex );
   }
   return status;
}

utilStatus dcrudIParticipant_registerRemoteFactory(
   dcrudIParticipant    self,
   dcrudRemoteFactory * remoteFactory )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( remoteFactory == NULL || remoteFactory->classID == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIParticipantImpl * This = dcrudIParticipant_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = osMutex_take( This->publishersMutex );
         if( UTIL_STATUS_NO_ERROR == status ) {
            collMapPair previous;
            status = collMap_put(
               This->publishers,
               remoteFactory->classID,
               (collMapValue)remoteFactory,
               &previous );
            if( previous.key && previous.value ) {
               free( previous.value );
            }
            remoteFactory->participant = self;
            osMutex_release( This->publishersMutex );
         }
      }
   }
   return status;
}

utilStatus dcrudIParticipant_getDefaultCache( dcrudIParticipant self, dcrudICache * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( target == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIParticipantImpl * This = dcrudIParticipant_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         osMutex_take( This->cachesMutex );
         *target = This->caches[0];
         osMutex_release( This->cachesMutex );
      }
   }
   return status;
}

utilStatus dcrudIParticipant_createCache(
   dcrudIParticipant self,
   unsigned int *    id,
   dcrudICache *     target )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( id == NULL || target == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIParticipantImpl * This = dcrudIParticipant_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = osMutex_take( This->cachesMutex );
         if( UTIL_STATUS_NO_ERROR == status ) {
            if( This->cacheCount < CACHES_COUNT ) {
               *id = This->cacheCount++;
               status = dcrudCache_new( target, This );
               if( status == UTIL_STATUS_NO_ERROR ) {
                  This->caches[*id] = *target;
               }
            }
            else {
               *target = NULL;
               status  = UTIL_STATUS_TOO_MANY;
            }
            osMutex_release( This->cachesMutex );
         }
      }
   }
   return status;
}

utilStatus dcrudIParticipant_getCache(
   dcrudIParticipant self,
   unsigned int      id,
   dcrudICache *     result )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIParticipantImpl * This = dcrudIParticipant_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = osMutex_take( This->cachesMutex );
         if( UTIL_STATUS_NO_ERROR == status ) {
            *result = This->caches[id];
            osMutex_release( This->cachesMutex );
         }
      }
   }
   return status;
}

utilStatus dcrudIParticipant_getDispatcher( dcrudIParticipant self, dcrudIDispatcher * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIParticipantImpl * This = dcrudIParticipant_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         *result = This->dispatcher;
      }
   }
   return status;
}

utilStatus dcrudIParticipant_createShareable(
   dcrudIParticipant  self,
   dcrudClassID       classID,
   dcrudShareableData data,
   dcrudShareable *   result )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIParticipantImpl * This = dcrudIParticipant_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = osMutex_take( This->factoriesMutex );
         if( UTIL_STATUS_NO_ERROR == status ) {
            dcrudLocalFactory * factory = NULL;
            status = collMap_get( This->factories, classID, &factory );
            osMutex_release( This->factoriesMutex );
            status = dcrudShareable_new( result, classID, factory, data );
         }
      }
   }
   return status;
}

utilStatus dcrudIParticipantImpl_newInstance(
   dcrudIParticipantImpl * This,
   ioByteBuffer            frame,
   dcrudShareableData      data,
   dcrudShareable *        result )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudClassID classID = NULL;
      CHK(__FILE__,__LINE__,dcrudClassID_unserialize( &classID, frame ))
      CHK(__FILE__,__LINE__,osMutex_take( This->factoriesMutex ))
      dcrudLocalFactory * factory = NULL;
      status = collMap_get( This->factories, classID, &factory );
      osMutex_release( This->factoriesMutex );
      if( UTIL_STATUS_NO_ERROR == status ) {
         if( NULL == data ) {
            CHK(__FILE__,__LINE__,factory->allocateUserData( &data ))
         }
         CHK(__FILE__,__LINE__,dcrudShareable_new( result, classID, factory, data ))
         CHK(__FILE__,__LINE__,dcrudShareable_getData( *result, &data ))
         CHK(__FILE__,__LINE__,factory->unserialize( data, frame ))
      }
      else {
         CHK(__FILE__,__LINE__,dcrudClassID_delete( &classID ))
      }
   }
   return status;
}

static utilStatus pushCreateOrUpdateItem( collForeach * context ) {
   utilStatus              status = UTIL_STATUS_NO_ERROR;
   dcrudIParticipantImpl * This   = dcrudIParticipant_safeCast( context->user, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      dcrudShareableImpl * item = dcrudShareable_safeCast( context->value, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         dcrudShareableData   data = NULL;
         size_t size;
         status = dcrudShareable_getData((dcrudShareable)item, &data );
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_clear( This->payload );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = item->factory->serialize( data, This->payload );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_flip( This->payload );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_remaining( This->payload, &size );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_clear( This->header );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_put( This->header, DCRUD_SIGNATURE, 0, DCRUD_SIGNATURE_SIZE );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_putByte( This->header, FRAMETYPE_DATA_CREATE_OR_UPDATE );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_putUInt( This->header, (unsigned int)size );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = dcrudGUID_serialize( item->id, This->header );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = dcrudClassID_serialize( item->classID, This->header );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_flip( This->header );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_clear( This->message );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_putBuffer( This->message, This->header );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_putBuffer( This->message, This->payload );
         }
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_flip( This->message );
         }
         /*
         dbgDump(
            stderr,
            ioByteBuffer_getBytes ( This->message ),
            ioByteBuffer_remaining( This->message ) );
         */
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = ioByteBuffer_sendTo( This->message, This->out, &This->target );
         }
      }
   }
   return status;
}

utilStatus dcrudIParticipantImpl_publishUpdated( dcrudIParticipantImpl * This, collSet updated ) {
   utilStatus status = osMutex_take( This->outMutex );
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = collSet_foreach( updated, pushCreateOrUpdateItem, This );
      osMutex_release( This->outMutex );
   }
   return status;
}

static utilStatus pushDeleteItem( collForeach * context ) {
   utilStatus              status = UTIL_STATUS_NO_ERROR;
   dcrudIParticipantImpl * This   = (dcrudIParticipantImpl *)context->user;
   dcrudShareableImpl *    item   = dcrudShareable_safeCast( context->value, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      dcrudGUID guid;
      status = ioByteBuffer_clear( This->header );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = ioByteBuffer_put( This->header, DCRUD_SIGNATURE, 0, DCRUD_SIGNATURE_SIZE );
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = ioByteBuffer_putByte( This->header, FRAMETYPE_DATA_DELETE );
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = dcrudShareable_getGUID((dcrudShareable)item, &guid );
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = dcrudGUID_serialize( guid, This->header );
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = ioByteBuffer_flip( This->header );
      }
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = ioByteBuffer_sendTo( This->header, This->out, &This->target );
      }
   }
   return status;
}

utilStatus dcrudIParticipantImpl_publishDeleted( dcrudIParticipantImpl * This, collSet deleted ) {
   utilStatus status = osMutex_take( This->outMutex );
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = collSet_foreach( deleted, pushDeleteItem, This );
      osMutex_release( This->outMutex );
   }
   return status;
}

utilStatus dcrudIParticipantImpl_sendCall(
   dcrudIParticipantImpl * This,
   const char *            intrfcName,
   const char *            opName,
   dcrudArguments          args,
   int                     callId )
{
   utilStatus   status = UTIL_STATUS_NO_ERROR;
   unsigned int count  = 0U;
   CHK(__FILE__,__LINE__,dcrudArguments_getCount( args, &count ))
   CHK(__FILE__,__LINE__,osMutex_take( This->outMutex ))
   status = ioByteBuffer_clear( This->message );
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_put( This->message, DCRUD_SIGNATURE, 0, DCRUD_SIGNATURE_SIZE );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putByte( This->message, FRAMETYPE_OPERATION );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putString( This->message, intrfcName );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putString( This->message, opName );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putInt( This->message, callId );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_putByte( This->message, count );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = dcrudArguments_serialize( args, This->message );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_flip( This->message );
   }
   if( UTIL_STATUS_NO_ERROR == status ) {
      status = ioByteBuffer_sendTo( This->message, This->out, &This->target );
   }
   CHK(__FILE__,__LINE__,osMutex_release( This->outMutex ))
   return status;
}

utilStatus dcrudIParticipantImpl_call(
   dcrudIParticipantImpl * This,
   const char *            intrfcName,
   const char *            opName,
   dcrudArguments          args,
   dcrudICallback          callback )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( callback ) {
      status = dcrudIParticipantImpl_sendCall( This, intrfcName, opName, args, This->callId );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = collMap_put( This->callbacks, &This->callId, callback, NULL );
         This->callId++;
      }
   }
   else {
      status = dcrudIParticipantImpl_sendCall( This, intrfcName, opName, args, 0 );
   }
   return status;
}

utilStatus dcrudIParticipantImpl_callback(
   dcrudIParticipantImpl * This,
   const char *            intrfcName,
   const char *            opName,
   dcrudArguments          args,
   int                     callId )
{
   dcrudICallback callback = NULL;
   CHK(__FILE__,__LINE__,collMap_get( This->callbacks, &callId, &callback ))
   CHK(__FILE__,__LINE__,dcrudICallback_callback( callback, intrfcName, opName, args ))
   return UTIL_STATUS_NO_ERROR;
}

utilStatus dcrudIParticipantImpl_createData(
   dcrudIParticipantImpl * This,
   dcrudClassID            classId,
   dcrudArguments          how )
{
   dcrudRemoteFactory * factory = NULL;
   CHK(__FILE__,__LINE__,collMap_get( This->publishers, classId, &factory ))
   CHK(__FILE__,__LINE__,factory->create( factory, how ))
   return UTIL_STATUS_NO_ERROR;
}

utilStatus dcrudIParticipantImpl_updateData(
   dcrudIParticipantImpl * This,
   dcrudGUID               guid,
   dcrudArguments          how )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   byte i;
   for( i = 0; i < This->cacheCount; ++i ) {
      dcrudShareable       what = NULL;
      dcrudClassID         classID;
      dcrudRemoteFactory * factory;
      CHK(__FILE__,__LINE__,dcrudICache_read( This->caches[i], guid, &what ))
      CHK(__FILE__,__LINE__,dcrudShareable_getClassID( what, &classID ))
      CHK(__FILE__,__LINE__,collMap_get( This->publishers, classID, &factory ))
      CHK(__FILE__,__LINE__,factory->update( factory, what, how ))
   }
   return status;
}

utilStatus dcrudIParticipantImpl_deleteData( dcrudIParticipantImpl * This, dcrudGUID guid ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   byte i;
   for( i = 0; i < This->cacheCount; ++i ) {
      dcrudShareable       what = NULL;
      dcrudClassID         classID;
      dcrudRemoteFactory * factory;
      CHK(__FILE__,__LINE__,dcrudICache_read( This->caches[i], guid, &what ))
      CHK(__FILE__,__LINE__,dcrudShareable_getClassID( what, &classID ))
      CHK(__FILE__,__LINE__,collMap_get( This->publishers, classID, &factory ))
      CHK(__FILE__,__LINE__,factory->delete( factory, what ))
   }
   return status;
}
