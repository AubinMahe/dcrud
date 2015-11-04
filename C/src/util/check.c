#include <util/check.h>
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <windows.h>
#endif
#include <stdarg.h>
#include <stdio.h>

void check(
   bool         ok,
   const char * file,
   int          line,
   const char * format,
   ... )
{
   if( ! ok) {
      DWORD   err = GetLastError();
      char    systMsg[1000];
      char    userMsg[1200];
      va_list args;
      if( err == 0 ) {
         err = (DWORD)WSAGetLastError();
      }
      FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, systMsg, sizeof( systMsg ), NULL );
      va_start( args, format );
      vsnprintf( userMsg, sizeof( userMsg ), format, args );
      fprintf( stderr, "%s:%d:%s:%s", file, line, userMsg, systMsg );
   }
}
