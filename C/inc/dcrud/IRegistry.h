#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <io/InetSocketAddress.h>
#include <coll/Set.h>

UTIL_ADT( dcrudIRegistry );

typedef collSet ( * dcrudIRegistry_getParticipants_t )( void * userContext );
typedef collSet ( * dcrudIRegistry_getClients_t      )( void * userContext, const char * interfaceName );
typedef collSet ( * dcrudIRegistry_getConsumers_t    )( void * userContext, const char * dataName );

dcrudIRegistry dcrudIRegistry_new(
   void *                           userContext,
   dcrudIRegistry_getParticipants_t getParticipants,
   dcrudIRegistry_getClients_t      getClients,
   dcrudIRegistry_getConsumers_t    getConsumers );

void dcrudIRegistry_delete( dcrudIRegistry * This );

collSet dcrudIRegistry_getParticipants( dcrudIRegistry This );
collSet dcrudIRegistry_getClients     ( dcrudIRegistry This, const char * interfaceName );
collSet dcrudIRegistry_getConsumers   ( dcrudIRegistry This, const char * dataName );

#ifdef __cplusplus
}
#endif
