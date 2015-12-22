#include <dcrud/Network.h>
#include <os/System.h>
#include <util/CheckSysCall.h>

#include "Person.h"
#include "Settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void remotelyCreatePerson(
   dcrudICRUD   personCRUD,
   const char * forname,
   const char * name,
   const char * birthdate  )
{
   dcrudArguments how = dcrudArguments_new();
   dcrudArguments_putString( how, "forname"  , forname );
   dcrudArguments_putString( how, "name"     , name );
   dcrudArguments_putString( how, "birthdate", birthdate );
   dcrudICRUD_create( personCRUD, how );
   dcrudArguments_delete( &how );
}

/*
 * C Person subscriber, Java publisher
 */
void test_006( void ) {
   dcrudIParticipant participant = dcrudNetwork_join( 2, MCAST_ADDRESS, 2417, NETWORK_INTERFACE );
   if( participant ) {
      dcrudICache          cache;
      dcrudIDispatcher     dispatcher;
      dcrudICRUD           personCRUD;
      dcrudIRequired       monitor;
      dcrudCounterpart     firstCounterpart = { MCAST_ADDRESS, 2416 };
      dcrudCounterpart *   counterparts[] = { &firstCounterpart, NULL };
      dcrudLocalFactory *  localFactory;
      dcrudRemoteFactory * remoteFactory;

      Person_initFactories( participant, &localFactory, &remoteFactory );

      dcrudIParticipant_listen( participant, NETWORK_INTERFACE, counterparts );

      cache      = dcrudIParticipant_getDefaultCache( participant );
      dispatcher = dcrudIParticipant_getDispatcher( participant );
      personCRUD = dcrudIDispatcher_requireCRUD( dispatcher, localFactory->classID );
      monitor    = dcrudIDispatcher_require( dispatcher, "IMonitor" );

      printf( "Ready to ask Java process for a Person creation, press <enter> please...\n" );
      fgetc( stdin );
      remotelyCreatePerson( personCRUD, "Aubin", "Mahé", "1966-01-24" );
      osSystem_sleep( 1000U );
      dcrudIDispatcher_handleRequests( dispatcher );
      dcrudICache_refresh( cache );
      printf( "Cache content:\n" );
      dcrudICache_foreach( cache, Person_print, NULL );
      printf( "Press <enter> to terminate Java counterpart\n" );
      fgetc( stdin );
      dcrudIRequired_call( monitor, "exit", NULL, NULL );
      dcrudIParticipant_delete( &participant );
   }
   else {
      fprintf( stderr, "Unable to join network.\n" );
   }
}
