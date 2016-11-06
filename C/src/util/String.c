#include <util/String.h>
#include <util/Pool.h>

#include "poolSizes.h"

typedef char utilString_20[20];
UTIL_POOL_DECLARE( utilString_20 )

typedef char utilString_40[40];
UTIL_POOL_DECLARE( utilString_40 )

typedef char utilString_80[80];
UTIL_POOL_DECLARE( utilString_80 )

typedef char utilString_160[160];
UTIL_POOL_DECLARE( utilString_160 )

typedef char utilString_1000[1000];
UTIL_POOL_DECLARE( utilString_1000 )

typedef char utilString_5000[5000];
UTIL_POOL_DECLARE( utilString_5000 )

typedef char utilString_10000[10000];
UTIL_POOL_DECLARE( utilString_10000 )

typedef char utilString_65536[65536];
UTIL_POOL_DECLARE( utilString_65536 )

#include <string.h>

#ifdef STATIC_ALLOCATION
#  define RESERVE(T)\
   UTIL_POOL_INIT( T )\
   status = utilPool_reserve( &T ## Pool, This );\
   if( UTIL_STATUS_NO_ERROR == status ) {\
      memset( *This, 0, sizeof( T ));\
   }
#endif

utilStatus utilString_new( char ** This, size_t size ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( This == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
#ifdef STATIC_ALLOCATION
      if( size < 20 ) {
         RESERVE( utilString_20 )
      }
      else if( size < 40 ) {
         RESERVE( utilString_40 )
      }
      else if( size < 80 ) {
         RESERVE( utilString_80 )
      }
      else if( size < 160 ) {
         RESERVE( utilString_160 )
      }
      else if( size < 1000 ) {
         RESERVE( utilString_1000 )
      }
      else if( size < 5000 ) {
         RESERVE( utilString_5000 )
      }
      else if( size < 10000 ) {
         RESERVE( utilString_10000 )
      }
      else if( size < 65536 ) {
         RESERVE( utilString_65536 )
      }
      else {
         status = UTIL_STATUS_OVERFLOW;
      }
#else
      *This = (char *)malloc( size );
      if( NULL == *This ) {
         status = UTIL_STATUS_TOO_MANY;
      }
#endif
   }
   return status;
}

utilStatus utilString_dup( char ** This, const char * source ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if(( This == NULL )||( source == NULL )) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      size_t len = strlen( source ) + 1;
      CHK(__FILE__,__LINE__,utilString_new( This, len ))
      strcpy( *This, source );
   }
   return status;
}

utilStatus utilString_concat( char * This, const char * source, size_t * size ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if(( NULL != This )&&( NULL != source )&&( NULL != size )) {
      size_t len = strlen( source );
      if( len < *size ) {
         strcat( This, source );
         *size -= len;
      }
      else {
         status = UTIL_STATUS_OVERFLOW;
      }
   }
   else {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   return status;
}

utilStatus utilString_delete( char ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
#ifdef STATIC_ALLOCATION
   status = utilPool_release( &utilString_20Pool, self );
   if( status == UTIL_STATUS_NO_ERROR ) return status;
   status = utilPool_release( &utilString_40Pool, self );
   if( status == UTIL_STATUS_NO_ERROR ) return status;
   status = utilPool_release( &utilString_80Pool, self );
   if( status == UTIL_STATUS_NO_ERROR ) return status;
   status = utilPool_release( &utilString_160Pool, self );
   if( status == UTIL_STATUS_NO_ERROR ) return status;
   status = utilPool_release( &utilString_1000Pool, self );
   if( status == UTIL_STATUS_NO_ERROR ) return status;
   status = utilPool_release( &utilString_5000Pool, self );
   if( status == UTIL_STATUS_NO_ERROR ) return status;
   status = utilPool_release( &utilString_10000Pool, self );
   if( status == UTIL_STATUS_NO_ERROR ) return status;
   status = utilPool_release( &utilString_65536Pool, self );
   if( status == UTIL_STATUS_NO_ERROR ) return status;
#else
   free( *self );
   *self = NULL;
#endif
   return status;
}
