#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>

UTIL_ADT( dcrudICallback );

struct dcrudArguments_s;

typedef void (* dcrudICallback_function)(
   dcrudICallback            This,
   const char *              intrfc,
   const char *              operation,
   struct dcrudArguments_s * results   );

utilStatus dcrudICallback_new        ( dcrudICallback * This, dcrudICallback_function callback, void * userData );
utilStatus dcrudICallback_delete     ( dcrudICallback * This );
utilStatus dcrudICallback_callback   ( dcrudICallback   This, const char * intrfc, const char * operation, struct dcrudArguments_s * results );
utilStatus dcrudICallback_getUserData( dcrudICallback   This, void ** userData );

#ifdef __cplusplus
}
#endif
