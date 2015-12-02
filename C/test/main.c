#ifdef _WIN32
#  include <crtdbg.h>
#endif
#include <string.h>
#include <stdio.h>

int collTests           ( int argc, char * argv[] );
int ioTests             ( int argc, char * argv[] );
int shapesPublisherTests( int argc, char * argv[] );

int main( int argc, char * argv[] ) {
#ifdef _WIN32
   _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
   if( argc == 1 || ( argc > 1 && 0 == strcmp( "--?", argv[1] ))) {
      printf( "%s --? | coll | io | shapes\n", argv[0] );
   }
   else if( argc > 1 && 0 == strcmp( "coll", argv[1] )) {
      collTests( argc, argv );
   }
   else if( argc > 1 && 0 == strcmp( "io", argv[1] )) {
      ioTests( argc, argv );
   }
   else if( argc > 1 && 0 == strcmp( "shapes", argv[1] )) {
      shapesPublisherTests( argc, argv );
   }
   return 0;
}
