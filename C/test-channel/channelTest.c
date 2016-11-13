#include <channel/UDPChannel.h>
#include <channel/Factories.h>

#include "Family.h"
#include "Person.h"

#include <util/Pool.h>
#include <util/CmdLine.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#ifdef _WIN32
#  define STRICT 1
#  include <windows.h>
#endif

#define HELLO_MSG_ID            10
#define FAMILY_REFERENCE_MSG_ID 20

typedef struct channelTestApp__ {

   ioByteBuffer      encoder;
   channelUDPChannel UDPChannel;

} channelTestApp;

static utilStatus parseHello( short msgId, ioByteBuffer decoder, void * app ) {
   char   text[1024];
   double value = 0.0;
   CHK(__FILE__,__LINE__,ioByteBuffer_getString( decoder, text, sizeof( text )))
   CHK(__FILE__,__LINE__,ioByteBuffer_getDouble( decoder, &value ))
   fprintf( stderr, "parseHello: %d received: '%s', %f\n", msgId, text, value );
   return UTIL_STATUS_NO_ERROR;
   (void)app;
}

static utilStatus parseFamily( short msgId, ioByteBuffer decoder, void * app ) {
   channelTestFamily * family = NULL;
   char                buffer[256];
   size_t              sizeOfBuffer = sizeof( buffer );

   CHK(__FILE__,__LINE__,channelCoDec_getReference( decoder, &family ))
   buffer[0] = '\0';
   CHK(__FILE__,__LINE__,channelTestFamily_toString( family, buffer, &sizeOfBuffer ))
   fprintf( stderr, "parseFamily: %d:%s\n", msgId, buffer );
   channelTestFamily_delete( &family );
   return UTIL_STATUS_NO_ERROR;
   (void)app;
}

static utilStatus channelTestApp_sendHello( channelTestApp * This ) {
   CHK(__FILE__,__LINE__,channelCoDec_init( This->encoder, HELLO_MSG_ID ));
   CHK(__FILE__,__LINE__,ioByteBuffer_putString( This->encoder, "Hello, World!" ));
   CHK(__FILE__,__LINE__,ioByteBuffer_putDouble( This->encoder, 4.2 ));
   CHK(__FILE__,__LINE__,ioByteBuffer_flip( This->encoder ));
   CHK(__FILE__,__LINE__,channelUDPChannel_send( This->UDPChannel, This->encoder ));
   return UTIL_STATUS_NO_ERROR;
}

static utilStatus channelTestApp_sendFamily( channelTestApp * This ) {
   channelTestPerson muriel;
   channelTestPerson eve;
   channelTestFamily family;

   CHK(__FILE__,__LINE__,channelTestFamily_init( &family ));
   CHK(__FILE__,__LINE__,channelTestPerson_init( &muriel ));
   CHK(__FILE__,__LINE__,channelTestPerson_init( &eve    ));
   CHK(__FILE__,__LINE__,channelTestForname_set      (  family.parent1.forname  , "Aubin"      ));
   CHK(__FILE__,__LINE__,channelTestName_set         (  family.parent1.name     , "Mahé"       ));
   CHK(__FILE__,__LINE__,channelTestISO8601Date_parse( &family.parent1.birthdate, "1966-01-24" ));
   CHK(__FILE__,__LINE__,channelTestForname_set      (  muriel.forname          , "Muriel"     ));
   CHK(__FILE__,__LINE__,channelTestName_set         (  muriel.name             , "Le Nain"    ));
   CHK(__FILE__,__LINE__,channelTestISO8601Date_parse( &muriel.birthdate        , "1973-01-26" ));
   CHK(__FILE__,__LINE__,channelTestForname_set      (  eve.forname             , "Eve"        ));
   CHK(__FILE__,__LINE__,channelTestName_set         (  eve.name                , "Mahé"       ));
   CHK(__FILE__,__LINE__,channelTestISO8601Date_parse( &eve.birthdate           , "2008-02-28" ));
   family.parent2 = &muriel;
   CHK(__FILE__,__LINE__,channelList_addItem((channelList*)&(family.children), &eve ));
   CHK(__FILE__,__LINE__,channelCoDec_init( This->encoder, FAMILY_REFERENCE_MSG_ID ));
   CHK(__FILE__,__LINE__,channelCoDec_putReference( This->encoder, (const void *)&family ));
   CHK(__FILE__,__LINE__,channelUDPChannel_send( This->UDPChannel, This->encoder ));
   CHK(__FILE__,__LINE__,channelTestFamily_done( &family ))
   return UTIL_STATUS_NO_ERROR;
}

static utilStatus channelTestApp_init(
   channelTestApp * This,
   const char *     intrfc,
   unsigned short   local,
   const char *     targetHost,
   unsigned short   remote      )
{
   CHK(__FILE__,__LINE__,ioByteBuffer_new( &This->encoder, 64*1024 ));
   CHK(__FILE__,__LINE__,channelTestISO8601Date_registerFactory());
   CHK(__FILE__,__LINE__,channelTestPerson_registerFactory());
   CHK(__FILE__,__LINE__,channelTestFamily_registerFactory());
   CHK(__FILE__,__LINE__,channelUDPChannel_new( &This->UDPChannel, intrfc, local, targetHost, remote ));
   CHK(__FILE__,__LINE__,channelUDPChannel_addHandler( This->UDPChannel, HELLO_MSG_ID, parseHello, This ));
   CHK(__FILE__,__LINE__,channelUDPChannel_addHandler( This->UDPChannel, FAMILY_REFERENCE_MSG_ID, parseFamily, This ));
   return UTIL_STATUS_NO_ERROR;
}

static utilStatus channelTestApp_done( channelTestApp * This ) {
   CHK(__FILE__,__LINE__,ioByteBuffer_delete     ( &This->encoder    ))
   CHK(__FILE__,__LINE__,channelUDPChannel_delete( &This->UDPChannel ))
   return UTIL_STATUS_NO_ERROR;
}

/*
 * Pour thales :
--interface=192.168.56.1
--target-host=192.168.56.1
--local-port=2417
--remote-port=2416
*/
utilStatus channelTest( const utilCmdLine cmdLine ) {
   channelTestApp app;
   const char *   intrfc;
   unsigned short local;
   const char *   targetHost;
   unsigned short remote;
   int            c = 0;

   CHK(__FILE__,__LINE__,utilCmdLine_getString( cmdLine, "interface"  , &intrfc ));
   CHK(__FILE__,__LINE__,utilCmdLine_getUShort( cmdLine, "local-port" , &local ));
   CHK(__FILE__,__LINE__,utilCmdLine_getString( cmdLine, "target-host", &targetHost ));
   CHK(__FILE__,__LINE__,utilCmdLine_getUShort( cmdLine, "remote-port", &remote ));
   CHK(__FILE__,__LINE__,channelTestApp_init( &app, intrfc, local, targetHost, remote ));
   while( c != 'Q' ) {
      printf( "\n[Q]uit, [H]ello or [F]amilly: " );
      fflush( stdout );
      c = toupper( getc( stdin ));
      printf( "\n" );
      if( c == 'H' ) {
         channelTestApp_sendHello( &app );
      }
      else if( c == 'F' ) {
         channelTestApp_sendFamily( &app );
      }
   }
   channelTestApp_done( &app );
   printf( "\nGood bye!\n" );
   fflush( stdout );
   return UTIL_STATUS_NO_ERROR;
}
