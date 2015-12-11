#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/types.h>
#include <stdio.h>

void dbgDump_range( FILE * target, const byte * buffer, size_t from, size_t to );

void dbgDump( FILE * target, const byte * buffer, size_t length );

#ifdef __cplusplus
}
#endif
