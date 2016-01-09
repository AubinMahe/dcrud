#pragma once

void utilTrace_open ( const char * filename );
void utilTrace_entry( const char * functionName, const char * format, ... );
void utilTrace_exit ( const char * functionName, const char * format, ... );
void utilTrace_error( const char * functionName, const char * format, ... );
