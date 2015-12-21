#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "Shareable.h"

UTIL_ADT( dcrudICRUD );

void dcrudICRUD_create( dcrudICRUD This, dcrudArguments how );
void dcrudICRUD_update( dcrudICRUD This, dcrudShareable what, dcrudArguments how );
void dcrudICRUD_delete( dcrudICRUD This, dcrudShareable what );

#ifdef __cplusplus
}
#endif
