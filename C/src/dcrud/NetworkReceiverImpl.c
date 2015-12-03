#include "INetworkReceiver.h"
#include "Dispatcher.h"
#include "Cache.h"
#include "ParticipantImpl.h"

#include <io/ByteBuffer.h>
#include <io/socket.h>

#include <os/System.h>

#include <util/CheckSysCall.h>

#include <dbg/Performance.h>
#include <dbg/Dump.h>

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

typedef struct NetworkReceiverImpl_s {

   ParticipantImpl * participant;
   SOCKET            in;
   ioByteBuffer      inBuf;
   pthread_t         thread;

} NetworkReceiverImpl;

static void dataDelete( NetworkReceiverImpl * This, ioByteBuffer frame ) {
   dcrudGUID    id;
   unsigned int c;

   dcrudGUID_unserialize( frame, &id );
   osMutex_take( This->participant->cachesMutex );
   for( c = 0; c < 256; ++c ) {
      if( This->participant->caches[c] ) {
         dcrudCache_deleteFromNetwork( This->participant->caches[c], &id );
      }
      else {
         break;
      }
   }
   osMutex_release( This->participant->cachesMutex );
}

static void dataUpdate( NetworkReceiverImpl * This, ioByteBuffer frame ) {
   unsigned int size = 0;
   unsigned int c;

   ioByteBuffer_getInt( frame, &size );
   osMutex_take( This->participant->cachesMutex );
   for( c = 0; c < 256; ++c ) {
      if( This->participant->caches[c] ) {
         dcrudCache_updateFromNetwork(
            This->participant->caches[c],
            ioByteBuffer_copy( frame, GUID_SIZE + CLASS_ID_SIZE + size ));
      }
      else {
         break;
      }
   }
   osMutex_release( This->participant->cachesMutex );
   ioByteBuffer_setPosition( frame,
      ioByteBuffer_getPosition( frame ) + GUID_SIZE + CLASS_ID_SIZE + size );
}

static void operation( NetworkReceiverImpl * This, ioByteBuffer frame ) {
   byte          count = 0;
   char          intrfcName[1000];
   char          opName[1000];
   int           callId;
   unsigned int  i;
   byte          queueNdx  = DCRUD_DEFAULT_QUEUE;
   dcrudCallMode callMode  = DCRUD_ASYNCHRONOUS_DEFERRED;
   collMap       arguments = collMap_new((collComparator)collStringComparator );

   ioByteBuffer_getByte  ( frame, &count );
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
         dcrudClassID classID;
         byte         pckg1, pckg2, pckg3, clazz;
         dcrudClassID_unserialize( frame, &classID );
         dcrudClassID_get( classID, &pckg1, &pckg2, &pckg3, &clazz );
         if(( pckg1 == 0 )&&( pckg2 == 0 )&&( pckg3 == 0 )) {
            switch( clazz ) {
            case 0: {
               dcrudClassID item;
               dcrudClassID_unserialize( frame, &item );
               collMap_put( arguments, strdup( argName ), item, NULL );
            }
            break;
            case 1:  /* byte    */
            case 2: {/* boolean */
               byte * item = (byte *)malloc( 1 );
               ioByteBuffer_getByte( frame, item );
               collMap_put( arguments, strdup( argName ), item, NULL );
            }
            break;
            case 3: {
               unsigned short * item = (unsigned short *)malloc( sizeof( short ));
               ioByteBuffer_getShort( frame, item );
               collMap_put( arguments, strdup( argName ), item, NULL );
            }
            break;
            case 4: {
               unsigned int * item = (unsigned int *)malloc( sizeof( int ));
               ioByteBuffer_getInt( frame, item );
               collMap_put( arguments, strdup( argName ), item, NULL );
            }
            break;
            case 5: {
               uint64_t * item = (uint64_t *)malloc( sizeof( uint64_t ));
               ioByteBuffer_getLong( frame, item );
               collMap_put( arguments, strdup( argName ), item, NULL );
            }
            break;
            case 6: {
               float * item = (float *)malloc( sizeof( float ));
               ioByteBuffer_getFloat( frame, item );
               collMap_put( arguments, strdup( argName ), item, NULL );
            }
            break;
            case 7: {
               double * item = (double *)malloc( sizeof( double ));
               ioByteBuffer_getDouble( frame, item );
               collMap_put( arguments, strdup( argName ), item, NULL );
            }
            break;
            case 8: {
               char item[64*1024];
               ioByteBuffer_getString( frame, item, sizeof( item ));
               collMap_put( arguments, strdup( argName ), strdup( item ), NULL );
            }
            break;
            case 9: {
               dcrudClassID item;
               dcrudClassID_unserialize( frame, &item );
               collMap_put( arguments, strdup( argName ), item, NULL );
            }
            break;
            case 10: {
               dcrudGUID item;
               dcrudGUID_unserialize( frame, &item );
               collMap_put( arguments, strdup( argName ), item, NULL );
            }
            break;
            default:
               fprintf( stderr, "%s:%d: Unexpected class ID: %d\n", __FILE__, __LINE__, clazz );
            break;
            }
         }
         else {
            dcrudShareable item = ParticipantImpl_newInstance( This->participant, frame );
            collMap_put( arguments, strdup( argName ), item, NULL );
         }
      }
   }
   if( callId > 0 ) {
      collMap out = collMap_new((collComparator)collStringComparator );
      dcrudIDispatcher_execute(
         This->participant->dispatcher,
         intrfcName,
         opName,
         arguments,
         out,
         callId,
         queueNdx,
         callMode );
   }
   else if( callId < 0 ) {
      int            key      = -callId;
      dcrudICallback callback = collMap_get( This->participant->callbacks, &key );
      if( callback == NULL ) {
         fprintf( stderr, "Unknown Callback received: %s.%s, id: %d\n",
            intrfcName, opName, -callId );
      }
      else {
         dcrudICallback_callback( callback, intrfcName, opName, arguments );
      }
   }
}

static void * run( NetworkReceiverImpl * This ) {
   char     signa[DCRUD_SIGNATURE_SIZE];
   uint64_t atStart = 0;

   while( true ) {
      if( atStart > 0 ) {
         dbgPerformance_record( "network", osSystem_nanotime() - atStart );
      }
      if( IO_STATUS_NO_ERROR == ioByteBuffer_receive( This->inBuf, This->in )) {
         atStart = osSystem_nanotime();
         ioByteBuffer_flip( This->inBuf );
         /*
         ioByteBuffer_dump( This->inBuf, stderr );
         */
         ioByteBuffer_get( This->inBuf, (byte *)signa, 0, sizeof( signa ));
         if( 0 == strncmp( signa, (const char *)DCRUD_SIGNATURE, DCRUD_SIGNATURE_SIZE )) {
            const FrameType frameType = FRAMETYPE_NO_OP;
            ioByteBuffer_getByte( This->inBuf, (byte*)&frameType );
            switch( frameType ) {
            case FRAMETYPE_DATA_CREATE_OR_UPDATE: dataUpdate( This, This->inBuf ); break;
            case FRAMETYPE_DATA_DELETE          : dataDelete( This, This->inBuf ); break;
            case FRAMETYPE_OPERATION            : operation ( This, This->inBuf ); break;
            default:
               fprintf( stderr, "%d isn't a valid FrameType\n", frameType );
               break;
            }
            if( ioByteBuffer_remaining( This->inBuf ) != 0 ) {
               fprintf( stderr, "%d received bytes ignored\n", ioByteBuffer_remaining( This->inBuf ));
            }
         }
         else {
            fprintf( stderr, "Garbage received, %d bytes discarded!\n",
               ioByteBuffer_getLimit( This->inBuf ));
         }
      }
      else {
         break;
      }
   }
   return NULL;
}

typedef void * ( * pthread_routine_t )( void * );

INetworkReceiver INetworkReceiver_new(
   ParticipantImpl * participant,
   const char *      address,
   unsigned short    port,
   const char *      intrfc )
{
   NetworkReceiverImpl * This = (NetworkReceiverImpl *)malloc( sizeof( NetworkReceiverImpl ));
   int                   trueValue = 1;
   struct sockaddr_in    local_sin;
   struct ip_mreq        mreq;

   memset( &local_sin, 0, sizeof( local_sin ));
   memset( &mreq     , 0, sizeof( mreq ));
   memset( This, 0, sizeof( NetworkReceiverImpl ));
   This->participant = participant;
   This->inBuf       = ioByteBuffer_new( 64*1024 );
   This->in          = socket( AF_INET, SOCK_DGRAM, 0 );
   if( ! utilCheckSysCall( This->in != INVALID_SOCKET, __FILE__, __LINE__, "socket" )) {
      return (INetworkReceiver)This;
   }
   if( ! utilCheckSysCall( 0 ==
      setsockopt( This->in, SOL_SOCKET, SO_REUSEADDR, (char*)&trueValue, sizeof( trueValue )),
      __FILE__, __LINE__, "setsockopt(SO_REUSEADDR)" ))
   {
      return (INetworkReceiver)This;
   }
   local_sin.sin_family      = AF_INET;
   local_sin.sin_port        = htons( port );
   local_sin.sin_addr.s_addr = htonl( INADDR_ANY );
   if( ! utilCheckSysCall( 0 ==
      bind( This->in, (struct sockaddr *)&local_sin, sizeof( local_sin )),
      __FILE__, __LINE__, "bind(%s,%d)", intrfc, port ))
   {
      return (INetworkReceiver)This;
   }
   mreq.imr_multiaddr.s_addr = inet_addr( address );
   mreq.imr_interface.s_addr = inet_addr( intrfc );
   if( ! utilCheckSysCall( 0 ==
      setsockopt( This->in, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof( mreq )),
      __FILE__, __LINE__, "setsockopt(IP_ADD_MEMBERSHIP,%s)", address ))
   {
      return (INetworkReceiver)This;
   }
   printf( "receiving from %s, bound to %s:%d\n", address, intrfc, port );
   if( ! utilCheckSysCall( 0 ==
      pthread_create( &This->thread, NULL, (pthread_routine_t)run, This ),
      __FILE__, __LINE__, "pthread_create" ))
   {
      return (INetworkReceiver)This;
   }
   return (INetworkReceiver)This;
}

void INetworkReceiver_delete( INetworkReceiver * self ) {
   NetworkReceiverImpl * This   = (NetworkReceiverImpl *)*self;
   void *                retVal = NULL;

   pthread_cancel( This->thread ); /* break ioByteBuffer_receive and cause the thread to exit */
   pthread_join( This->thread, &retVal );
   close( This->in );
   ioByteBuffer_delete( &This->inBuf );
   free( This );
   *self = NULL;
}
