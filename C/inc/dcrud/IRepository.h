#pragma once
#include <coll/Map.h>
#include <dcrud/Shareable.h>

DCRUD_ADT( dcrudIRepository );

void             dcrudIRepository_create    ( dcrudIRepository This, dcrudShareable item );
dcrudShareable * dcrudIRepository_read      ( dcrudIRepository This, dcrudGUID id );
bool             dcrudIRepository_update    ( dcrudIRepository This, dcrudShareable item );
void             dcrudIRepository_delete    ( dcrudIRepository This, dcrudShareable item );
collMap          dcrudIRepository_getAll    ( dcrudIRepository This );
bool             dcrudIRepository_isProducer( dcrudIRepository This );
bool             dcrudIRepository_isConsumer( dcrudIRepository This );
void             dcrudIRepository_publish   ( dcrudIRepository This );
void             dcrudIRepository_refresh   ( dcrudIRepository This );
