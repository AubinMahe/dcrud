#include <util/iso8601.h>
#include <stdio.h>
#include <string.h>

char * strptime( const char * s, const char * format, struct tm * tm ) {
   static char * retVal = "";
   if( 0 == strcmp( format, "%F" )) {
      sscanf( s, "%4d-%d-%d", &tm->tm_year, &tm->tm_mon, &tm->tm_mday );
   }
   return retVal;
}
