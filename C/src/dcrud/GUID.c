#include "GUID_private.h"
#include "magic.h"
#include "poolSizes.h"

#include <util/Pool.h>
#include <util/Trace.h>

#include <stdio.h>
#include <string.h>

UTIL_DEFINE_SAFE_CAST( dcrudGUID     )
UTIL_POOL_DECLARE    ( dcrudGUIDImpl )

utilStatus dcrudGUID_new( dcrudGUID * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudGUIDImpl * This = NULL;
      UTIL_ALLOCATE_ADT( dcrudGUID, self, This );
   }
   return status;
}

utilStatus dcrudGUID_delete( dcrudGUID * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   UTIL_RELEASE( dcrudGUIDImpl )
   return status;
}

utilStatus dcrudGUID_unserialize( dcrudGUID * self, ioByteBuffer source ) {
   utilStatus status = dcrudGUID_new( self );
   if( UTIL_STATUS_NO_ERROR == status ) {
      dcrudGUIDImpl * This = (dcrudGUIDImpl *)*self;
      /*     */ioByteBuffer_getUInt( source, &This->publisher );
      status = ioByteBuffer_getUInt( source, &This->instance );
   }
   return status;
}

utilStatus dcrudGUID_serialize( const dcrudGUID self, ioByteBuffer target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   dcrudGUIDImpl * This = dcrudGUID_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status  ) {
      status = ioByteBuffer_putUInt( target, This->publisher );
   }
   if( UTIL_STATUS_NO_ERROR == status  ) {
      status = ioByteBuffer_putUInt( target, (unsigned)This->instance );
   }
   return status;
}

utilStatus dcrudGUID_isShared( const dcrudGUID self, bool * shared ) {
   utilStatus     status = UTIL_STATUS_NO_ERROR;
   dcrudGUIDImpl * This   = dcrudGUID_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      if( shared == NULL ) {
         status = UTIL_STATUS_NULL_ARGUMENT;
      }
      else {
         *shared = This->instance > 0;
      }
   }
   return status;
}

utilStatus dcrudGUID_isOwnedBy( const dcrudGUID self, unsigned publisherId, bool * isOwnedBy ) {
   utilStatus     status = UTIL_STATUS_NO_ERROR;
   dcrudGUIDImpl * This   = dcrudGUID_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      if( isOwnedBy == NULL ) {
         status = UTIL_STATUS_NULL_ARGUMENT;
      }
      else {
         *isOwnedBy = This->publisher == publisherId;
      }
   }
   return status;
}

utilStatus dcrudGUID_set( dcrudGUID self, const dcrudGUID id ) {
   utilStatus     status = UTIL_STATUS_NO_ERROR;
   dcrudGUIDImpl * This   = dcrudGUID_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      dcrudGUIDImpl * right = dcrudGUID_safeCast( id, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         This->publisher = right->publisher;
         This->instance  = right->instance;
      }
   }
   return status;
}

utilStatus dcrudGUID_init( dcrudGUID self, unsigned int publisher, unsigned int instance ) {
   utilStatus     status = UTIL_STATUS_NO_ERROR;
   dcrudGUIDImpl * This   = dcrudGUID_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      This->publisher = publisher;
      This->instance  = instance;
   }
   return status;
}

utilStatus dcrudGUID_toString( const dcrudGUID self, char * target, size_t targetSize ) {
   utilStatus     status = UTIL_STATUS_NO_ERROR;
   dcrudGUIDImpl * This   = dcrudGUID_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      int count = snprintf( target, targetSize, "Instance-%08X-%08X",
         This->publisher, This->instance );
      target[targetSize-1] = '\0';
      if( count >= (int)targetSize ) {
         status = UTIL_STATUS_OVERFLOW;
      }
   }
   return status;
}

int dcrudGUID_compareTo( const dcrudGUID * l, const dcrudGUID * r ) {
   int              diff  = 0;
   dcrudGUIDImpl ** left  = (dcrudGUIDImpl **)l;
   dcrudGUIDImpl ** right = (dcrudGUIDImpl **)r;
   if( left && right
      && (*left )->magic == dcrudGUIDImplMAGIC
      && (*right)->magic == dcrudGUIDImplMAGIC )
   {
      if( diff == 0 ) {
         diff = (int)((*left)->publisher - (*right)->publisher );
      }
      if( diff == 0 ) {
         diff = (int)((*left)->instance  - (*right)->instance );
      }
   }
   return diff;
}
