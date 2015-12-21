#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/Arguments.h>

UTIL_ADT( dcrudICallback );

typedef void (* dcrudICallback_function)(
   dcrudICallback This,
   const char *   intrfc,
   const char *   operation,
   dcrudArguments results   );

dcrudICallback dcrudICallback_new        ( dcrudICallback_function callback, void * userData );
void           dcrudICallback_delete     ( dcrudICallback * This );
void           dcrudICallback_callback   ( dcrudICallback   This, const char * intrfc, const char * operation, dcrudArguments results );
void *         dcrudICallback_getUserData( dcrudICallback   This );

#ifdef __cplusplus
}
#endif
