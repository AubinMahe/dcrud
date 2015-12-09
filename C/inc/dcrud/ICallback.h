#pragma once
#include <coll/Map.h>

UTIL_ADT( dcrudICallback );

typedef void (* dcrudICallback_function)(
   dcrudICallback This,
   const char *   intrfc,
   const char *   operation,
   collMap        results   );

dcrudICallback dcrudICallback_new( dcrudICallback_function callback, void * userData );
void   dcrudICallback_delete     ( dcrudICallback * This );
void   dcrudICallback_callback   ( dcrudICallback   This, const char * intrfc, const char * operation, collMap results );
void * dcrudICallback_getUserData( dcrudICallback   This );
