#pragma once

#include <dcrud/IDispatcher.h>
#include <dcrud/IParticipant.h>

dcrudIDispatcher dcrudDispatcher_new( dcrudIParticipant network );
void             dcrudIDispatcher_execute(
   dcrudIDispatcher This,
   const char *     intrfcName,
   const char *     opName,
   collMap          arguments,
   collMap          results,
   unsigned         queueNdx,
   dcrudCallMode    callMode );
