#include <os/System.h>
#include <time.h>

uint64_t osSystem_nanotime( void ) {
   struct timespec tp;
   uint64_t        retVal;
   if( clock_gettime( CLOCK_THREAD_CPUTIME_ID, &tp )) {
      tp.tv_sec  = 0;
      tp.tv_nsec = 0;
   }
   retVal = (uint64_t)tp.tv_sec;
   retVal *= 1000; /* milli */
   retVal *= 1000; /* micro */
   retVal *= 1000; /* nano  */
   retVal += (uint64_t)tp.tv_nsec;
   return retVal;
}
