#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <coll/Set.h>

UTIL_ADT( dcrudIRegistry );

typedef collSet ( * dcrudIRegistry_getParticipants_t )( void * userContext );
typedef collSet ( * dcrudIRegistry_getClients_t      )( void * userContext, const char * interfaceName );
typedef collSet ( * dcrudIRegistry_getConsumers_t    )( void * userContext, const char * dataName );

utilStatus dcrudIRegistry_new            ( dcrudIRegistry * This,
   void * userContext,
   dcrudIRegistry_getParticipants_t getParticipants,
   dcrudIRegistry_getClients_t      getClients,
   dcrudIRegistry_getConsumers_t    getConsumers );
utilStatus dcrudIRegistry_delete         ( dcrudIRegistry * This );
utilStatus dcrudIRegistry_getParticipants( dcrudIRegistry   This, collSet * target );
utilStatus dcrudIRegistry_getClients     ( dcrudIRegistry   This, const char * interfaceName, collSet * target );
utilStatus dcrudIRegistry_getConsumers   ( dcrudIRegistry   This, const char * dataName, collSet * target );

#ifdef __cplusplus
}
#endif
