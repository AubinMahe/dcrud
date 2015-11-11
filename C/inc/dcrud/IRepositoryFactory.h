#pragma once
#include <dcrud/Shareable.h>
#include <dcrud/IRepository.h>

typedef dcrudShareable ( * dcrudShareableFactory)( int classId );

DCRUD_ADT( dcrudIRepositoryFactory );

dcrudIRepository dcrudIRepositoryFactory_getRepository(
   dcrudIRepositoryFactory This,
   int                     classId,
   int                     owner,
   dcrudShareableFactory   factory );
