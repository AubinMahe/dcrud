#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>

utilStatus utilString_new   ( char ** This, size_t size );
utilStatus utilString_dup   ( char ** This, const char * source );
utilStatus utilString_concat( char *  This, const char * source, size_t * size );
utilStatus utilString_delete( char ** This );

#ifdef __cplusplus
}
#endif
