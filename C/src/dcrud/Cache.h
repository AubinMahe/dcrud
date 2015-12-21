#pragma once

#include "ParticipantImpl.h"

dcrudICache dcrudCache_new( ParticipantImpl *participant );
void dcrudCache_delete           ( dcrudICache * This );
void dcrudCache_updateFromNetwork( dcrudICache   This, ioByteBuffer item );
void dcrudCache_deleteFromNetwork( dcrudICache   This, const dcrudGUID * id );
