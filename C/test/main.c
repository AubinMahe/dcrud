#include <util/types.h>
#include <util/Trace.h>

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

bool dumpReceivedBuffer = false;

int main( int argc, char * argv[] ) {
   char logname[200];
   int  testNumber = -1;
   int  i;
   for( i = 1; i < argc; ++i ) {
      if( 0 == strncmp( argv[i], "--test=", 7 )) {
         testNumber = atoi( argv[i] + 7 );
      }
      else if( 0 == strcmp( argv[i], "--dump-received-buffer" )) {
         dumpReceivedBuffer = true;
      }
   }
#ifdef _WIN32
   _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
   sprintf( logname, "Test_%03d.log", testNumber );
   utilTrace_open( logname );
   switch( testNumber ) {
   default:
      fprintf( stderr, "%s --test=<nnn> where nnn is a test number > 0\n", argv[0] );
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
