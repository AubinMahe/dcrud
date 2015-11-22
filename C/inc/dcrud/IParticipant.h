#pragma once
#include <dcrud/ICache.h>
#include <dcrud/Shareable.h>
#include <dcrud/IDispatcher.h>

typedef dcrudShareable ( * dcrudShareableFactory)();

UTIL_ADT( dcrudIParticipant );

bool             dcrudIParticipant_registerClass(
   dcrudIParticipant     This,
   dcrudClassID          id,
   dcrudShareableFactory factory );
dcrudStatus      dcrudIParticipant_createCache  ( dcrudIParticipant This, dcrudICache * target );
dcrudICache      dcrudIParticipant_getCache     ( dcrudIParticipant This, byte ID );
dcrudIDispatcher dcrudIParticipant_getDispatcher( dcrudIParticipant This );
void             dcrudIParticipant_run          ( dcrudIParticipant This );
