#include <dcrud/Network.h>
#include <os/System.h>
#include <util/CheckSysCall.h>

#include "Person.h"
#include "Settings.h"
#include "StaticRegistry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static dcrudArguments exitSrvc( dcrudIParticipant participant, dcrudArguments args ) {
   printf( "Well done, press <enter> to exit\n" );
   fgetc( stdin );
   dcrudNetwork_leave( &participant );
   exit(0);
   (void)args;
}

extern bool dumpReceivedBuffer;

/*
 * C Person publisher, Java subscriber
 */
void test_005( void ) {
   ioInetSocketAddress p1;
   dcrudIParticipant participant;

   ioInetSocketAddress_init( &p1, MCAST_ADDRESS, 2417 );
   participant = dcrudNetwork_join( 2, &p1, NULL );
   if( participant ) {
      dcrudIDispatcher     dispatcher;
      dcrudIProvided       monitor;
      dcrudLocalFactory *  localFactory;
      dcrudRemoteFactory * remoteFactory;
      ioInetSocketAddress  p2;
      dcrudIRegistry       registry = getStaticRegistry();

      ioInetSocketAddress_init( &p2, MCAST_ADDRESS, 2416 );
      Person_initFactories( participant, &localFactory, &remoteFactory );
      dcrudIParticipant_listen( participant, registry, NULL, dumpReceivedBuffer );
      dispatcher = dcrudIParticipant_getDispatcher( participant );
      monitor    = dcrudIDispatcher_provide( dispatcher, "IMonitor" );
      dcrudIProvided_addOperation( monitor, "exit", participant, (dcrudIOperation)exitSrvc );

      for(;;) {
         osSystem_sleep( 10U );
         dcrudIDispatcher_handleRequests( dispatcher );
      }
   }
   else {
      fprintf( stderr, "Unable to join network.\n" );
   }
}
