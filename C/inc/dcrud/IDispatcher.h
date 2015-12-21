#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <coll/Map.h>
#include "Shareable.h"
#include "Status.h"
#include "IProvided.h"
#include "IRequired.h"
#include "ICRUD.h"

UTIL_ADT( dcrudIDispatcher );

dcrudIProvided dcrudIDispatcher_provide       ( dcrudIDispatcher   This, const char * name );
dcrudIRequired dcrudIDispatcher_require       ( dcrudIDispatcher   This, const char * name );
dcrudICRUD     dcrudIDispatcher_requireCRUD   ( dcrudIDispatcher   This, dcrudClassID classId );
void           dcrudIDispatcher_handleRequests( dcrudIDispatcher   This );

#ifdef __cplusplus
}
#endif
