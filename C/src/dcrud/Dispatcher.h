#pragma once

#include <dcrud/IDispatcher.h>
#include "ParticipantImpl.h"

dcrudIDispatcher dcrudIDispatcher_new    ( ParticipantImpl * participant );
void             dcrudIDispatcher_delete ( dcrudIDispatcher * This );
void             dcrudIDispatcher_execute( dcrudIDispatcher   This,
   const char *     intrfcName,
   const char *     opName,
   collMap          arguments,
   collMap          results,
   unsigned         queueNdx,
   dcrudCallMode    callMode );
