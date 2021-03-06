#include <util/Trace.h>
#include <os/System.h>

#include <stdarg.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef linux
#  include <execinfo.h>
#endif

static FILE * log = NULL;

static void closeLog( void ) {
   if( log ) {
      fclose( log );
      log = NULL;
   }
}

utilStatus utilTrace_open( const char * filename ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( ! log ) {
      uint64_t now = osSystem_nanotime();
      log = fopen( filename, "wt" );
      if( log ) {
         fprintf( log, "%020"PRIu64": begin\n", now );
         atexit( closeLog );
      }
      else {
         status = UTIL_STATUS_STD_API_ERROR;
      }
   }
   return status;
}

utilStatus utilTrace_entry( const char * functionName, const char * format, ... ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( log ) {
      uint64_t now  = osSystem_nanotime();
      va_list  args;
      char     buff[10*1024];
      va_start( args, format );
      vsprintf( buff, format, args );
      fprintf( log, "%020"PRIu64": %s;ENTRY;%s\n", now, functionName, buff );
   }
   return status;
}

utilStatus utilTrace_error( const char * functionName, const char * format, ... ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( log ) {
      uint64_t now  = osSystem_nanotime();
      va_list  args;
      char     buff[10*1024];
      va_start( args, format );
      vsprintf( buff, format, args );
      fprintf( log, "%020"PRIu64": %s;ERROR;%s\n", now, functionName, buff );
   }
   return status;
}

utilStatus utilTrace_exit( const char * functionName, const char * format, ... ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( log ) {
      uint64_t now  = osSystem_nanotime();
      va_list  args;
      char     buff[10*1024];
      va_start( args, format );
      vsprintf( buff, format, args );
      fprintf( log, "%020"PRIu64": %s;EXIT;%s\n", now, functionName, buff );
   }
   return status;
}

utilStatus utilPrintStackTrace( void ) {
#ifdef linux
   unsigned i;
   void *   addresses[80];
   unsigned count   = backtrace( addresses, sizeof(addresses)/sizeof(addresses[0]));
   char **  symbols = backtrace_symbols( addresses, count );
   for( i = 1; i < count; ++i ) {
      char * s = strchr( symbols[i], '(' );
      char * e = strchr( symbols[i], '+' );
      if( e ) {
         *e = '\0';
      }
      else if( strchr( symbols[i], '[' )) {
         continue;
      }
      if( s ) {
         printf( "%s ", s+1 );
      }
      else {
         printf( "%s\n", symbols[i] );
      }
   }
   printf( "\n" );
   free( symbols );
#endif
   return UTIL_STATUS_NO_ERROR;
}
