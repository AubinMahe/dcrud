#include <util/check.h>
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <windows.h>
#elif __linux__
#  include <errno.h>
#  include <string.h>
#endif
#include <stdarg.h>
#include <stdio.h>

extern void catchError();

void check(
   bool         ok,
   const char * file,
   int          line,
   const char * format, ... )
{
   if( !ok ) {
      char    userMsg[1200];
#ifdef WIN32
      DWORD   err = GetLastError();
      char    systMsg[1000];
      if( err == 0 ) {
         err = (DWORD)WSAGetLastError();
      }
      FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, systMsg, sizeof( systMsg ), NULL );
#elif __linux__
      const char * systMsg = strerror( errno );
#endif
      va_list args;
      va_start( args, format );
      vsnprintf( userMsg, sizeof( userMsg ), format, args );
      fprintf( stderr, "%s:%d:%s:%s", file, line, userMsg, systMsg );
      catchError();
   }
}
