#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "Shareable.h"

#include <coll/Set.h>

UTIL_ADT( dcrudICache );

typedef bool( * dcrudPredicate)( dcrudShareable item );

utilStatus dcrudICache_setOwnership( dcrudICache This, bool enabled );
utilStatus dcrudICache_owns        ( dcrudICache This, dcrudGUID id, bool * owns );
utilStatus dcrudICache_create      ( dcrudICache This, dcrudShareable item );
utilStatus dcrudICache_read        ( dcrudICache This, dcrudGUID id, dcrudShareable * target );
utilStatus dcrudICache_update      ( dcrudICache This, dcrudShareable item );
utilStatus dcrudICache_delete      ( dcrudICache This, dcrudShareable item );
utilStatus dcrudICache_foreach     ( dcrudICache This, collForeachFunction fn, void * userData, collForeachResult * result );
utilStatus dcrudICache_select      ( dcrudICache This, dcrudPredicate query, collSet result );
utilStatus dcrudICache_publish     ( dcrudICache This );
utilStatus dcrudICache_subscribe   ( dcrudICache This, dcrudClassID id );
utilStatus dcrudICache_refresh     ( dcrudICache This );

#ifdef __cplusplus
}
#endif
