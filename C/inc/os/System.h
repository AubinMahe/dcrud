#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>

utilStatus osSystem_sleep( unsigned int milliseconds );
uint64_t   osSystem_nanotime( void );

#ifdef __cplusplus
}
#endif
