#pragma once
#include <dcrud/Shareable.h>
#include <dcrud/IRepository.h>

typedef dcrudShareable * ( * dcrudShareableFactory)( int classId );

typedef struct dcrudIRepositoryFactory_s {

   int unused;

} dcrudIRepositoryFactory;

dcrudIRepository * dcrudIRepositoryFactory_getRepository(
   dcrudIRepositoryFactory * This,
   int                       classId,
   int                       owner,
   dcrudShareableFactory     factory );
