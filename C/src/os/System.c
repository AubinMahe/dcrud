#include <os/System.h>
#include <util/Status.h>

#if defined( WIN32 ) || defined( _WIN32 )
#  include <windows.h>
#else
#  include <time.h>
#  include <sys/time.h>
#  include <stdio.h>
#  include <errno.h>
#endif

utilStatus osSystem_sleep( unsigned int milliseconds ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
#ifdef WIN32
   Sleep( milliseconds );
#else
   int result = 0;
   struct timespec ts_remaining;
   ts_remaining.tv_sec  = milliseconds / 1000;
   ts_remaining.tv_nsec = ( milliseconds % 1000 ) * 1000000L;
   do {
      struct timespec ts_sleep = ts_remaining;
      result = nanosleep( &ts_sleep, &ts_remaining );
   } while( EINTR == result );
   if( result ) {
      status = UTIL_STATUS_STD_API_ERROR;
   }
#endif
   return status;
}

uint64_t osSystem_nanotime( void ) {
#ifdef WIN32
   return 1000U*1000U*GetTickCount();
#else
   struct timeval tv = { 0, 0 };
   uint64_t       retVal;
   if( gettimeofday( &tv, NULL )) {
      tv.tv_sec  = 0;
      tv.tv_usec = 0;
   }
   retVal = (uint64_t)tv.tv_sec;
   retVal *= 1000; /* milli */
   retVal *= 1000; /* micro */
   retVal += (uint64_t)tv.tv_usec;
   retVal *= 1000; /* nano  */
   return retVal;
#endif
}
