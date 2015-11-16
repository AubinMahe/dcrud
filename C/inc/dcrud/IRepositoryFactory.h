#pragma once
#include <dcrud/Shareable.h>
#include <dcrud/IRepository.h>

typedef dcrudShareable ( * dcrudShareableFactory)( dcrudGUID id );

UTIL_ADT( dcrudIRepositoryFactory );

dcrudIRepository dcrudIRepositoryFactory_getRepository(
   dcrudIRepositoryFactory This,
   const char *            topic,
   dcrudShareableFactory   factory );
