#include <dcrud/Network.h>
#include <os/System.h>
#include <util/CmdLine.h>
#include <io/NetworkInterfaces.h>

#include "Person.h"
#include "Settings.h"
#include "StaticRegistry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static utilStatus remotelyCreatePerson(
   dcrudICRUD   personCRUD,
   const char * forname,
   const char * name,
   const char * birthdate  )
{
   dcrudArguments how;
   CHK(__FILE__,__LINE__,dcrudArguments_new( &how ));
   CHK(__FILE__,__LINE__,dcrudArguments_putString( how, "forname"  , forname ));
   CHK(__FILE__,__LINE__,dcrudArguments_putString( how, "name"     , name ));
   CHK(__FILE__,__LINE__,dcrudArguments_putString( how, "birthdate", birthdate ));
   CHK(__FILE__,__LINE__,dcrudICRUD_create( personCRUD, how ));
   CHK(__FILE__,__LINE__,dcrudArguments_delete( &how ));
   return UTIL_STATUS_NO_ERROR;
}

utilStatus c_subscriber_java_publisher_Person( const utilCmdLine cmdLine ) {
   ioInetSocketAddress  p1;
   dcrudIParticipant    participant;
   dcrudICache          cache;
   dcrudIDispatcher     dispatcher;
   dcrudICRUD           personCRUD;
   dcrudIRequired       monitor;
   dcrudLocalFactory *  localFactory;
   dcrudRemoteFactory * remoteFactory;
   ioInetSocketAddress  p2;
   char                 c = '\0';
   int                  i;
   bool                 dumpReceivedBuffer;
   dcrudIRegistry       registry = NULL;
   CHK(__FILE__,__LINE__,getStaticRegistry( &registry ));
   CHK(__FILE__,__LINE__,ioInetSocketAddress_init( &p1, MCAST_ADDRESS, 2417 ));
   CHK(__FILE__,__LINE__,dcrudNetwork_join( 2, &p1, NETWORK_INTERFACE, &participant ));
   CHK(__FILE__,__LINE__,utilCmdLine_getBoolean( cmdLine, "dump-received-buffer", &dumpReceivedBuffer ));
   CHK(__FILE__,__LINE__,ioInetSocketAddress_init( &p2, MCAST_ADDRESS, 2416 ));
   CHK(__FILE__,__LINE__,Person_initFactories( participant, &localFactory, &remoteFactory ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_listen( participant, registry, NETWORK_INTERFACE, dumpReceivedBuffer ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_getDefaultCache( participant, &cache ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_getDispatcher( participant, &dispatcher ));
   CHK(__FILE__,__LINE__,dcrudIDispatcher_requireCRUD( dispatcher, localFactory->classID, &personCRUD ));
   CHK(__FILE__,__LINE__,dcrudIDispatcher_require( dispatcher, "IMonitor", &monitor ));

   for( i = 0; c != 'Q'; ++i ) {
      printf( "Ready to ask Java process for a Person creation, press <enter> please, 'Q' to quit...\n" );
      c = (char)fgetc( stdin );
      if( c != 'Q' ) {
         dcrudIDispatcher_handleRequests( dispatcher );
         dcrudICache_refresh( cache );
         printf( "Cache content:\n" );
         dcrudICache_foreach( cache, Person_print, stdout, NULL );
         if( i % 2 ) {
            remotelyCreatePerson( personCRUD, "Aubin", "Mahé", "1966-01-24" );
         }
         else {
            remotelyCreatePerson( personCRUD, "Muriel", "Le Nain", "1973-01-26" );
         }
      }
   }
   printf( "Press <enter> to terminate Java counterpart\n" );
   fgetc( stdin );
   dcrudIRequired_call( monitor, "exit", NULL, NULL );
   dcrudNetwork_leave();
   return UTIL_STATUS_NO_ERROR;
}
