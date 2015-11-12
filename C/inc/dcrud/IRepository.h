#pragma once
#include <coll/Map.h>
#include <dcrud/Shareable.h>
#include <dcrud/Errors.h>

UTIL_ADT( dcrudIRepository );

dcrudErrorCode   dcrudIRepository_create    ( dcrudIRepository This, dcrudShareable item );
dcrudShareable * dcrudIRepository_read      ( dcrudIRepository This, dcrudGUID id );
dcrudErrorCode   dcrudIRepository_update    ( dcrudIRepository This, dcrudShareable item );
dcrudErrorCode   dcrudIRepository_delete    ( dcrudIRepository This, dcrudShareable item );
collMap          dcrudIRepository_getAll    ( dcrudIRepository This );
bool             dcrudIRepository_isProducer( dcrudIRepository This );
bool             dcrudIRepository_isConsumer( dcrudIRepository This );
dcrudErrorCode   dcrudIRepository_publish   ( dcrudIRepository This );
dcrudErrorCode   dcrudIRepository_refresh   ( dcrudIRepository This );
