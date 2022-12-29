#include "ISO8601Date.h"

#include <util/Pool.h>
#include <util/iso8601.h>
#include <util/String.h>

#include <stdlib.h>
#include <string.h>

#include "poolSizes.h"

UTIL_POOL_DECLARE( channelTestISO8601Date )

static int getClassID() {
   return 1;
}

static channelFactory TheFactory;

utilStatus channelTestISO8601Date_registerFactory( void ) {
   TheFactory.getClassID = (channelGetClassID)getClassID;
   TheFactory.create     = (channelCreate    )channelTestISO8601Date_new;
   TheFactory.encode     = (channelEncode    )channelTestISO8601Date_encode;
   TheFactory.decode     = (channelDecode    )channelTestISO8601Date_decode;
   return channelFactories_add( &TheFactory );
}

utilStatus channelTestISO8601Date_getFactory( channelFactory ** This ) {
   if( This ) {
      *This = &TheFactory;
      return UTIL_STATUS_NO_ERROR;
   }
   return UTIL_STATUS_NULL_ARGUMENT;
}

utilStatus channelTestISO8601Date_new( channelTestISO8601Date ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self ) {
      channelTestISO8601Date * This = *self;
      UTIL_ALLOCATE( channelTestISO8601Date );
      if( UTIL_STATUS_NO_ERROR == status ) {
         status = channelTestISO8601Date_init( This );
      }
   }
   else {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   return status;
}

utilStatus channelTestISO8601Date_encode( const channelTestISO8601Date * This, ioByteBuffer encoder ) {
   char buffer[100];
   CHK(__FILE__,__LINE__,channelTestISO8601Date_format( This, buffer, sizeof( buffer )));
   CHK(__FILE__,__LINE__,ioByteBuffer_putString( encoder, buffer ));
   return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestISO8601Date_decode( channelTestISO8601Date * This, ioByteBuffer decoder ) {
   char buffer[100];
   CHK(__FILE__,__LINE__,ioByteBuffer_getString( decoder, buffer, sizeof( buffer )));
   CHK(__FILE__,__LINE__,channelTestISO8601Date_parse( This, buffer ));
   return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestISO8601Date_init( channelTestISO8601Date * This ) {
   if( NULL == This ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   memset( This, 0, sizeof( channelTestISO8601Date ));
   This->factory = &TheFactory;
   return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestISO8601Date_isValid( const channelTestISO8601Date * This ) {
   return ( This != NULL ) && ( This->day > 0 ) && ( This->month > 0 );
}

utilStatus channelTestISO8601Date_parse( channelTestISO8601Date * This, const char * src ) {
   struct tm tm;
   if(( NULL == This ) || ( NULL == src )) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   memset( &tm, 0, sizeof( tm ));
   if( strptime( src, "%F", &tm ) != NULL ) {
      This->day   = (byte)tm.tm_mday;
      This->month = (byte)tm.tm_mon;
      This->year  = (unsigned short)tm.tm_year;
   }
   return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestISO8601Date_format(
   const channelTestISO8601Date * This,
   char *                         target,
   size_t                         targetSize )
{
   struct tm tm;
   if(( NULL == This ) || ( NULL == target )) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   memset( &tm, 0, sizeof( tm ));
   tm.tm_mday = This->day;
   tm.tm_mon  = This->month;
   tm.tm_year = This->year;
   if( strftime( target, targetSize, "%F", &tm ) < 1 ) {
      return UTIL_STATUS_STD_API_ERROR;
   }
   return UTIL_STATUS_NO_ERROR;
}

utilStatus channelTestISO8601Date_toString(
   const channelTestISO8601Date * This,
   char *                         target,
   size_t *                       size )
{
   char s[80];
   if(( NULL == This ) || ( NULL == target ) || ( NULL == size )) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   CHK(__FILE__,__LINE__,channelTestISO8601Date_format( This, s, sizeof( s )));
   CHK(__FILE__,__LINE__,utilString_concat( s, target, size ));
   return UTIL_STATUS_NO_ERROR;
}
