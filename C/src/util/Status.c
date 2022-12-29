#include <util/Status.h>
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <winsock2.h>
#elif __linux__
#  include <errno.h>
#  include <string.h>
#endif
#include <stdarg.h>
#include <stdio.h>

const char * utilStatusMessages[UTIL_STATUS_LAST+1] = {
/* UTIL_STATUS_NO_ERROR = 0,  */"No error",
/* UTIL_STATUS_BAD_CAST,      */"Bad cast",
/* UTIL_STATUS_DUPLICATE,     */"Duplicate entry or call",
/* UTIL_STATUS_ILLEGAL_STATE, */"Illegal state",
/* UTIL_STATUS_NOT_FOUND,     */"Not found",
/* UTIL_STATUS_NULL_ARGUMENT, */"Argument is null",
/* UTIL_STATUS_OUT_OF_RANGE,  */"Out of range",
/* UTIL_STATUS_OVERFLOW,      */"Overflow (put over limit)",
/* UTIL_STATUS_STD_API_ERROR, */"Syscall failed",
/* UTIL_STATUS_TOO_MANY,      */"Not enougth memory",
/* UTIL_STATUS_TYPE_MISMATCH, */"Type mismatch",
/* UTIL_STATUS_UNDERFLOW,     */"Underflow (get over limit)",
/* UTIL_STATUS_LAST           */""
};

utilStatus utilStatus_checkAndLog(
   utilStatus   This,
   const char * file,
   int          line,
   const char * format,
   ...                  )
{
   va_list args;
   va_start( args, format );
   if( This != UTIL_STATUS_NO_ERROR ) {
      char userMsg[1200] = "";
      char systMsg[1000] = "";
      const char * utilMsg = ( This < UTIL_STATUS_LAST ) ? utilStatusMessages[This] : NULL;
#ifdef WIN32
      DWORD err = GetLastError();
      if( err == 0 ) {
         err = (DWORD)WSAGetLastError();
      }
      FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, systMsg, sizeof( systMsg ), NULL );
#elif __linux__
      strerror_r( errno, systMsg, sizeof( systMsg ));
#endif
      if( format ) {
         vsnprintf( userMsg, sizeof( userMsg ), format, args );
         if( systMsg[0] ) {
            if( utilMsg ) {
               fprintf( stderr, "%s:%d:%s:%s:%s\n", file, line, utilMsg, userMsg, systMsg );
            }
            else {
               fprintf( stderr, "%s:%d:%s:%s\n", file, line, userMsg, systMsg );
            }
         }
         else {
            if( utilMsg ) {
               fprintf( stderr, "%s:%d:%s:%s\n", file, line, utilMsg, userMsg );
            }
            else {
               fprintf( stderr, "%s:%d:%s\n", file, line, userMsg );
            }
         }
      }
      else {
         if( systMsg[0] ) {
            if( utilMsg ) {
               fprintf( stderr, "%s:%d:%s:%s\n", file, line, userMsg, systMsg );
            }
            else {
               fprintf( stderr, "%s:%d:%s\n", file, line, systMsg );
            }
         }
         else {
            if( utilMsg ) {
               fprintf( stderr, "%s:%d:%s\n", file, line, userMsg );
            }
            else {
               fprintf( stderr, "%s:%d:Unexpected status: %d!\n", file, line, This );
            }
         }
      }
   }
   return This;
}
