/*
 *
 * TEST 6
 *
 */
#include <os/System.h>
#include <util/CmdLine.h>
#include <io/NetworkInterfaces.h>
#include "Person.h"
#include "Settings.h"
#include "StaticRegistry.h"

#include <stdio.h>
#include <ctype.h>
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

static utilStatus getFirstEntry( collForeach * context ) {
   dcrudShareable * first = context->user;
   *first = context->value;
   return UTIL_STATUS_ILLEGAL_STATE;
}

static utilStatus remotelyDeleteFirstEntry( dcrudICache cache, dcrudICRUD personCRUD ) {
   dcrudShareable first = NULL;
   dcrudICache_foreach( cache, getFirstEntry, &first );
   if( first ) {
      CHK(__FILE__,__LINE__,dcrudICRUD_delete( personCRUD, first ));
   }
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
   dcrudIRegistry       registry = NULL;

   (void)cmdLine;
   CHK(__FILE__,__LINE__,getStaticRegistry( &registry ));
   CHK(__FILE__,__LINE__,ioInetSocketAddress_init( &p1, MCAST_ADDRESS, 2417 ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_new( &participant, 2, &p1, NETWORK_INTERFACE ));
   CHK(__FILE__,__LINE__,ioInetSocketAddress_init( &p2, MCAST_ADDRESS, 2416 ));
   CHK(__FILE__,__LINE__,Person_initFactories( participant, &localFactory, &remoteFactory ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_listen( participant, registry, NETWORK_INTERFACE ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_getDefaultCache( participant, &cache ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_getDispatcher( participant, &dispatcher ));
   CHK(__FILE__,__LINE__,dcrudIDispatcher_requireCRUD( dispatcher, localFactory->classID, &personCRUD ));
   CHK(__FILE__,__LINE__,dcrudIDispatcher_require( dispatcher, "IMonitor", &monitor ));

   for( i = 0; c != 'Q'; ++i ) {
      printf( "[A]ubin, [M]uriel, [E]ve, [C]ache, [D]elete first, [Q]uit: " );
      c = toupper((char)fgetc( stdin ));
      printf( "%c\n", c );
      switch(c) {
      case 'Q':
         CHK(__FILE__,__LINE__,dcrudIRequired_call( monitor, "exit", NULL, NULL ))
         break;
      case 'A':
         CHK(__FILE__,__LINE__,remotelyCreatePerson( personCRUD, "Aubin", "Mahé", "1966-01-24" ))
         break;
      case 'M':
         CHK(__FILE__,__LINE__,remotelyCreatePerson( personCRUD, "Muriel", "Le Nain", "1973-01-26" ))
         break;
      case 'E':
         CHK(__FILE__,__LINE__,remotelyCreatePerson( personCRUD, "Eve", "Mahé", "2008-02-28" ))
         break;
      case 'D':
         CHK(__FILE__,__LINE__,remotelyDeleteFirstEntry( cache, personCRUD ))
         break;
      case 'C':
         printf( "Cache content:\n" );
         CHK(__FILE__,__LINE__,dcrudICache_refresh( cache ))
         CHK(__FILE__,__LINE__,dcrudICache_foreach( cache, Person_print, stdout ))
         break;
      }
      CHK(__FILE__,__LINE__,dcrudIDispatcher_handleRequests( dispatcher ))
   }
   CHK(__FILE__,__LINE__,dcrudIParticipant_leave( participant ));
   CHK(__FILE__,__LINE__,dcrudIParticipant_delete( &participant ));
   CHK(__FILE__,__LINE__,releaseStaticRegistry( &registry ))
   CHK(__FILE__,__LINE__,Person_releaseFactories())
   return UTIL_STATUS_NO_ERROR;
}
