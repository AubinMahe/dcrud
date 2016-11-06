#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <util/Status.h>

utilStatus utilTrace_open ( const char * filename );
utilStatus utilTrace_entry( const char * functionName, const char * format, ... );
utilStatus utilTrace_exit ( const char * functionName, const char * format, ... );
utilStatus utilTrace_error( const char * functionName, const char * format, ... );
utilStatus utilPrintStackTrace( void );

#ifdef __cplusplus
}
#endif
