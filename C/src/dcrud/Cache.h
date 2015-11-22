#pragma once

#include "Network.h"

dcrudICache dcrudCache_init( dcrudIParticipant network, byte platformId, byte execId );
void        dcrudCache_updateFromNetwork( dcrudICache This, dcrudShareable item );
void        dcrudCache_deleteFromNetwork( dcrudICache This, dcrudGUID id );
