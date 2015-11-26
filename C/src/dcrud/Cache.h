#pragma once

#include "Network.h"

dcrudICache dcrudCache_new( dcrudIParticipant network, byte platformId, byte execId );
void dcrudCache_delete           ( dcrudICache * This );
void dcrudCache_updateFromNetwork( dcrudICache   This, ioByteBuffer item );
void dcrudCache_deleteFromNetwork( dcrudICache   This, const dcrudGUID * id );
bool dcrudCache_matches          ( dcrudICache   This, byte platformId, byte execId, byte cacheId );
