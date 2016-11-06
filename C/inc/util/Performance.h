#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "Status.h"

utilStatus utilPerformance_enable( bool enabled);
utilStatus utilPerformance_record( const char * attribute, uint64_t elapsed );
utilStatus utilPerformance_saveToDisk( void );

#ifdef __cplusplus
}
#endif
