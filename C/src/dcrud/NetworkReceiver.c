#include "NetworkReceiver.h"
#include "Dispatcher.h"
#include "Cache.h"
#include "ParticipantImpl.h"

#include <io/ByteBuffer.h>

#include <os/System.h>

#include <util/CheckSysCall.h>

#include <dbg/Performance.h>
#include <dbg/Dump.h>

#if defined( WIN32 ) || defined( _WIN32 )
#  include <windows.h>
#  pragma warning(disable : 4996)
#else
#  include <pthread.h>
#  include <unistd.h>
#endif
#include <stdio.h>

typedef struct NetworkReceiver_s {

   ParticipantImpl * participant;
   SOCKET            in;
   ioByteBuffer      inBuf;
#ifdef WIN32
   HANDLE            thread;
#else
   pthread_t         thread;
#endif
} NetworkReceiver;

static void dataDelete( NetworkReceiver * This, ioByteBuffer frame ) {
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

static void dataUpdate( NetworkReceiver * This, ioByteBuffer frame ) {
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

static void operation( NetworkReceiver * This, ioByteBuffer frame ) {
   byte           count = 0;
   char           intrfcName[1000];
   char           opName[1000];
   int            callId;
   unsigned int   i;
   byte           queueNdx = DCRUD_DEFAULT_QUEUE;
   dcrudCallMode  callMode = DCRUD_ASYNCHRONOUS_DEFERRED;
   dcrudArguments args     = dcrudArguments_new();

   ioByteBuffer_getByte  ( frame, &count );
   ioByteBuffer_getString( frame, intrfcName, sizeof( intrfcName ));
   ioByteBuffer_getString( frame, opName    , sizeof( opName     ));
   ioByteBuffer_getInt   ( frame, (unsigned int *)&callId );
   for( i = 0; i < count; ++i ) {
      char         name[1000];
      dcrudClassID classID;
      dcrudType    type;

      ioByteBuffer_getString( frame, name, sizeof( name ));
      dcrudClassID_unserialize( frame, &classID );
      type = dcrudClassID_getType( classID );
      switch( type ) {
      case dcrudTYPE_NULL: dcrudArguments_putNull( args, name ); break;
      case dcrudTYPE_BYTE:{
         byte item;
         ioByteBuffer_getByte( frame, &item );
         dcrudArguments_putByte( args, name, item );
      }break;
      case dcrudTYPE_BOOLEAN:{
         byte item;
         ioByteBuffer_getByte( frame, &item );
         dcrudArguments_putBoolean( args, name, item );
      }break;
      case dcrudTYPE_SHORT:{
         unsigned short item;
         ioByteBuffer_getShort( frame, &item );
         dcrudArguments_putShort( args, name, item );
      }break;
      case dcrudTYPE_INTEGER:{
         unsigned int item;
         ioByteBuffer_getInt( frame, &item );
         dcrudArguments_putInt( args, name, item );
      }break;
      case dcrudTYPE_LONG:{
         uint64_t item;
         ioByteBuffer_getLong( frame, &item );
         dcrudArguments_putLong( args, name, item );
      }break;
      case dcrudTYPE_FLOAT:{
         float item;
         ioByteBuffer_getFloat( frame, &item );
         dcrudArguments_putFloat( args, name, item );
      }break;
      case dcrudTYPE_DOUBLE:{
         double item;
         ioByteBuffer_getDouble( frame, &item );
         dcrudArguments_putDouble( args, name, item );
      }break;
      case dcrudTYPE_STRING:{
         char item[64*1024];
         ioByteBuffer_getString( frame, item, sizeof( item ));
         dcrudArguments_putString( args, name, item );
      }break;
      case dcrudTYPE_CLASS_ID:{
         dcrudClassID item;
         dcrudClassID_unserialize( frame, &item );
         dcrudArguments_putClassID( args, name, item );
      }break;
      case dcrudTYPE_GUID:{
         dcrudGUID item;
         dcrudGUID_unserialize( frame, &item );
         dcrudArguments_putGUID( args, name, item );
      }break;
      case dcrudTYPE_CALL_MODE:{
         byte item;
         ioByteBuffer_getByte( frame, &item );
         dcrudArguments_setMode( args, item );
      }break;
      case dcrudTYPE_QUEUE_INDEX:{
         byte item;
         ioByteBuffer_getByte( frame, &item );
         dcrudArguments_setQueue( args, item );
      }break;
      case dcrudTYPE_SHAREABLE:{
         dcrudShareable item = ParticipantImpl_newInstance( This->participant, frame );
         dcrudArguments_putShareable( args, name, item );
      }break;
      default:
         fprintf( stderr, "%s:%d:Unexpected class ID: %d\n", __FILE__, __LINE__, (int)type );
      break;
      }
   }
   if( callId >= 0 ) {
      dcrudIDispatcher_execute( This->participant->dispatcher,
         intrfcName, opName, args, callId, queueNdx, callMode );
   }
   else if( callId < 0 ) {
      ParticipantImpl_callback( This->participant, intrfcName, opName, args, -callId );
   }
}

static void * run( NetworkReceiver * This ) {
   char     signa[DCRUD_SIGNATURE_SIZE];
   uint64_t atStart = 0;

   while( true ) {
      if( atStart > 0 ) {
         dbgPerformance_record( "network", osSystem_nanotime() - atStart );
      }
      if( IO_STATUS_NO_ERROR == ioByteBuffer_receive( This->inBuf, This->in )) {
         atStart = osSystem_nanotime();
         ioByteBuffer_flip( This->inBuf );
         /**/
         ioByteBuffer_dump( This->inBuf, stderr );
         /**/
         ioByteBuffer_get( This->inBuf, (byte *)signa, 0, sizeof( signa ));
         if( 0 == strncmp( signa, (const char *)DCRUD_SIGNATURE, DCRUD_SIGNATURE_SIZE )) {
            FrameType    frameType = FRAMETYPE_NO_OP;
            unsigned int ignored = 0;
            ioByteBuffer_getByte( This->inBuf, (byte*)&frameType );
            switch( frameType ) {
            case FRAMETYPE_DATA_CREATE_OR_UPDATE: dataUpdate( This, This->inBuf ); break;
            case FRAMETYPE_DATA_DELETE          : dataDelete( This, This->inBuf ); break;
            case FRAMETYPE_OPERATION            : operation ( This, This->inBuf ); break;
            default:
               fprintf( stderr, "%s:%d:%d isn't a valid FrameType\n",
                  __FILE__, __LINE__, frameType );
               break;
            }
            ignored = ioByteBuffer_remaining( This->inBuf );
            if( ignored != 0 ) {
               fprintf( stderr, "%s:%d:%d received byte%s ignored\n",
                  __FILE__, __LINE__, ignored, ignored > 1 ? "s": "" );
            }
         }
         else {
            fprintf( stderr, "%s:%d:Garbage received, %d bytes discarded!\n",
               __FILE__, __LINE__, ioByteBuffer_getLimit( This->inBuf ));
         }
      }
      else {
         break;
      }
   }
   return NULL;
}

typedef void * ( * pthread_routine_t )( void * );

struct NetworkReceiver_s * createNetworkReceiver(
   ParticipantImpl * participant,
   const char *      address,
   unsigned short    port,
   const char *      intrfc )
{
   NetworkReceiver *  This = (NetworkReceiver *)malloc( sizeof( NetworkReceiver ));
   int                trueValue = 1;
   struct sockaddr_in local_sin;
   struct ip_mreq     mreq;

   memset( &local_sin, 0, sizeof( local_sin ));
   memset( &mreq     , 0, sizeof( mreq ));
   memset( This, 0, sizeof( NetworkReceiver ));
   This->participant = participant;
   This->inBuf       = ioByteBuffer_new( 64*1024 );
   This->in          = socket( AF_INET, SOCK_DGRAM, 0 );
   if( ! utilCheckSysCall( This->in != INVALID_SOCKET, __FILE__, __LINE__, "socket" )) {
      return This;
   }
   if( ! utilCheckSysCall( 0 ==
      setsockopt( This->in, SOL_SOCKET, SO_REUSEADDR, (char*)&trueValue, sizeof( trueValue )),
      __FILE__, __LINE__, "setsockopt(SO_REUSEADDR)" ))
   {
      return This;
   }
   local_sin.sin_family      = AF_INET;
   local_sin.sin_port        = htons( port );
   local_sin.sin_addr.s_addr = htonl( INADDR_ANY );
   if( ! utilCheckSysCall( 0 ==
      bind( This->in, (struct sockaddr *)&local_sin, sizeof( local_sin )),
      __FILE__, __LINE__, "bind(%s,%d)", intrfc, port ))
   {
      return This;
   }
   mreq.imr_multiaddr.s_addr = inet_addr( address );
   mreq.imr_interface.s_addr = inet_addr( intrfc );
   if( ! utilCheckSysCall( 0 ==
      setsockopt( This->in, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof( mreq )),
      __FILE__, __LINE__, "setsockopt(IP_ADD_MEMBERSHIP,%s)", address ))
   {
      return This;
   }
   printf( "Receiving from %s, bound to %s:%d\n", address, intrfc, port );
#ifdef WIN32
   DWORD tid;
   This->thread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)run, This, 0, &tid );
   if( ! utilCheckSysCall( This->thread != NULL, __FILE__, __LINE__, "CreateThread" ))
#else
   if( ! utilCheckSysCall( 0 ==
      pthread_create( &This->thread, NULL, (pthread_routine_t)run, This ),
      __FILE__, __LINE__, "pthread_create" ))
#endif
   {
      return This;
   }
   return This;
}

void deleteNetworkReceiver( struct NetworkReceiver_s * This ) {
   void * retVal = NULL;

#ifdef WIN32
   WSACancelBlockingCall();
   WaitForSingleObject( This->thread, INFINITE );
   closesocket( This->in );
#else
   pthread_cancel( This->thread ); /* break ioByteBuffer_receive and cause the thread to exit */
   pthread_join( This->thread, &retVal );
   close( This->in );
#endif
   ioByteBuffer_delete( &This->inBuf );
   free( This );
}
