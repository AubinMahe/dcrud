#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "Comparator.h"
#include "ForeachFunction.h"

UTIL_ADT( collList );

typedef void *         collListItem;
typedef collListItem * collListValues;

utilStatus collList_new     ( collList * This );
utilStatus collList_delete  ( collList * This );
utilStatus collList_clear   ( collList   This );
utilStatus collList_add     ( collList   This, collListItem item );
utilStatus collList_remove  ( collList   This, collListItem item );
utilStatus collList_get     ( collList   This, unsigned int index, collListItem * result );
utilStatus collList_size    ( collList   This, unsigned int * result );
utilStatus collList_foreach ( collList   This, collForeachFunction fn, void * userData );
utilStatus collList_indexOf ( collList   This, collListItem item, collComparator cmp, unsigned * result );
utilStatus collList_contains( collList   This, collListItem item, collComparator cmp, bool * result );
utilStatus collList_values  ( collList   This, collListValues * result );

#ifdef __cplusplus
}
#endif
