#include "NetworkReceiver.h"
#include "Dispatcher.h"
#include "Cache.h"
#include "ParticipantImpl.h"
#include "IProtocol.h"
#include "magic.h"
#include "poolSizes.h"

#include <util/DebugSettings.h>
#include <util/Dump.h>
#include <util/Performance.h>
#include <util/Pool.h>
#include <util/String.h>

#include <os/System.h>
#include <os/threads.h>

#include <io/ByteBuffer.h>

#include <stdio.h>

typedef struct dcrudNetworkReceiverImpl_s {

   unsigned                magic;
   dcrudIParticipantImpl * participant;
   int                     in;
   ioByteBuffer            inBuf;
   osThread                thread;

} dcrudNetworkReceiverImpl;

void dcrudNetworkReceiver_dataDelete( dcrudNetworkReceiverImpl * This, ioByteBuffer frame ) {
   dcrudGUID    id;
   unsigned int c;

   dcrudGUID_unserialize( &id, frame );
   osMutex_take( This->participant->cachesMutex );
   for( c = 0; c < CACHES_COUNT; ++c ) {
      if( This->participant->caches[c] ) {
         dcrudCache_deleteFromNetwork( This->participant->caches[c], id );
      }
   }
   osMutex_release( This->participant->cachesMutex );
}

utilStatus dcrudNetworkReceiver_dataUpdate( dcrudNetworkReceiverImpl * This, ioByteBuffer frame ) {
   utilStatus   status = UTIL_STATUS_NO_ERROR;
   unsigned int size   = 0;
   unsigned int c;

   CHK(__FILE__,__LINE__,ioByteBuffer_getUInt( frame, &size ))
   CHK(__FILE__,__LINE__,osMutex_take( This->participant->cachesMutex ))
   for( c = 0; ( status == UTIL_STATUS_NO_ERROR )&&( c < CACHES_COUNT ); ++c ) {
      if( This->participant->caches[c] ) {
         ioByteBuffer copy;
         status = ioByteBuffer_copy( frame, &copy, GUID_SIZE + CLASS_ID_SIZE + size );
         if( UTIL_STATUS_NO_ERROR == status ) {
            status = dcrudCache_updateFromNetwork( This->participant->caches[c], copy );
         }
      }
      else {
         break;
      }
   }
   osMutex_release( This->participant->cachesMutex );
   if( status == UTIL_STATUS_NO_ERROR ) {
      size_t pos = 0;
      status = ioByteBuffer_getPosition( frame, &pos );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = ioByteBuffer_setPosition( frame, pos + GUID_SIZE + CLASS_ID_SIZE + size );
      }
   }
   return status;
}

utilStatus dcrudNetworkReceiver_operation( dcrudNetworkReceiverImpl * This, ioByteBuffer frame ) {
   utilStatus     status = UTIL_STATUS_NO_ERROR;
   byte           count  = 0;
   char           intrfcName[INTERFACE_NAME_MAX_LENGTH];
   char           opName    [OPERATION_NAME_MAX_LENGTH];
   int            callId;
   unsigned int    i;
   dcrudQueueIndex queueNdx = DCRUD_DEFAULT_QUEUE;
   dcrudCallMode   callMode = DCRUD_ASYNCHRONOUS_DEFERRED;
   dcrudArguments  args     = NULL;
   bool            alive    = true;

   CHK(__FILE__,__LINE__,ioByteBuffer_getString( frame, intrfcName, sizeof( intrfcName )))
   CHK(__FILE__,__LINE__,ioByteBuffer_getString( frame, opName    , sizeof( opName     )))
   CHK(__FILE__,__LINE__,ioByteBuffer_getInt   ( frame, &callId ))
   CHK(__FILE__,__LINE__,ioByteBuffer_getByte  ( frame, &count  ))
   CHK(__FILE__,__LINE__,ioByteBuffer_getByte  ( frame, (byte *)&callMode ))
   CHK(__FILE__,__LINE__,ioByteBuffer_getByte  ( frame, (byte *)&queueNdx ))
   CHK(__FILE__,__LINE__,dcrudArguments_new( &args ))
   CHK(__FILE__,__LINE__,dcrudArguments_setMode ( args, callMode ))
   CHK(__FILE__,__LINE__,dcrudArguments_setQueue( args, queueNdx ))
   if( utilDebugSettings->dumpNetworkReceiverOperations ) {
      fprintf( stderr, "NetworkReceiver.operation\n" );
      fprintf( stderr, "\tintrfcName: %s\n", intrfcName );
      fprintf( stderr, "\t    opName: %s\n", opName );
      fprintf( stderr, "\t    callId: %d\n", callId );
      fprintf( stderr, "\t     count: %d\n", count );
      fprintf( stderr, "\t  callMode: %d\n", callMode );
      fprintf( stderr, "\t  queueNdx: %d\n", queueNdx );
   }
   for( i = 0; alive && ( i < count ); ++i ) {
      char         name[ARG_NAME_MAX_LENGTH];
      dcrudClassID classID;
      dcrudType    type;

      CHK(__FILE__,__LINE__,ioByteBuffer_getString( frame, name, sizeof( name )))
      CHK(__FILE__,__LINE__,dcrudClassID_unserialize( &classID, frame ))
      CHK(__FILE__,__LINE__,dcrudClassID_getType( classID, &type ))
      if( utilDebugSettings->dumpNetworkReceiverOperations ) {
         char szClassID[40];
         CHK(__FILE__,__LINE__,dcrudClassID_toString( classID, szClassID, sizeof( szClassID )))
         fprintf( stderr, "\t\t%d:                name: %s\n", i, name );
         fprintf( stderr, "\t\t%d:             classID: %s\n", i, szClassID );
         fprintf( stderr, "\t\t%d:                type: %d\n", i, type );
      }
      switch( type ) {
      case dcrudTYPE_NULL:
         CHK(__FILE__,__LINE__,dcrudArguments_putNull( args, name ))
         break;
      case dcrudTYPE_BYTE:{
         byte item;
         CHK(__FILE__,__LINE__,ioByteBuffer_getByte( frame, &item ))
         CHK(__FILE__,__LINE__,dcrudArguments_putByte( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:          byte value: %d\n", i, item );
         }
      }
      break;
      case dcrudTYPE_BOOLEAN:{
         byte item;
         CHK(__FILE__,__LINE__,ioByteBuffer_getByte( frame, &item ))
         CHK(__FILE__,__LINE__,dcrudArguments_putBoolean( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:       boolean value: %d\n", i, item );
         }
      }break;
      case dcrudTYPE_SHORT:{
         short item;
         CHK(__FILE__,__LINE__,ioByteBuffer_getShort( frame, &item ))
         CHK(__FILE__,__LINE__,dcrudArguments_putShort( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:         short value: %d\n", i, item );
         }
      }break;
      case dcrudTYPE_UNSIGNED_SHORT:{
         unsigned short item;
         CHK(__FILE__,__LINE__,ioByteBuffer_getUShort( frame, &item ))
         CHK(__FILE__,__LINE__,dcrudArguments_putUshort( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:unsigned short value: %d\n", i, item );
         }
      }break;
      case dcrudTYPE_INTEGER:{
         unsigned int item;
         CHK(__FILE__,__LINE__,ioByteBuffer_getUInt( frame, &item ))
         CHK(__FILE__,__LINE__,dcrudArguments_putUint( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:           int value: %d\n", i, item );
         }
      }break;
      case dcrudTYPE_UNSIGNED_INTEGER:{
         unsigned int item;
         CHK(__FILE__,__LINE__,ioByteBuffer_getUInt( frame, &item ))
         CHK(__FILE__,__LINE__,dcrudArguments_putUint( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:  unsigned int value: %d\n", i, item );
         }
      }break;
      case dcrudTYPE_LONG:{
         int64_t item;
         CHK(__FILE__,__LINE__,ioByteBuffer_getLong( frame, &item ))
         CHK(__FILE__,__LINE__,dcrudArguments_putLong( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:          long value: %"PRId64"\n", i, item );
         }
      }break;
      case dcrudTYPE_UNSIGNED_LONG:{
         uint64_t item;
         CHK(__FILE__,__LINE__,ioByteBuffer_getULong( frame, &item ))
         CHK(__FILE__,__LINE__,dcrudArguments_putUlong( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d: unsigned long value: %"PRIu64"\n", i, item );
         }
      }break;
      case dcrudTYPE_FLOAT:{
         float item;
         CHK(__FILE__,__LINE__,ioByteBuffer_getFloat( frame, &item ))
         CHK(__FILE__,__LINE__,dcrudArguments_putFloat( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:               float: %f\n", i, item );
         }
      }break;
      case dcrudTYPE_DOUBLE:{
         double item;
         CHK(__FILE__,__LINE__,ioByteBuffer_getDouble( frame, &item ))
         CHK(__FILE__,__LINE__,dcrudArguments_putDouble( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:              double: %f\n", i, item );
         }
      }break;
      case dcrudTYPE_STRING:{
         char item[64*1024];
         CHK(__FILE__,__LINE__,ioByteBuffer_getString( frame, item, sizeof( item )))
         CHK(__FILE__,__LINE__,dcrudArguments_putString( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:              string: %s\n", i, item );
         }
      }break;
      case dcrudTYPE_CLASS_ID:{
         dcrudClassID item = NULL;
         char szClassID[40];
         CHK(__FILE__,__LINE__,dcrudClassID_unserialize( &item, frame ))
         CHK(__FILE__,__LINE__,dcrudClassID_toString   ( classID, szClassID, sizeof( szClassID )))
         CHK(__FILE__,__LINE__,dcrudArguments_putClassID( args, name, item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:             classID: %s\n", i, szClassID );
         }
      }break;
      case dcrudTYPE_GUID:{
         dcrudGUID item = NULL;
         char szGUID[40];
         CHK(__FILE__,__LINE__,dcrudGUID_unserialize( &item, frame ))
         CHK(__FILE__,__LINE__,dcrudGUID_toString   ( item, szGUID, sizeof( szGUID )))
         CHK(__FILE__,__LINE__,dcrudArguments_putGUID( args, name, item ))
         CHK(__FILE__,__LINE__,dcrudGUID_delete     ( &item ))
         if( utilDebugSettings->dumpNetworkReceiverOperations ) {
            fprintf( stderr, "\t\t%d:                GUID: %s\n", i, szGUID );
         }
      }break;
      case dcrudTYPE_SHAREABLE:{
         dcrudShareable item = NULL;
         CHK(__FILE__,__LINE__,
            dcrudIParticipantImpl_newInstance( This->participant, frame, NULL, &item ))
         CHK(__FILE__,__LINE__,dcrudArguments_putShareable( args, name, item ))
      }break;
      default:{
         char cidas[100];
         dcrudClassID_toString( classID, cidas, sizeof( cidas ));
         fprintf( stderr,
            "%s:%d:Unexpected type: %3d (%02X), intrfcName: '%s', operation: '%s',"
            " call-id: %d, arg-count: %d, arg-name: '%s', class-id: %s\n",
            __FILE__, __LINE__, (int)type, (int)type, intrfcName, opName,
            callId, count, name, cidas );
         status = UTIL_STATUS_ILLEGAL_STATE;
      }break;
      }
      CHK(__FILE__,__LINE__,dcrudClassID_delete( &classID ))
   }
   CHK(__FILE__,__LINE__,dcrudIParticipant_isAlive((dcrudIParticipant)This->participant, &alive ))
   if( alive ) {
      if( 0 ==  strcmp( intrfcName, ICRUD_INTERFACE_NAME )) {
         CHK(__FILE__,__LINE__,
            dcrudIDispatcher_executeCrud( This->participant->dispatcher, opName, args ))
      }
      else if( callId >= 0 ) {
         CHK(__FILE__,__LINE__,
            dcrudIDispatcher_execute( This->participant->dispatcher, intrfcName, opName, args, callId ))
      }
      else if( callId < 0 ) {
         CHK(__FILE__,__LINE__,
            dcrudIParticipantImpl_callback( This->participant, intrfcName, opName, args, -callId ))
      }
   }
   else {
      CHK(__FILE__,__LINE__,dcrudArguments_delete( &args ))
   }
   return status;
}

void * dcrudNetworkReceiver_run( dcrudNetworkReceiverImpl * This ) {
   char     signa[DCRUD_SIGNATURE_SIZE];
#ifdef PERFORMANCE
   uint64_t atStart = 0;
#endif
   bool     alive = true;

   while( alive ) {
#ifdef PERFORMANCE
      if( atStart > 0 ) {
         dbgPerformance_record( "network", osSystem_nanotime() - atStart );
      }
#endif
      ioByteBuffer_clear( This->inBuf );
      if( UTIL_STATUS_NO_ERROR == ioByteBuffer_receive( This->inBuf, This->in )) {
#ifdef PERFORMANCE
         atStart = osSystem_nanotime();
#endif
         ioByteBuffer_flip( This->inBuf );
         if( utilDebugSettings->dumpReceivedBuffer ) {
            ioByteBuffer_dump( This->inBuf, stderr );
         }
         ioByteBuffer_get( This->inBuf, (byte *)signa, 0, sizeof( signa ));
         if( 0 == strncmp( signa, (const char *)DCRUD_SIGNATURE, DCRUD_SIGNATURE_SIZE )) {
            FrameType frameType = FRAMETYPE_NO_OP;
            size_t ignored = 0;
            ioByteBuffer_getByte( This->inBuf, (byte*)&frameType );
            switch( frameType ) {
            case FRAMETYPE_DATA_CREATE_OR_UPDATE: dcrudNetworkReceiver_dataUpdate( This, This->inBuf ); break;
            case FRAMETYPE_DATA_DELETE          : dcrudNetworkReceiver_dataDelete( This, This->inBuf ); break;
            case FRAMETYPE_OPERATION            : dcrudNetworkReceiver_operation ( This, This->inBuf ); break;
            default:
               fprintf( stderr, "%s:%d:%d isn't a valid FrameType\n",
                  __FILE__, __LINE__, frameType );
               break;
            }
            ioByteBuffer_remaining( This->inBuf, &ignored );
            if( ignored != 0 ) {
               fprintf( stderr, "%s:%d:%lu received byte%s ignored\n",
                  __FILE__, __LINE__, (long unsigned int)ignored, ignored > 1 ? "s": "" );
            }
         }
         else {
            size_t limit;
            ioByteBuffer_getLimit( This->inBuf, &limit );
            fprintf( stderr, "%s:%d:Garbage received, %lu bytes discarded!\n",
               __FILE__, __LINE__, (long unsigned int)limit );
         }
      }
      else {
         break;
      }
      dcrudIParticipant_isAlive((dcrudIParticipant)This->participant, &alive );
   }
   return NULL;
}

typedef void * ( * pthread_routine_t )( void * );

UTIL_DEFINE_SAFE_CAST( dcrudNetworkReceiver     )
UTIL_POOL_DECLARE    ( dcrudNetworkReceiverImpl )

utilStatus dcrudNetworkReceiver_new(
   dcrudNetworkReceiver *      self,
   dcrudIParticipantImpl *     participant,
   const ioInetSocketAddress * addr,
   const char *                intrfc )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self || NULL == participant || NULL == addr || NULL == intrfc ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudNetworkReceiverImpl * This = NULL;
      UTIL_ALLOCATE_ADT( dcrudNetworkReceiver, self, This );
      if( UTIL_STATUS_NO_ERROR == status ) {
         struct sockaddr_in local_sin;
         struct ip_mreq     mreq;
         int                T = 1;
         int                sckt;

         memset( &local_sin, 0, sizeof( local_sin ));
         memset( &mreq     , 0, sizeof( mreq ));
         local_sin.sin_family      = AF_INET;
         local_sin.sin_port        = htons( addr->port );
         local_sin.sin_addr.s_addr = htonl( INADDR_ANY );
         mreq.imr_multiaddr.s_addr = inet_addr( addr->inetAddress );
         mreq.imr_interface.s_addr = inet_addr( intrfc );
         CHK(__FILE__,__LINE__,((sckt = socket( AF_INET, SOCK_DGRAM, 0 ))<0)?UTIL_STATUS_STD_API_ERROR:UTIL_STATUS_NO_ERROR);
         This->in = sckt;
         CHK(__FILE__,__LINE__,setsockopt( This->in, SOL_SOCKET, SO_REUSEADDR, (char*)&T, sizeof( T ))?UTIL_STATUS_STD_API_ERROR:UTIL_STATUS_NO_ERROR);
         CHK(__FILE__,__LINE__,bind( This->in, (struct sockaddr *)&local_sin, sizeof( local_sin ))?UTIL_STATUS_STD_API_ERROR:UTIL_STATUS_NO_ERROR);
         CHK(__FILE__,__LINE__,setsockopt( This->in, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof( mreq ))?UTIL_STATUS_STD_API_ERROR:UTIL_STATUS_NO_ERROR);
         CHK(__FILE__,__LINE__,ioByteBuffer_new( &This->inBuf, 64*1024 ));
         This->participant = participant;
         CHK(__FILE__,__LINE__,
            osThread_create( &This->thread, (osThreadRoutine)dcrudNetworkReceiver_run, This )
            ? UTIL_STATUS_NO_ERROR : UTIL_STATUS_STD_API_ERROR );
      }
   }
   return status;
}

utilStatus dcrudNetworkReceiver_delete( dcrudNetworkReceiver * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   dcrudNetworkReceiverImpl * This = *(dcrudNetworkReceiverImpl **)self;
#ifdef _linux
   shutdown( This->in, SHUT_RD );
#endif
   osThread_join( This->thread );
   closesocket( This->in );
   CHK(__FILE__,__LINE__,ioByteBuffer_delete( &This->inBuf ))
   UTIL_RELEASE( dcrudNetworkReceiverImpl );
   return status;
}
