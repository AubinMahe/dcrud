#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "IProvided.h"
#include "IRequired.h"
#include "ICRUD.h"

UTIL_ADT( dcrudIDispatcher );

utilStatus dcrudIDispatcher_provide       ( dcrudIDispatcher This, const char * name   , dcrudIProvided * result );
utilStatus dcrudIDispatcher_require       ( dcrudIDispatcher This, const char * name   , dcrudIRequired * result );
utilStatus dcrudIDispatcher_requireCRUD   ( dcrudIDispatcher This, dcrudClassID classId, dcrudICRUD     * result );
utilStatus dcrudIDispatcher_handleRequests( dcrudIDispatcher This );

#ifdef __cplusplus
}
#endif
