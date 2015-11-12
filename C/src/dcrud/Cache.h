#pragma once

#include <dcrud/IRepositoryFactory.h>

dcrudIRepository dcrudCache_init(
   const char *            topic,
   dcrudShareableFactory   factory,
   dcrudIRepositoryFactory network );

void dcrudCache_updateFromNetwork( dcrudShareable item );

void dcrudCache_deleteFromNetwork( dcrudGUID id );
