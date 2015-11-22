#pragma once
#include <util/types.h>
#include "ForeachFunction.h"

UTIL_ADT( collList );

typedef void * collListItem;

collList          collList_new    ( void );
void              collList_delete ( collList * This );
void              collList_clear  ( collList   This );
void              collList_add    ( collList   This, collListItem item );
bool              collList_remove ( collList   This, collListItem item );
collListItem      collList_get    ( collList   This, unsigned int index );
unsigned int      collList_size   ( collList   This );
collForeachResult collList_foreach( collList   This, collForeachFunction fn, void * userData );
