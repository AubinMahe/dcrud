#include <dcrud/ICallback.h>
#include <dcrud/Arguments.h>
#include <util/Pool.h>
#include "magic.h"
#include "poolSizes.h"

#include <string.h>

typedef struct dcrudICallbackImpl_s {

   unsigned int            magic;
   dcrudICallback_function callback;
   void *                  userData;

} dcrudICallbackImpl;

UTIL_DEFINE_SAFE_CAST( dcrudICallback     )
UTIL_POOL_DECLARE    ( dcrudICallbackImpl )

utilStatus dcrudICallback_new(
   dcrudICallback *        self,
   dcrudICallback_function callback,
   void *                  userData )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudICallbackImpl * This = NULL;
      UTIL_ALLOCATE_ADT( dcrudICallback, self, This );
      if( UTIL_STATUS_NO_ERROR == status ) {
         This->callback = callback;
         This->userData = userData;
      }
   }
   return status;
}

utilStatus dcrudICallback_delete( dcrudICallback * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      UTIL_RELEASE( dcrudICallbackImpl );
   }
   return status;
}

utilStatus dcrudICallback_callback(
   dcrudICallback self,
   const char *   intrfc,
   const char *   operation,
   dcrudArguments results   )
{
   utilStatus           status = UTIL_STATUS_NO_ERROR;
   dcrudICallbackImpl * This   = dcrudICallback_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      This->callback( self, intrfc, operation, results );
   }
   return status;
}

utilStatus dcrudICallback_getUserData( dcrudICallback self, void ** result ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudICallbackImpl * This = dcrudICallback_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         *result = This->userData;
      }
   }
   return status;
}
