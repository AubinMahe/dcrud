#include <channel/UDPChannel.h>

utilStatus channelFactories_done( void );

#include "poolSizes.h"
#include "magic.h"

#include <util/Pool.h>
#include <util/DebugSettings.h>

#include <os/threads.h>

#include <io/sockets.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct handler_s {

   short                msgId;
   channelMsgConsumerFn fn;
   void *               userData;

} handler_t;

typedef struct __ {

   unsigned     magic;
   int          sckt;
   ioByteBuffer decoder;
   osThread     thread;
   unsigned     maxHandlersCount;
   unsigned     handlersCount;
#ifdef STATIC_ALLOCATION
   handler_t    handlers[channelUDPChannelImplHandlersMaxCount];
#else
   handler_t *  handlers;
#endif

} channelUDPChannelImpl;

UTIL_DEFINE_SAFE_CAST( channelUDPChannel     )
UTIL_POOL_DECLARE    ( channelUDPChannelImpl )

static int handlerCmp( const void * l, const void * r ) {
   const handler_t * left  = (const handler_t *)l;
   const handler_t * right = (const handler_t *)r;
   return left->msgId - right->msgId;
}

static void * channelUDPChannel_run( void * self ) {
   channelUDPChannelImpl * This = (channelUDPChannelImpl *)self;
   utilStatus status;
   short msgId;
   for(;;) {
      status = ioByteBuffer_clear( This->decoder );
      if( UTIL_STATUS_NO_ERROR != status ) {
         utilStatus_checkAndLog( status, __FILE__, __LINE__, "ioByteBuffer_clear" );
         return NULL;
      }
      status = ioByteBuffer_receive( This->decoder, This->sckt );
      if( UTIL_STATUS_NO_ERROR != status ) {
         utilStatus_checkAndLog( status, __FILE__, __LINE__, "ioByteBuffer_receive" );
         return NULL;
      }
      status = ioByteBuffer_flip( This->decoder );
      if( UTIL_STATUS_NO_ERROR != status ) {
         utilStatus_checkAndLog( status, __FILE__, __LINE__, "ioByteBuffer_flip" );
         return NULL;
      }
      status = ioByteBuffer_getShort( This->decoder, &msgId );
      if( UTIL_STATUS_NO_ERROR != status ) {
         utilStatus_checkAndLog( status, __FILE__, __LINE__, "ioByteBuffer_getShort" );
      }
      else {
         handler_t   key     = { msgId, NULL, NULL };
         handler_t * handler = (handler_t *)
            bsearch( &key, This->handlers, This->handlersCount, sizeof( handler_t ), handlerCmp );
         if( utilDebugSettings->dumpReceivedBuffer ) {
            ioByteBuffer_dump( This->decoder, stderr );
         }
         if( handler ) {
            handler->fn( msgId, This->decoder, handler->userData );
         }
         else {
            fprintf( stderr, "Unexpected msg id: %d", msgId );
         }
      }
   }
   return NULL;
}

utilStatus channelUDPChannel_new(
   channelUDPChannel * self,
   const char *        intrfc,
   unsigned short int  local,
   const char *        targetHost,
   unsigned short int  remote      )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      struct hostent * he = gethostbyname( targetHost );
      if( he == NULL ) {
         status = UTIL_STATUS_STD_API_ERROR;
      }
      else {
         int sckt = socket( AF_INET, SOCK_DGRAM, 0 );
         if( sckt < 0 ) {
            status = UTIL_STATUS_STD_API_ERROR;
         }
         else {
            struct sockaddr_in localAddr;
            memset( &localAddr, 0, sizeof( localAddr ));
            localAddr.sin_family = AF_INET;
            inet_pton( AF_INET, intrfc, &localAddr.sin_addr.s_addr );
            localAddr.sin_port = htons( local );
            if( bind( sckt, (struct sockaddr *)&localAddr, sizeof( localAddr ))) {
               status = UTIL_STATUS_STD_API_ERROR;
               closesocket( sckt );
            }
            else {
               struct sockaddr_in target;
               target.sin_family = AF_INET;
               target.sin_port   = htons( remote );
               memcpy( &target.sin_addr, he->h_addr_list[0], (size_t)he->h_length );
               if( connect( sckt, (struct sockaddr *)&target, sizeof( target ))) {
                  status = UTIL_STATUS_STD_API_ERROR;
                  closesocket( sckt );
               }
               else {
                  channelUDPChannelImpl * This   = NULL;
                  UTIL_ALLOCATE_ADT( channelUDPChannel, self, This );
                  if( UTIL_STATUS_NO_ERROR == status ) {
                     This->sckt             = sckt;
                     This->maxHandlersCount = channelUDPChannelImplHandlersMaxCount;
                     status = ioByteBuffer_new( &This->decoder, 64*1024 );
                     if( UTIL_STATUS_NO_ERROR != status ) {
                        closesocket( sckt );
                        UTIL_RELEASE( channelUDPChannelImpl );
                     }
                     else {
#ifndef STATIC_ALLOCATION
                        size_t size = This->maxHandlersCount * sizeof( handler_t );
                        This->handlers = (handler_t *)malloc( size );
                        memset( This->handlers, 0, size );
#endif
                        if( ! osThread_create( &This->thread, channelUDPChannel_run, This )) {
                           status = UTIL_STATUS_STD_API_ERROR;
                           UTIL_RELEASE( channelUDPChannelImpl );
                        }
                     }
                  }
                  else {
                     closesocket( sckt );
                  }
               }
            }
         }
      }
   }
   return status;
}

utilStatus channelUDPChannel_send( channelUDPChannel self, ioByteBuffer encoder ) {
   utilStatus              status = UTIL_STATUS_NO_ERROR;
   channelUDPChannelImpl * This   = channelUDPChannel_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      size_t pos = 0U;
      CHK(__FILE__,__LINE__,ioByteBuffer_getPosition( encoder, &pos ));
      if( pos > 0U ) {
         CHK(__FILE__,__LINE__,ioByteBuffer_flip( encoder ));
      }
      CHK(__FILE__,__LINE__,ioByteBuffer_send( encoder, This->sckt ))
   }
   return status;
}

utilStatus channelUDPChannel_addHandler(
   channelUDPChannel    self,
   short                msgId,
   channelMsgConsumerFn handler,
   void *               userData )
{
   utilStatus              status = UTIL_STATUS_NO_ERROR;
   channelUDPChannelImpl * This   = channelUDPChannel_safeCast( self, &status );
   if(( status == UTIL_STATUS_NO_ERROR )&&( This->handlersCount == This->maxHandlersCount )) {
#ifdef STATIC_ALLOCATION
      status = UTIL_STATUS_TOO_MANY;
#else
      size_t      size = ( This->maxHandlersCount + 100 ) * sizeof( handler_t );
      handler_t * ra   = (handler_t *)realloc( This->handlers, size );
      if( ra != NULL ) {
         This->handlers          = ra;
         This->maxHandlersCount += 100;
      }
      else {
         status = UTIL_STATUS_TOO_MANY;
      }
#endif
   }
   if( status == UTIL_STATUS_NO_ERROR ) {
      handler_t * cell = This->handlers + This->handlersCount;
      ++This->handlersCount;
      cell->msgId    = msgId;
      cell->fn       = handler;
      cell->userData = userData;
      qsort( This->handlers, This->handlersCount, sizeof( handler_t ), handlerCmp );
   }
   return status;
}

utilStatus channelUDPChannel_delete( channelUDPChannel * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      channelUDPChannelImpl * This = channelUDPChannel_safeCast( *self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         shutdown( This->sckt, SHUT_RD );
         osThread_join( This->thread );
         closesocket( This->sckt );
         ioByteBuffer_delete( &This->decoder );
#ifndef STATIC_ALLOCATION
         free( This->handlers );
#endif
         UTIL_RELEASE( channelUDPChannelImpl )
         CHK(__FILE__,__LINE__,channelFactories_done())
      }
   }
   return status;
}
