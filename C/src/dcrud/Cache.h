#pragma once

#include <dcrud/IRepositoryFactory.h>

dcrudIRepository * dcrudCache_init(
   int                       sourceId,
   bool                      producer,
   dcrudShareableFactory     factory,
   dcrudIRepositoryFactory * network );
void dcrudCache_updateFromNetwork( void * item );
void dcrudCache_deleteFromNetwork( dcrudGUID id );
