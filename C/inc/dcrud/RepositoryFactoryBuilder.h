#pragma once
#include <dcrud/IRepositoryFactory.h>

dcrudIRepositoryFactory * dcrudRepositoryFactoryBuilder_join(
   const char *   address,
   const char *   intrfc,
   unsigned short port    );
