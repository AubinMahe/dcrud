#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "Shareable.h"
#include "Status.h"

#include <coll/Set.h>

UTIL_ADT( dcrudICache );

typedef bool( * dcrudPredicate)( dcrudShareable item );

void              dcrudICache_setOwnership( dcrudICache This, bool enabled );
bool              dcrudICache_owns        ( dcrudICache This, dcrudGUID id );
dcrudStatus       dcrudICache_create      ( dcrudICache This, dcrudShareable item );
dcrudShareable    dcrudICache_read        ( dcrudICache This, dcrudGUID id );
dcrudStatus       dcrudICache_update      ( dcrudICache This, dcrudShareable item );
dcrudStatus       dcrudICache_delete      ( dcrudICache This, dcrudShareable item );
collForeachResult dcrudICache_foreach     ( dcrudICache This, collForeachFunction fn, void * userData );
collSet           dcrudICache_select      ( dcrudICache This, dcrudPredicate query );
dcrudStatus       dcrudICache_publish     ( dcrudICache This );
void              dcrudICache_subscribe   ( dcrudICache This, dcrudClassID id );
void              dcrudICache_refresh     ( dcrudICache This );

#ifdef __cplusplus
}
#endif
