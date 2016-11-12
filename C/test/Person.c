#include "Person.h"
#include "poolSizes.h"

#include <coll/Map.h>
#include <util/Pool.h>

#include <string.h>

UTIL_POOL_DECLARE( Person )

static utilStatus allocate( Person ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   Person *   This   = NULL;
   UTIL_ALLOCATE( Person );
   return status;
}

utilStatus Person_new(
   Person **    self,
   const char * name,
   const char * forname,
   int          age      )
{
   utilStatus status = allocate( self );
   if( UTIL_STATUS_NO_ERROR == status ) {
      Person * This = *self;
      strncpy( This->name   , name   , PERSON_NAME_MAX_LENGTH );
      strncpy( This->forname, forname, PERSON_NAME_MAX_LENGTH );
      This->age = age;
   }
   return status;
}

static void serialize( const Person * This, ioByteBuffer target ) {
   ioByteBuffer_putString( target, This->forname );
   ioByteBuffer_putString( target, This->name );
   ioByteBuffer_putUShort( target, This->birthdate_year );
   ioByteBuffer_putByte  ( target, This->birthdate_month );
   ioByteBuffer_putByte  ( target, This->birthdate_day );
}

static void unserialize( Person * This, ioByteBuffer source ) {
   ioByteBuffer_getString( source, This->forname, PERSON_NAME_MAX_LENGTH );
   ioByteBuffer_getString( source, This->name   , PERSON_NAME_MAX_LENGTH );
   ioByteBuffer_getUShort( source, &This->birthdate_year );
   ioByteBuffer_getByte  ( source, &This->birthdate_month );
   ioByteBuffer_getByte  ( source, &This->birthdate_day );
}

static utilStatus release( Person ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   UTIL_RELEASE( Person );
   return status;
}

static bool updateBirthdate( Person * person, dcrudArguments how ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   char       tmp[100];

   if(( NULL == person )||( NULL == how )) {
      return false;
   }
   status = dcrudArguments_getString( how, "birthdate", tmp, sizeof( tmp ));
   if( UTIL_STATUS_NO_ERROR == status ) {
      char * s = strtok( tmp, "-" );
      if( s ) {
         unsigned short year = (unsigned short)atoi( s );
         s = strtok( NULL, "-" );
         if( s ) {
            byte month = (byte)atoi( s );
            s = strtok( NULL, "-" );
            if( s ) {
               person->birthdate_day   = (byte)atoi( s );
               person->birthdate_month = month;
               person->birthdate_year  = year;
               return true;
            }
         }
      }
   }
   return false;
}

static utilStatus create( dcrudRemoteFactory * This, dcrudArguments how ) {
   dcrudShareable shareable = NULL;
   Person *       person    = NULL;
   dcrudICache    cache     = NULL;
   char           buffer[100];

   if(( NULL == This )||( NULL == how )) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   CHK(__FILE__,__LINE__,allocate( &person ))
   CHK(__FILE__,__LINE__,updateBirthdate( person, how )?UTIL_STATUS_NO_ERROR:UTIL_STATUS_NULL_ARGUMENT)
   CHK(__FILE__,__LINE__,dcrudArguments_getString( how, "forname", person->forname, PERSON_NAME_MAX_LENGTH ))
   CHK(__FILE__,__LINE__,dcrudArguments_getString( how, "name"   , person->name   , PERSON_NAME_MAX_LENGTH ))
   CHK(__FILE__,__LINE__,dcrudIParticipant_getDefaultCache( This->participant, &cache ))
   CHK(__FILE__,__LINE__,dcrudIParticipant_createShareable( This->participant, This->classID, person, &shareable ))
   CHK(__FILE__,__LINE__,dcrudICache_create( cache, shareable ))
   CHK(__FILE__,__LINE__,dcrudICache_publish( cache ))
   Person_toString( person, buffer, sizeof( buffer ));
   printf( "Person remotely created: %s\n", buffer );
   return UTIL_STATUS_NO_ERROR;
}

static utilStatus update( dcrudRemoteFactory * This, dcrudShareable what, dcrudArguments how ) {
   utilStatus status             = UTIL_STATUS_NO_ERROR;
   Person *   person             = NULL;
   bool       isFornameUpdated   = false;
   bool       isNameUpdated      = false;
   bool       isBirthdateUpdated = false;

   if(( NULL == This )||( NULL == what )||( NULL == how )) {
      return UTIL_STATUS_NULL_ARGUMENT;
   }
   CHK(__FILE__,__LINE__,dcrudShareable_getData( what, (dcrudShareableData *)&person ))
   isFornameUpdated = UTIL_STATUS_NO_ERROR ==
      dcrudArguments_getString( how, "forname", person->forname, PERSON_NAME_MAX_LENGTH );
   isNameUpdated = UTIL_STATUS_NO_ERROR ==
      dcrudArguments_getString( how, "name"   , person->name   , PERSON_NAME_MAX_LENGTH );
   isBirthdateUpdated = updateBirthdate( person, how );
   if( isFornameUpdated || isNameUpdated || isBirthdateUpdated ) {
      char        buffer[100];
      dcrudICache cache = NULL;

      CHK(__FILE__,__LINE__,dcrudIParticipant_getDefaultCache( This->participant, &cache ))
      CHK(__FILE__,__LINE__,dcrudICache_update ( cache, what ))
      CHK(__FILE__,__LINE__,dcrudICache_publish( cache ))
      Person_toString( person, buffer, sizeof( buffer ));
      printf( "Person remotely updated: %s\n", buffer );
   }
   else {
      status = UTIL_STATUS_ILLEGAL_STATE;
   }
   return status;
}

static utilStatus delete( dcrudRemoteFactory * This, dcrudShareable what ) {
   utilStatus  status = UTIL_STATUS_NO_ERROR;
   Person *    person = NULL;
   Person **   self   = &person;
   dcrudICache cache  = NULL;
   char        buffer[100];

   CHK(__FILE__,__LINE__,dcrudShareable_getData( what, (dcrudShareableData *)self ))
   CHK(__FILE__,__LINE__,dcrudIParticipant_getDefaultCache( This->participant, &cache ))
   CHK(__FILE__,__LINE__,dcrudICache_delete ( cache, what ))
   Person_toString( person, buffer, sizeof( buffer ));
   dcrudShareable_detach( what );
   UTIL_RELEASE( Person );
   CHK(__FILE__,__LINE__,dcrudICache_publish( cache ))
   printf( "Person remotely deleted: %s\n", buffer );
   return status;
}

static dcrudLocalFactory  PersonLocalFactory;
static dcrudRemoteFactory PersonRemoteFactory;

utilStatus Person_initFactories(
   dcrudIParticipant     participant,
   dcrudLocalFactory **  localFactory,
   dcrudRemoteFactory ** remoteFactory )
{
   dcrudClassID_resolve( &PersonLocalFactory.classID, 1, 1, 1, 1 );
   PersonLocalFactory.allocateUserData = (dcrudLocalFactory_Allocate   )allocate;
   PersonLocalFactory.serialize        = (dcrudLocalFactory_Serialize  )serialize;
   PersonLocalFactory.unserialize      = (dcrudLocalFactory_Unserialize)unserialize;
   PersonLocalFactory.releaseUserData  = (dcrudLocalFactory_Release    )release;
   CHK(__FILE__,__LINE__,dcrudIParticipant_registerLocalFactory( participant, &PersonLocalFactory  ))

   PersonRemoteFactory.classID     = PersonLocalFactory.classID;
   PersonRemoteFactory.userContext = NULL;
   PersonRemoteFactory.participant = participant;
   PersonRemoteFactory.create      = (dcrudRemoteFactory_create)create;
   PersonRemoteFactory.update      = (dcrudRemoteFactory_update)update;
   PersonRemoteFactory.delete      = (dcrudRemoteFactory_delete)delete;
   CHK(__FILE__,__LINE__,dcrudIParticipant_registerRemoteFactory( participant, &PersonRemoteFactory ))
   *localFactory  = &PersonLocalFactory;
   *remoteFactory = &PersonRemoteFactory;
   return UTIL_STATUS_NO_ERROR;
}

utilStatus Person_releaseFactories( void ) {
   CHK(__FILE__,__LINE__,dcrudClassID_delete( &PersonLocalFactory.classID ))
   return UTIL_STATUS_NO_ERROR;
}

utilStatus Person_print( collForeach * context ) {
   FILE *               target    = (FILE *              )context->user;
   const dcrudGUID      key       = (const dcrudGUID     )context->key;
   const dcrudShareable shareable = (const dcrudShareable)context->value;
   const Person *       person    = NULL;

   CHK(__FILE__,__LINE__,dcrudShareable_getData( shareable, (dcrudShareableData *)&person ))
   if( key ) {
      char szGUID[27];
      CHK(__FILE__,__LINE__,dcrudGUID_toString( key, szGUID, sizeof( szGUID ) ))
      fprintf( target, "%s ==> %s %s %d-%d-%d\n",
         szGUID,
         person->forname,
         person->name,
         person->birthdate_year,
         person->birthdate_month,
         person->birthdate_day    );
   }
   else {
      fprintf( target, "%s %s %d-%d-%d\n",
         person->forname,
         person->name,
         person->birthdate_year,
         person->birthdate_month,
         person->birthdate_day    );
   }
   return UTIL_STATUS_NO_ERROR;
}

utilStatus Person_toString( const Person * person, char * buffer, size_t bufferSize ) {
   size_t result = snprintf( buffer, bufferSize, "%s %s %d-%d-%d",
      person->forname,
      person->name,
      person->birthdate_year,
      person->birthdate_month,
      person->birthdate_day    );
   return ( result < bufferSize ) ? UTIL_STATUS_NO_ERROR : UTIL_STATUS_OVERFLOW;
}

int Person_fornameCmp( const Person ** left, const Person ** right ) {
   return strncmp( (*left)->forname, (*right)->forname, PERSON_NAME_MAX_LENGTH );
}

int Person_cmp( const Person ** left, const Person ** right ) {
   int cmp = strncmp( (*left)->name   , (*right)->name   , PERSON_NAME_MAX_LENGTH );
   if( 0 == cmp ) {
      cmp  = strncmp( (*left)->forname, (*right)->forname, PERSON_NAME_MAX_LENGTH );
   }
   return cmp;
}

utilStatus Person_delete( Person ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   UTIL_RELEASE( Person );
   return status;
}
