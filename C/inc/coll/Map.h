#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "Comparator.h"
#include "ForeachFunction.h"
#include <util/Status.h>

UTIL_ADT( collMap );

typedef const void * collMapKey;
typedef void * collMapValue;
typedef struct collMapPair_s {
   collMapKey   key;
   collMapValue value;
} collMapPair;
typedef collMapPair * collMapValues;

utilStatus collMap_new    ( collMap * This, collComparator cmp );
utilStatus collMap_delete ( collMap * This );
utilStatus collMap_clear  ( collMap   This );
utilStatus collMap_put    ( collMap   This, collMapKey key, collMapValue value, collMapPair * prevPair );
utilStatus collMap_remove ( collMap   This, collMapKey key, collMapPair * previousPair );
utilStatus collMap_get    ( collMap   This, collMapKey key, void * result );
utilStatus collMap_size   ( collMap   This, unsigned int * size );
utilStatus collMap_hasKey ( collMap   This, collMapKey key, bool * result );
utilStatus collMap_foreach( collMap   This, collForeachFunction fn, void * userData );
utilStatus collMap_values ( collMap   This, collMapValues * result );

#ifdef __cplusplus
}
#endif
