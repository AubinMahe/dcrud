#include <dcrud/Network.h>
#include <os/System.h>
#include <util/CheckSysCall.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#  include <sys/socket.h>
#  include <netdb.h>
#  include <time.h>
#  include <inttypes.h>
#elif _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
/*#  include <Ws2tcpip.h>*/
#  include <mswsock.h>
#endif

#define NETWORK_INTERFACE  "192.168.1.7"
#define MCAST_ADDRESS      "224.0.0.3"

typedef struct Person_s {

   char           forname[40];
   char           name[40];
   unsigned short birthdate_year;
   byte           birthdate_month;
   byte           birthdate_day;

} Person;

static void Person_set( Person * This, const Person * source ) {
   strncpy( This->forname, source->forname, 40 );
   strncpy( This->name   , source->name, 40 );
   This->birthdate_year  = source->birthdate_year;
   This->birthdate_month = source->birthdate_month;
   This->birthdate_day   = source->birthdate_day;
}

static void Person_serialize( const Person * This, ioByteBuffer target ) {
   ioByteBuffer_putString( target, This->forname );
   ioByteBuffer_putString( target, This->name );
   ioByteBuffer_putShort ( target, This->birthdate_year );
   ioByteBuffer_putByte  ( target, This->birthdate_month );
   ioByteBuffer_putByte  ( target, This->birthdate_day );
}

static void Person_unserialize( Person * This, ioByteBuffer source ) {
   ioByteBuffer_getString( source, This->forname, 40 );
   ioByteBuffer_getString( source, This->name   , 40 );
   ioByteBuffer_getShort ( source, &This->birthdate_year );
   ioByteBuffer_getByte  ( source, &This->birthdate_month );
   ioByteBuffer_getByte  ( source, &This->birthdate_day );
}

static bool Person_init( dcrudShareable shareable ) {
   (void)shareable;
   return true;
}

static bool printPerson( collForeach * context ) {
   collMapPair *  pair      = (collMapPair *)context->item;
   dcrudShareable shareable = (dcrudShareable)pair->value;
   Person *       person    = (Person *)dcrudShareable_getUserData( shareable );
   printf( "%s %s %d-%d-%d\n",
      person->forname,
      person->name,
      person->birthdate_year,
      person->birthdate_month,
      person->birthdate_day    );
   return true;
}

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
      dcrudIFactory      personFactory;
      dcrudICache        cache            = NULL;
      dcrudIDispatcher   dispatcher       = NULL;
      dcrudICRUD         personCRUD       = NULL;
      dcrudIRequired     monitor          = NULL;
      dcrudCounterpart   firstCounterpart = { MCAST_ADDRESS, 2416 };
      dcrudCounterpart * counterparts[]   = {
         &firstCounterpart,
         NULL
      };

      personFactory.classID     = dcrudClassID_new( 1, 1, 1, 1 );
      personFactory.size        = sizeof( Person );
      personFactory.initialize  = (dcrudShareable_Initialize )Person_init;
      personFactory.set         = (dcrudShareable_Set        )Person_set;
      personFactory.serialize   = (dcrudShareable_Serialize  )Person_serialize;
      personFactory.unserialize = (dcrudShareable_Unserialize)Person_unserialize;
      dcrudIParticipant_registerFactory( participant, &personFactory );
      dcrudIParticipant_listen( participant, NETWORK_INTERFACE, counterparts );

      cache      = dcrudIParticipant_getDefaultCache( participant );
      dispatcher = dcrudIParticipant_getDispatcher( participant );
      personCRUD = dcrudIDispatcher_requireCRUD( dispatcher, personFactory.classID );
      monitor    = dcrudIDispatcher_require( dispatcher, "IMonitor" );

      printf( "Ready to ask Java process for a Person creation, press <enter> please...\n" );
      fgetc( stdin );
      remotelyCreatePerson( personCRUD, "Aubin", "Mahé", "1966-01-24" );
      osSystem_sleep( 1000U );
      dcrudIDispatcher_handleRequests( dispatcher );
      dcrudICache_refresh( cache );
      printf( "Cache content:\n" );
      dcrudICache_foreach( cache, printPerson, NULL );
      printf( "Press <enter> to terminate Java counterpart\n" );
      fgetc( stdin );
      dcrudIRequired_call( monitor, "exit", NULL, NULL );
      printf( "Well done, press <enter> to exit\n" );
      fgetc( stdin );
      dcrudIParticipant_delete( &participant );
   }
   else {
      fprintf( stderr, "Unable to join network.\n" );
   }
}
