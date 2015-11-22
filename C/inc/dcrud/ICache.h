#pragma once
#include <coll/MapVoidPtr.h>
#include <dcrud/Shareable.h>
#include <dcrud/Status.h>

UTIL_ADT( dcrudICache );

byte             dcrudICache_getId();
void             dcrudICache_setOwnership( dcrudICache This, bool enabled );
bool             dcrudICache_owns        ( dcrudICache This, dcrudGUID id );
dcrudStatus      dcrudICache_create      ( dcrudICache This, dcrudShareable item );
dcrudShareable * dcrudICache_read        ( dcrudICache This, dcrudGUID id );
dcrudStatus      dcrudICache_update      ( dcrudICache This, dcrudShareable item );
dcrudStatus      dcrudICache_delete      ( dcrudICache This, dcrudShareable item );
collMapVoidPtr   dcrudICache_getAll      ( dcrudICache This );
/*collSet          dcrudICache_select      ( dcrudICache This, dcrudPredicate query );*/
bool             dcrudICache_isProducer  ( dcrudICache This );
bool             dcrudICache_isConsumer  ( dcrudICache This );
dcrudStatus      dcrudICache_publish     ( dcrudICache This );
void             dcrudICache_subscribe   ( dcrudICache This, dcrudClassID id );
dcrudStatus      dcrudICache_refresh     ( dcrudICache This );
