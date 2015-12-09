#pragma once

#include <util/types.h>

UTIL_ADT( osMutex );

int osMutex_new    ( osMutex * This );
int osMutex_delete ( osMutex * This );
int osMutex_take   ( osMutex   This );
int osMutex_release( osMutex   This );
