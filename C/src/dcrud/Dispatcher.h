#pragma once

#include <dcrud/IDispatcher.h>
#include "ParticipantImpl.h"

dcrudIDispatcher dcrudIDispatcher_new(
   ParticipantImpl * participant );

void dcrudIDispatcher_delete(
   dcrudIDispatcher * This );

void dcrudIDispatcher_executeCrud(
   dcrudIDispatcher This,
   const char *     opName,
   dcrudArguments   args   );

void dcrudIDispatcher_execute(
   dcrudIDispatcher This,
   const char *     intrfcName,
   const char *     opName,
   dcrudArguments   args,
   int              callId,
   unsigned         queueNdx,
   dcrudCallMode    callMode );
