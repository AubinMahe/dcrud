#include "StaticRegistry.h"
#include "Settings.h"

#include <coll/Map.h>
#include <io/InetSocketAddress.h>

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
   collSet clients = NULL;
   collMap_get( userContext->clients, (collMapKey)interfaceName, &clients );
   return clients;
}

static collSet getConsumers( Context * userContext, const char * dataName ) {
   collSet consumers = NULL;
   collMap_get( userContext->consumers, (collMapKey)dataName, &consumers );
   return consumers;
}

static Context        context;
static dcrudIRegistry registry;

utilStatus getStaticRegistry( dcrudIRegistry * target ) {
   if( ! registry ) {
      static ioInetSocketAddress ad2416;
      static ioInetSocketAddress ad2417;
      static ioInetSocketAddress ad2418;
      static ioInetSocketAddress ad2419;

      CHK(__FILE__,__LINE__,ioInetSocketAddress_init( &ad2416, MCAST_ADDRESS, 2416 ));
      CHK(__FILE__,__LINE__,ioInetSocketAddress_init( &ad2417, MCAST_ADDRESS, 2417 ));
      CHK(__FILE__,__LINE__,ioInetSocketAddress_init( &ad2418, MCAST_ADDRESS, 2418 ));
      CHK(__FILE__,__LINE__,ioInetSocketAddress_init( &ad2419, MCAST_ADDRESS, 2419 ));
      CHK(__FILE__,__LINE__,collSet_new( &context.participants, (collComparator)ioInetSocketAddress_comparator ));
      CHK(__FILE__,__LINE__,collMap_new( &context.clients     , (collComparator)strcmp ));
      CHK(__FILE__,__LINE__,collMap_new( &context.consumers   , (collComparator)strcmp ));
      CHK(__FILE__,__LINE__,collSet_add( context.participants, &ad2416 ));
      CHK(__FILE__,__LINE__,collSet_add( context.participants, &ad2417 ));
      CHK(__FILE__,__LINE__,collSet_add( context.participants, &ad2418 ));
      CHK(__FILE__,__LINE__,collSet_add( context.participants, &ad2419 ));
      CHK(__FILE__,__LINE__,dcrudIRegistry_new(
         &registry,
         &context,
         (dcrudIRegistry_getParticipants_t)getParticipants,
         (dcrudIRegistry_getClients_t     )getClients,
         (dcrudIRegistry_getConsumers_t   )getConsumers ));
   }
   *target = registry;
   return UTIL_STATUS_NO_ERROR;
}

utilStatus releaseStaticRegistry( dcrudIRegistry * registry ) {
   collSet_delete( &context.participants );
   collMap_delete( &context.clients      );
   collMap_delete( &context.consumers    );
   dcrudIRegistry_delete( registry );
   return UTIL_STATUS_NO_ERROR;
}
