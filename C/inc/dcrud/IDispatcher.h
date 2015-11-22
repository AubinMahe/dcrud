#pragma once
#include <coll/MapVoidPtr.h>
#include <dcrud/Shareable.h>
#include <dcrud/Status.h>
#include <dcrud/IProvided.h>
#include <dcrud/IRequired.h>

UTIL_ADT( dcrudIDispatcher );

dcrudIProvided dcrudDispatcher_provide       ( dcrudIDispatcher This, const char * name );
dcrudIRequired dcrudDispatcher_require       ( dcrudIDispatcher This, const char * name );
void           dcrudDispatcher_handleRequests( dcrudIDispatcher This );
