#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <dcrud/IParticipant.h>

#define PERSON_NAME_MAX_LENGTH 40

typedef struct Person_s {

   char           forname[PERSON_NAME_MAX_LENGTH];
   char           name   [PERSON_NAME_MAX_LENGTH];
   unsigned       age;
   unsigned short birthdate_year;
   byte           birthdate_month;
   byte           birthdate_day;

} Person;

utilStatus Person_new(
   Person **    This,
   const char * name,
   const char * forname,
   int          age      );

utilStatus Person_initFactories(
   dcrudIParticipant     participant,
   dcrudLocalFactory **  localFactory,
   dcrudRemoteFactory ** remoteFactory );

utilStatus Person_releaseFactories( void );

utilStatus Person_print( collForeach * context );

utilStatus Person_toString( const Person * This, char * buffer, size_t bufferSize );

utilStatus Person_delete( Person ** This );

int Person_fornameCmp( const Person ** left, const Person ** right );

int Person_cmp( const Person ** left, const Person ** right );

#ifdef __cplusplus
}
#endif
