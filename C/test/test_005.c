#include <dcrud/Network.h>
#include <os/System.h>
#include <util/CheckSysCall.h>

#include "Person.h"
#include "Settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static dcrudArguments exitSrvc( dcrudIParticipant participant, dcrudArguments args ) {
   printf( "Well done, press <enter> to exit\n" );
   fgetc( stdin );
   dcrudIParticipant_delete( &participant );
   exit(0);
   (void)args;
}

/*
 * C Person publisher, Java subscriber
 */
void test_005( void ) {
   dcrudIParticipant participant = dcrudNetwork_join( 2, MCAST_ADDRESS, 2417, NETWORK_INTERFACE );
   if( participant ) {
      dcrudIDispatcher   dispatcher       = NULL;
      dcrudIProvided     monitor          = NULL;
      dcrudCounterpart   firstCounterpart = { MCAST_ADDRESS, 2416 };
      dcrudCounterpart * counterparts[]   = {
         &firstCounterpart,
         NULL
      };
      dcrudLocalFactory *  localFactory;
      dcrudRemoteFactory * remoteFactory;

      Person_initFactories( participant, &localFactory, &remoteFactory );
      dcrudIParticipant_listen( participant, NETWORK_INTERFACE, counterparts );
      dispatcher = dcrudIParticipant_getDispatcher( participant );
      monitor    = dcrudIDispatcher_provide( dispatcher, "IMonitor" );
      dcrudIProvided_addOperation( monitor, "exit", participant, (dcrudIOperation)exitSrvc );

      for(;;) {
         osSystem_sleep( 100U );
         dcrudIDispatcher_handleRequests( dispatcher );
      }
   }
   else {
      fprintf( stderr, "Unable to join network.\n" );
   }
}
