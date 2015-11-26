#pragma once

UTIL_ADT( dcrudIProvided );

typedef void (* dcrudIOperation)( void * context, collMap in, collMap out );

bool dcrudIProvided_addOperation(
   dcrudIProvided  provided,
   const char *    name,
   void *          context,
   dcrudIOperation operation );
