#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>

UTIL_ADT( osMutex );

utilStatus osMutex_new    ( osMutex * This );
utilStatus osMutex_delete ( osMutex * This );
utilStatus osMutex_take   ( osMutex   This );
utilStatus osMutex_release( osMutex   This );

#ifdef __cplusplus
}
#endif
