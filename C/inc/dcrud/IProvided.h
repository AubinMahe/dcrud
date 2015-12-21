#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/Arguments.h>

UTIL_ADT( dcrudIProvided );

typedef dcrudArguments (* dcrudIOperation)( void * context, dcrudArguments args );

bool dcrudIProvided_addOperation(
   dcrudIProvided  provided,
   const char *    name,
   void *          context,
   dcrudIOperation operation );

#ifdef __cplusplus
}
#endif
