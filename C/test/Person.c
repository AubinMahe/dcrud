#include "Person.h"
#include <coll/Map.h>

static void set( Person * This, const Person * source ) {
   strncpy( This->forname, source->forname, PERSON_NAME_MAX_LENGTH );
   strncpy( This->name   , source->name   , PERSON_NAME_MAX_LENGTH );
   This->forname[PERSON_NAME_MAX_LENGTH-1] = '\0';
   This->name   [PERSON_NAME_MAX_LENGTH-1] = '\0';
   This->birthdate_year  = source->birthdate_year;
   This->birthdate_month = source->birthdate_month;
   This->birthdate_day   = source->birthdate_day;
}

static void serialize( const Person * This, ioByteBuffer target ) {
   ioByteBuffer_putString( target, This->forname );
   ioByteBuffer_putString( target, This->name );
   ioByteBuffer_putShort ( target, This->birthdate_year );
   ioByteBuffer_putByte  ( target, This->birthdate_month );
   ioByteBuffer_putByte  ( target, This->birthdate_day );
}

static void unserialize( Person * This, ioByteBuffer source ) {
   ioByteBuffer_getString( source, This->forname, PERSON_NAME_MAX_LENGTH );
   ioByteBuffer_getString( source, This->name   , PERSON_NAME_MAX_LENGTH );
   ioByteBuffer_getShort ( source, &This->birthdate_year );
   ioByteBuffer_getByte  ( source, &This->birthdate_month );
   ioByteBuffer_getByte  ( source, &This->birthdate_day );
}

static bool init( dcrudShareable shareable ) {
   (void)shareable;
   return true;
}

static void create( dcrudRemoteFactory * This, dcrudArguments how ) {
   char   birthdate[12];
   char * s;

   if( dcrudArguments_getString( how, "birthdate", birthdate, sizeof( birthdate ))) {
      s = strtok( birthdate, "-" );
      if( s ) {
         unsigned short year = (unsigned short)atoi( s );
         s = strtok( NULL, "-" );
         if( s ) {
            byte month = (byte)atoi( s );
            s = strtok( NULL, "-" );
            if( s ) {
               dcrudShareable shareable =
                  dcrudIParticipant_createShareable( This->participant, This->classID );
               Person *       person    = (Person *)dcrudShareable_getUserData( shareable );
               if(   dcrudArguments_getString( how, "forname", person->forname, PERSON_NAME_MAX_LENGTH )
                  && dcrudArguments_getString( how, "name"   , person->name   , PERSON_NAME_MAX_LENGTH ))
               {
                  dcrudICache cache = dcrudIParticipant_getDefaultCache( This->participant );
                  person->birthdate_day   = (byte)atoi( s );
                  person->birthdate_month = month;
                  person->birthdate_year  = year;
                  dcrudICache_create( cache, shareable );
                  dcrudICache_publish( cache );
                  printf( "Person remotely created: %s %s %02d/%02d/%4d\n",
                     person->forname, person->name,
                     person->birthdate_day, person->birthdate_month, person->birthdate_year );
               }
            }
         }
      }
   }
}

static void update( dcrudRemoteFactory * This, dcrudShareable what, dcrudArguments how ) {
   char   birthdate[12];
   char * s;

   if( dcrudArguments_getString( how, "birthdate", birthdate, sizeof( birthdate ))) {
      s = strtok( birthdate, "-" );
      if( s ) {
         unsigned short year = (unsigned short)atoi( s );
         s = strtok( NULL, "-" );
         if( s ) {
            byte month = (byte)atoi( s );
            s = strtok( NULL, "-" );
            if( s ) {
               Person * person = (Person *)dcrudShareable_getUserData( what );
               if( dcrudArguments_getString( how, "forname", person->forname, PERSON_NAME_MAX_LENGTH )) {
                  dcrudICache cache = dcrudIParticipant_getDefaultCache( This->participant );
                  person->birthdate_day   = (byte)atoi( s );
                  person->birthdate_month = month;
                  person->birthdate_year  = year;
                  dcrudICache_update( cache, what );
                  dcrudICache_publish( cache );
                  printf( "Person remotely updated: %s %s %02d/%02d/%4d\n",
                     person->forname, person->name,
                     person->birthdate_day, person->birthdate_month, person->birthdate_year );
               }
            }
         }
      }
   }
}

static void delete( dcrudRemoteFactory * This, dcrudShareable what ) {
   Person *    person = (Person *)dcrudShareable_getUserData( what );
   dcrudICache cache  = dcrudIParticipant_getDefaultCache( This->participant );
   dcrudICache_delete ( cache, what );
   dcrudICache_publish( cache );
   printf( "Person remotely deleted: %s %s %02d/%02d/%4d\n",
      person->forname, person->name,
      person->birthdate_day, person->birthdate_month, person->birthdate_year );
}

void Person_initFactories(
   dcrudIParticipant     participant,
   dcrudLocalFactory **  localFactory,
   dcrudRemoteFactory ** remoteFactory )
{
   static dcrudLocalFactory  PersonLocalFactory;
   static dcrudRemoteFactory PersonRemoteFactory;

   PersonLocalFactory.classID     = dcrudClassID_new( 1, 1, 1, 1 );
   PersonLocalFactory.size        = sizeof( Person );
   PersonLocalFactory.initialize  = (dcrudLocalFactory_Initialize )init;
   PersonLocalFactory.set         = (dcrudLocalFactory_Set        )set;
   PersonLocalFactory.serialize   = (dcrudLocalFactory_Serialize  )serialize;
   PersonLocalFactory.unserialize = (dcrudLocalFactory_Unserialize)unserialize;
   dcrudIParticipant_registerLocalFactory ( participant, &PersonLocalFactory  );

   PersonRemoteFactory.classID     = PersonLocalFactory.classID;
   PersonRemoteFactory.userContext = NULL;
   PersonRemoteFactory.participant = participant;
   PersonRemoteFactory.create      = (dcrudRemoteFactory_create)create;
   PersonRemoteFactory.update      = (dcrudRemoteFactory_update)update;
   PersonRemoteFactory.delete      = (dcrudRemoteFactory_delete)delete;
   dcrudIParticipant_registerRemoteFactory( participant, &PersonRemoteFactory );
   *localFactory  = &PersonLocalFactory;
   *remoteFactory = &PersonRemoteFactory;
}

bool Person_print( collForeach * context ) {
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
