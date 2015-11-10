#pragma once
#include <util/types.h>
#include <coll/Comparator.h>
#include "ForeachFunction.h"

DCRUD_ADT( collSet );

typedef void * collSetItem;

collSet           collSet_reserve( collComparator cmp );
void              collSet_clear  ( collSet   This );
bool              collSet_add    ( collSet   This, collSetItem item );
bool              collSet_remove ( collSet   This, collSetItem item );
unsigned          collSet_size   ( collSet   This );
collForeachResult collSet_foreach( collSet   This, collForeachFunction fn, void * userData );
void              collSet_release( collSet * This );
