#pragma once
#include <coll/Map.h>
#include <dcrud/Shareable.h>
#include <dcrud/Status.h>
#include <dcrud/IProvided.h>
#include <dcrud/IRequired.h>

UTIL_ADT( dcrudIDispatcher );

dcrudIProvided dcrudIDispatcher_provide       ( dcrudIDispatcher   This, const char * name );
dcrudIRequired dcrudIDispatcher_require       ( dcrudIDispatcher   This, const char * name );
void           dcrudIDispatcher_handleRequests( dcrudIDispatcher   This );
