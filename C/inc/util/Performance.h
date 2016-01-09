#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

void utilPerformance_enable( bool enabled);
void utilPerformance_record( const char * attribute, uint64_t elapsed );
void utilPerformance_saveToDisk( void );

#ifdef __cplusplus
}
#endif
