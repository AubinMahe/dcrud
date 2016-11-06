#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "Shareable.h"

UTIL_ADT( dcrudICRUD );

utilStatus dcrudICRUD_create( dcrudICRUD This, dcrudArguments how );
utilStatus dcrudICRUD_update( dcrudICRUD This, dcrudShareable what, dcrudArguments how );
utilStatus dcrudICRUD_delete( dcrudICRUD This, dcrudShareable what );

#ifdef __cplusplus
}
#endif
