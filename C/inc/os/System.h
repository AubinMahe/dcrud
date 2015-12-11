#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/types.h>

uint64_t osSystem_nanotime( void );
bool     osSystem_sleep( unsigned int milliseconds );

#ifdef __cplusplus
}
#endif
