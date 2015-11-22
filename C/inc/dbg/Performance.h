#pragma once

#include <util/types.h>

UTIL_ADT( Performance );

void Performance_enable( bool enabled);
void Performance_record( const char * attribute, uint64_t elapsed );
void Performance_saveToDisk( void );
