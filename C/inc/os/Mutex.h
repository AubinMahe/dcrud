#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/types.h>

UTIL_ADT( osMutex );

int osMutex_new    ( osMutex * This );
int osMutex_delete ( osMutex * This );
int osMutex_take   ( osMutex   This );
int osMutex_release( osMutex   This );

#ifdef __cplusplus
}
#endif
