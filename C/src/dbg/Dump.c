#include <dbg/Dump.h>
#include <string.h>

static void dumpLine( FILE * target, char * hexa, const char * ascii ) {
   hexa = strcat( hexa, "                                                                          " );
   hexa[6+( 3*8 )+2+( 3*8 )] = '\0';
   fprintf( target, "%s - %s\n", hexa, ascii );
}

void dbgDump_range( FILE * target, const byte * bytes, size_t from, size_t to ) {
   size_t addr        = 0;
   char   hexStr[200] = "";
   char   addStr[200] = "";
   char   hexa  [200] = "";
   char   ascii [200];
   size_t i;
   memset( hexStr, 0, sizeof( hexStr ));
   memset( addStr, 0, sizeof( addStr ));
   memset( hexa  , 0, sizeof( hexa   ));
   memset( ascii , 0, sizeof( ascii  ));
   for( i = from; i < to; ++i ) {
      byte c = bytes[i];
      if(( addr % 16 ) == 0 ) {
         if( addr > 0 ) {
            dumpLine( target, strcat( addStr, hexa ), ascii );
            memset( hexa , 0, sizeof( hexa  ));
            memset( ascii, 0, sizeof( ascii ));
         }
         sprintf( addStr, "%04X:", (unsigned int)addr );
      }
      else if(( addr % 8 ) == 0 ) {
         strcat( hexa, " -" );
      }
      ++addr;
      sprintf( hexStr, " %02X", c );
      strcat( hexa, hexStr );
      ascii[strlen(ascii)] = (( c > 32 ) && ( c < 127 )) ? (char)c : '.';
   }
   if( hexa[0] ) {
      dumpLine( target, strcat( addStr, hexa ), ascii );
   }
}

void dbgDump( FILE * target, const byte * bytes, size_t length ) {
   dbgDump_range( target, bytes, 0, length );
}
