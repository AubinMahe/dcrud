#pragma once
#include <coll/MapVoidPtr.h>

UTIL_ADT( dcrudICallback );

void dcrudICallback_callback(
   const char *   intrfc,
   const char *   operation,
   collMapVoidPtr results );
