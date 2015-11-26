#pragma once

#include <util/types.h>

void dbgPerformance_enable( bool enabled);
void dbgPerformance_record( const char * attribute, uint64_t elapsed );
void dbgPerformance_saveToDisk( void );
