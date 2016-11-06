#include "Shareable_private.h"
#include "magic.h"
#include "poolSizes.h"
#include "GUID_private.h"
#include "ParticipantImpl.h"

#include <util/Pool.h>

#include <string.h>

UTIL_DEFINE_SAFE_CAST( dcrudShareable     )
UTIL_POOL_DECLARE    ( dcrudShareableImpl )

utilStatus dcrudShareable_new(
   dcrudShareable *    self,
   dcrudClassID        classID,
   dcrudLocalFactory * factory,
   dcrudShareableData  data    )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self || NULL == factory ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudShareableImpl * This = NULL;
      UTIL_ALLOCATE_ADT( dcrudShareable, self, This );
      if( UTIL_STATUS_NO_ERROR == status ) {
         CHK(__FILE__,__LINE__,dcrudGUID_new( &This->id ))
         This->classID = classID;
         This->factory = factory;
         This->data    = data;
      }
   }
   return status;
}

utilStatus dcrudShareable_delete( dcrudShareable * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == self ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudShareableImpl * This = dcrudShareable_safeCast( *self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         dcrudGUID_delete( &This->id );
         if( This->data ) {
            This->factory->releaseUserData( &This->data );
         }
         UTIL_RELEASE( dcrudShareableImpl );
      }
   }
   return status;
}

utilStatus dcrudShareable_getGUID( dcrudShareable self, dcrudGUID * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == result ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudShareableImpl * This = dcrudShareable_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         *result = This->id;
      }
   }
   return status;
}

utilStatus dcrudShareable_getClassID( dcrudShareable self, dcrudClassID * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == result ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudShareableImpl * This = dcrudShareable_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         *result = This->classID;
      }
   }
   return status;
}

int dcrudShareable_compareTo( dcrudShareable * left, dcrudShareable * right ) {
   return (int)( *left - *right );
}

utilStatus dcrudShareable_getData( dcrudShareable self, dcrudShareableData * result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( NULL == result ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudShareableImpl * This = dcrudShareable_safeCast( self, &status );
      if( UTIL_STATUS_NO_ERROR == status ) {
         *result = This->data;
      }
   }
   return status;
}

utilStatus dcrudShareable_detach( dcrudShareable self ) {
   utilStatus           status = UTIL_STATUS_NO_ERROR;
   dcrudShareableImpl * This   = dcrudShareable_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      This->data = NULL;
   }
   return status;
}
