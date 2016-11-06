#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "ParticipantImpl.h"

utilStatus dcrudCache_new              ( dcrudICache * This, dcrudIParticipantImpl * participant );
utilStatus dcrudCache_delete           ( dcrudICache * This );
utilStatus dcrudCache_updateFromNetwork( dcrudICache   This, ioByteBuffer item );
utilStatus dcrudCache_deleteFromNetwork( dcrudICache   This, dcrudGUID id );

#ifdef __cplusplus
}
#endif
