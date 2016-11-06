#include <util/types.h>
#include <util/Trace.h>
#include <util/CmdLine.h>
#include <util/Pool.h>

#ifdef _WIN32
#  include <crtdbg.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* 001 Java only */
/* 002 */ utilStatus coll( void );
/* 003 */ utilStatus ioByteBufferTest( void );
/* 004 */ utilStatus channelTest                       ( const utilCmdLine cmdLine );
/* 005 */ utilStatus c_publisher_java_subscriber_Person( const utilCmdLine cmdLine );
/* 006 */ utilStatus c_subscriber_java_publisher_Person( const utilCmdLine cmdLine );
/* 007 Java only */
/* 008 */ utilStatus c_publisher_java_subscriber_Shapes( const utilCmdLine cmdLine );
/* 009 */ utilStatus poolTest( void );
/* 010 */ utilStatus dcrudArgumentsTest( void );

int main( int argc, char * argv[] ) {
   utilCmdLine cmdLine = NULL;
   char        logname[200];
   int         testNumber = -1;
#ifdef _WIN32
   WSADATA        wsaData;
   int            iResult;

   iResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
   if( iResult ) {
      fprintf( stderr, "WSAStartup failed: %d\n", iResult );
      return 1;
   }
#  ifdef _MSCVER
   _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
#  endif
#endif
   CHK(__FILE__,__LINE__,utilCmdLine_new       ( &cmdLine ))
   CHK(__FILE__,__LINE__,utilCmdLine_addInt    ( cmdLine, "test"                , 0 ))
   CHK(__FILE__,__LINE__,utilCmdLine_addBoolean( cmdLine, "dump-received-buffer", false ))
   CHK(__FILE__,__LINE__,utilCmdLine_addString ( cmdLine, "interface"           , "192.168.1.7" ))
   CHK(__FILE__,__LINE__,utilCmdLine_addString ( cmdLine, "target-host"         , "192.168.1.7" ))
   CHK(__FILE__,__LINE__,utilCmdLine_addUShort ( cmdLine, "local-port"          , 2416U ))
   CHK(__FILE__,__LINE__,utilCmdLine_addUShort ( cmdLine, "remote-port"         , 2417U ))
   CHK(__FILE__,__LINE__,utilCmdLine_parse     ( cmdLine, argc, argv ))
   CHK(__FILE__,__LINE__,utilCmdLine_getInt    ( cmdLine, "test"                , &testNumber ))
   sprintf( logname, "Test_%03d.log", testNumber );
   utilTrace_open( logname );
   printf( "TEST %03d - ", testNumber );
   switch( testNumber ) {
   default:
      fprintf( stderr, "%s --test=<nnn> [--dump-received-buffer={false,true}]\n"
         "nnn in:\n"
         "\t1 Java only\n"
         "\t2 collections\n"
         "\t3 ioByteBuffer\n"
         "\t4 channel\n"
         "\t5 c publisher  java subscriber Person\n"
         "\t6 c subscriber java publisher  Person\n"
         "\t7 Java only\n"
         "\t8 c publisher  java subscriber Shapes\n"
         "\t9 pool\n"
         "\t10 dcrudArguments\n",
         argv[0] );
      break;
   case 1:
      printf( "Java only test: Person pub/sub\n" );
      break;
   case 2:
      printf( "collMap, collSet, collList\n" );
      CHK(__FILE__,__LINE__,coll());
      break;
   case 3:
      printf( "ioByteBuffer\n" );
      ioByteBufferTest();
      break;
   case 4:
      printf( "Channel test\n" );
      channelTest( cmdLine );
      break;
   case 5:
      printf( "C Person publisher, Java subscriber\n" );
      c_publisher_java_subscriber_Person( cmdLine );
      break;
   case 6:
      printf( "C Person subscriber, Java publisher\n" );
      c_subscriber_java_publisher_Person( cmdLine );
      break;
   case 7:
      printf( "Java only test: Moving Shapes pub/sub\n" );
      break;
   case 8:
      printf( "C Shapes publisher, Java subscriber\n" );
      c_publisher_java_subscriber_Shapes( cmdLine );
      break;
   case 9:
      printf( "Pool test\n" );
      poolTest();
      break;
   case 10:
      printf( "dcrudArguments test\n" );
      dcrudArgumentsTest();
      break;
   }
   utilCmdLine_delete( &cmdLine );
   printf( "Press enter to exit\n" );
   fgetc( stdin );
   utilPool_dumpAll( stdout );
#ifdef _WIN32
   WSACleanup();
#endif
   return 0;
}
