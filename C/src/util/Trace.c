#include <os/System.h>

#include <stdarg.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/Trace.h>

static FILE * log;

static void closeLog( void ) {
   if( log ) {
      fclose( log );
   }
}

void utilTrace_open( const char * filename ) {
   if( ! log ) {
      uint64_t now = osSystem_nanotime();
      log = fopen( filename, "wt" );
      fprintf( log, "%020"PRIu64": begin\n", now );
      atexit( closeLog );
   }
}

void utilTrace_entry( const char * functionName, const char * format, ... ) {
   if( log ) {
      uint64_t now  = osSystem_nanotime();
      va_list  args;
      char     buff[10*1024];
      va_start( args, format );
      vsprintf( buff, format, args );
      fprintf( log, "%020"PRIu64": %s;ENTRY;%s\n", now, functionName, buff );
   }
}

void utilTrace_error( const char * functionName, const char * format, ... ) {
   if( log ) {
      uint64_t now  = osSystem_nanotime();
      va_list  args;
      char     buff[10*1024];
      va_start( args, format );
      vsprintf( buff, format, args );
      fprintf( log, "%020"PRIu64": %s;ERROR;%s\n", now, functionName, buff );
   }
}

void utilTrace_exit( const char * functionName, const char * format, ... ) {
   if( log ) {
      uint64_t now  = osSystem_nanotime();
      va_list  args;
      char     buff[10*1024];
      va_start( args, format );
      vsprintf( buff, format, args );
      fprintf( log, "%020"PRIu64": %s;EXIT;%s\n", now, functionName, buff );
   }
}
