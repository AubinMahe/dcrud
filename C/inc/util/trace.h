#pragma once

void trace_open ( const char * filename );
void trace_entry( const char * functionName, const char * format, ... );
void trace_exit ( const char * functionName, const char * format, ... );
void trace_error( const char * functionName, const char * format, ... );
