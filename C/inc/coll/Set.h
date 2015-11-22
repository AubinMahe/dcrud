#pragma once
#include <util/types.h>
#include <coll/Comparator.h>
#include "ForeachFunction.h"

UTIL_ADT( collSet );

typedef void * collSetItem;

collSet           collSet_new    ( collComparator cmp );
void              collSet_delete ( collSet * This );
void              collSet_clear  ( collSet   This );
bool              collSet_add    ( collSet   This, collSetItem item );
bool              collSet_remove ( collSet   This, collSetItem item );
unsigned int      collSet_size   ( collSet   This );
collForeachResult collSet_foreach( collSet   This, collForeachFunction fn, void * userData );
