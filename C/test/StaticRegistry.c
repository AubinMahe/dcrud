#include "StaticRegistry.h"
#include "Settings.h"

#include <coll/Map.h>
#include <string.h>

typedef struct Context_s {

   collSet participants;
   collMap clients;
   collMap consumers;

} Context;

static collSet getParticipants( Context * userContext ) {
   return userContext->participants;
}

static collSet getClients( Context * userContext, const char * interfaceName ) {
   collSet clients = (collSet)collMap_get( userContext->clients, (collMapKey)interfaceName );
   return clients;
}

static collSet getConsumers( Context * userContext, const char * dataName ) {
   collSet consumers = (collSet)collMap_get( userContext->consumers, (collMapKey)dataName );
   return consumers;
}

dcrudIRegistry getStaticRegistry( void ) {
   static Context        context;
   static dcrudIRegistry registry;
   if( ! registry ) {
      context.participants = collSet_new((collComparator)ioInetSocketAddress_comparator );
      context.clients      = collMap_new((collComparator)strcmp );
      context.consumers    = collMap_new((collComparator)strcmp );
      collSet_add( context.participants, ioInetSocketAddress_new( MCAST_ADDRESS, 2416 ));
      collSet_add( context.participants, ioInetSocketAddress_new( MCAST_ADDRESS, 2417 ));
      collSet_add( context.participants, ioInetSocketAddress_new( MCAST_ADDRESS, 2418 ));
      collSet_add( context.participants, ioInetSocketAddress_new( MCAST_ADDRESS, 2419 ));
      registry =
         dcrudIRegistry_new(
            &context,
            (dcrudIRegistry_getParticipants_t)getParticipants,
            (dcrudIRegistry_getClients_t     )getClients,
            (dcrudIRegistry_getConsumers_t   )getConsumers );
   }
   return registry;
}
