#include <dcrud/IRegistry.h>

typedef struct IRegistry_s {

   void *                           userContext;
   dcrudIRegistry_getParticipants_t getParticipants;
   dcrudIRegistry_getClients_t      getClients;
   dcrudIRegistry_getConsumers_t    getConsumers;

} IRegistry;

dcrudIRegistry dcrudIRegistry_new(
   void *                           userContext,
   dcrudIRegistry_getParticipants_t getParticipants,
   dcrudIRegistry_getClients_t      getClients,
   dcrudIRegistry_getConsumers_t    getConsumers )
{
   IRegistry * This = (IRegistry *)malloc( sizeof( IRegistry ));
   This->userContext     = userContext;
   This->getParticipants = getParticipants;
   This->getClients      = getClients;
   This->getConsumers    = getConsumers;
   return (dcrudIRegistry)This;
}

void dcrudIRegistry_delete( dcrudIRegistry * self ) {
   IRegistry * This = (IRegistry *)*self;
   free( This );
   *self = NULL;
}

collSet dcrudIRegistry_getParticipants( dcrudIRegistry self ) {
   IRegistry * This = (IRegistry *)self;
   return This->getParticipants( This->userContext );
}

collSet dcrudIRegistry_getClients( dcrudIRegistry self, const char * interfaceName ) {
   IRegistry * This = (IRegistry *)self;
   return This->getClients( This->userContext, interfaceName );
}

collSet dcrudIRegistry_getConsumers( dcrudIRegistry self, const char * dataName ) {
   IRegistry * This = (IRegistry *)self;
   return This->getConsumers( This->userContext, dataName );
}
