#pragma once

#include <util/types.h>
#include "Status.h"

UTIL_ADT( osMutex );

osStatus osMutex_new    ( osMutex * This );
osStatus osMutex_delete ( osMutex * This );
osStatus osMutex_take   ( osMutex   This );
osStatus osMutex_release( osMutex   This );
