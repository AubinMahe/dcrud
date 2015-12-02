#include <os/System.h>
#include <stdio.h>

int main( int argc, char * argv[]) {
   unsigned int i;
   unsigned int delay = ( argc > 1 ) ? ((unsigned int)atoi( argv[1] )) : 250U;
   for( i = 0; i < 100U; ++i ) {
      const uint64_t before = osSystem_nanotime();
      osSystem_sleep( delay );
      const uint64_t after = osSystem_nanotime();
      fprintf( stderr, "delta: %7.2f\n", (after-before)/1000000.0);
   }
   return 0;
}
