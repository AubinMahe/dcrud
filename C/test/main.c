#ifdef _WIN32
#  include <crtdbg.h>
#endif
#include <string.h>

int unitTests     ( int argc, char * argv[] );
int functionalTest( int argc, char * argv[] );

int main( int argc, char * argv[] ) {
#ifdef _WIN32
   _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
   if( argc > 1 && 0 == strcmp( "tu", argv[1] )) {
	   unitTests( argc, argv );
   }
   else {
	   functionalTest( argc, argv );
   }
   return 0;
}
