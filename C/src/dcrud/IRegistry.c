#include <dcrud/IRegistry.h>
#include <util/Pool.h>
#include "magic.h"
#include "poolSizes.h"

typedef struct IRegistry_s {

   unsigned                         magic;
   void *                           userContext;
   dcrudIRegistry_getParticipants_t getParticipants;
   dcrudIRegistry_getClients_t      getClients;
   dcrudIRegistry_getConsumers_t    getConsumers;

} dcrudIRegistryImpl;

UTIL_DEFINE_SAFE_CAST( dcrudIRegistry     )
UTIL_POOL_DECLARE    ( dcrudIRegistryImpl )

utilStatus dcrudIRegistry_new(
   dcrudIRegistry *                 self,
   void *                           userContext,
   dcrudIRegistry_getParticipants_t getParticipants,
   dcrudIRegistry_getClients_t      getClients,
   dcrudIRegistry_getConsumers_t    getConsumers )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIRegistryImpl * This = NULL;
      UTIL_ALLOCATE_ADT( dcrudIRegistry, self, This );
      if( UTIL_STATUS_NO_ERROR == status ) {
         This->magic           = dcrudIRegistryImplMAGIC;
         This->userContext     = userContext;
         This->getParticipants = getParticipants;
         This->getClients      = getClients;
         This->getConsumers    = getConsumers;
      }
   }
   return status;
}

utilStatus dcrudIRegistry_delete( dcrudIRegistry * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      UTIL_RELEASE( dcrudIRegistryImpl )
   }
   return status;
}

utilStatus dcrudIRegistry_getParticipants( dcrudIRegistry self, collSet * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   dcrudIRegistryImpl * This = dcrudIRegistry_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      if( target == NULL ) {
         status = UTIL_STATUS_NULL_ARGUMENT;
      }
      else {
         *target = This->getParticipants( This->userContext );
      }
   }
   return status;
}

utilStatus dcrudIRegistry_getClients( dcrudIRegistry self, const char * interfaceName, collSet * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   dcrudIRegistryImpl * This = dcrudIRegistry_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      if( target == NULL ) {
         status = UTIL_STATUS_NULL_ARGUMENT;
      }
      else {
         *target = This->getClients( This->userContext, interfaceName );
      }
   }
   return status;
}

utilStatus dcrudIRegistry_getConsumers( dcrudIRegistry self, const char * dataName, collSet * target ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   dcrudIRegistryImpl * This = dcrudIRegistry_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      if( target == NULL ) {
         status = UTIL_STATUS_NULL_ARGUMENT;
      }
      else {
         *target = This->getConsumers( This->userContext, dataName );
      }
   }
   return status;
}
