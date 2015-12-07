#include <os/System.h>
#include <util/CheckSysCall.h>

#if defined( WIN32 ) || defined( _WIN32 )
#  include <windows.h>
#else
#  include <time.h>
#  include <sys/time.h>
#  include <stdio.h>
#  include <errno.h>
#endif

bool osSystem_sleep( unsigned int milliseconds ) {
#ifdef WIN32
   Sleep( milliseconds );
   return true;
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
      perror( "nanosleep" );
   }
   utilCheckSysCall( 0 == result,
      __FILE__, __LINE__, "nanosleep( %u )\n", milliseconds*1000000U );
   return result == 0;
#endif
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
