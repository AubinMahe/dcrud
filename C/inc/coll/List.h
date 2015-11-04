#pragma once
#include <util/types.h>
#include "ForeachFunction.h"

CREATE_ADT( collList );

typedef void * collListItem;

collList          collList_reserve();
void              collList_clear  ( collList   This );
void              collList_add    ( collList   This, collListItem item );
bool              collList_remove ( collList   This, collListItem item );
unsigned int      collList_size   ( collList   This );
collForeachResult collList_foreach( collList   This, collForeachFunction fn, void * userData );
void              collList_release( collList * This );
