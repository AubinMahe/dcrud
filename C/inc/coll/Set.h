#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "Comparator.h"
#include "ForeachFunction.h"

#include <util/Status.h>

UTIL_ADT( collSet );

typedef void *        collSetItem;
typedef collSetItem * collSetValues;

utilStatus collSet_new     ( collSet * This, collComparator cmp );
utilStatus collSet_delete  ( collSet * This );
utilStatus collSet_clear   ( collSet   This );
utilStatus collSet_add     ( collSet   This, collSetItem item );
utilStatus collSet_remove  ( collSet   This, collSetItem item );
utilStatus collSet_size    ( collSet   This, unsigned int * size );
utilStatus collSet_contains( collSet   This, collSetItem item, bool * result );
utilStatus collSet_foreach ( collSet   This, collForeachFunction fn, void * userData, collForeachResult * result );
utilStatus collSet_values  ( collSet   This, collSetValues * result );

#ifdef __cplusplus
}
#endif
