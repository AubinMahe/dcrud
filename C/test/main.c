#ifdef _WIN32
#  include <crtdbg.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*   test_001: Java only */
void test_002( void );
void test_003( void );
void test_004( void ){}
void test_005( void );
void test_006( void );
/*   test_007: Java only */
void test_008( void );

int main( int argc, char * argv[] ) {
   int testNumber = argc > 1 ? atoi( argv[1] ) : 0;
#ifdef _WIN32
   _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

   switch( testNumber ) {
   default:
      fprintf( stderr, "%s <nnn> where nnn is a test number > 0\n", argv[0] );
      break;
   case 1:
      printf( "TEST %03d - Java only test: Person pub/sub\n", testNumber );
      break;
   case 2:
      printf( "TEST %03d - collMap, collSet, collList\n", testNumber );
      test_002();
      break;
   case 3:
      printf( "TEST %03d - ioByteBuffer\n", testNumber );
      test_003();
      break;
   case 4:
      printf( "TEST %03d - Not written yet\n", testNumber );
      test_004();
      break;
   case 5:
      printf( "TEST %03d - C Person publisher, Java subscriber\n", testNumber );
      test_005();
      break;
   case 6:
      printf( "TEST %03d - C Person subscriber, Java publisher\n", testNumber );
      test_006();
      break;
   case 7:
      printf( "TEST %03d - Java only test: Moving Shapes pub/sub\n", testNumber );
      break;
   case 8:
      printf( "TEST %03d - C Shapes publisher, Java subscriber on moving Shapes\n", testNumber );
      test_008();
      break;
   }
   printf( "Press enter to exit\n" );
   fgetc( stdin );
   return 0;
}
