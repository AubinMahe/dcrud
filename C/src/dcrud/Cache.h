#pragma once

#include <dcrud/IRepositoryFactory.h>

dcrudIRepository dcrudCache_init(
   int                     sourceId,
   bool                    producer,
   dcrudShareableFactory   factory,
   dcrudIRepositoryFactory network );
void dcrudCache_updateFromNetwork( dcrudShareable item );
void dcrudCache_deleteFromNetwork( dcrudGUID id );
