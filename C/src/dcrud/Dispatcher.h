#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/IDispatcher.h>
#include "ParticipantImpl.h"

utilStatus dcrudIDispatcher_new        ( dcrudIDispatcher * This, dcrudIParticipantImpl * participant );
utilStatus dcrudIDispatcher_delete     ( dcrudIDispatcher * This );
utilStatus dcrudIDispatcher_executeCrud( dcrudIDispatcher   This, const char * opName, dcrudArguments args);
utilStatus dcrudIDispatcher_execute    ( dcrudIDispatcher   This, const char * intrfcName, const char * opName, dcrudArguments args, int callId, unsigned queueNdx, dcrudCallMode callMode );

#ifdef __cplusplus
}
#endif
