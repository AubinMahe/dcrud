#pragma once

#include <dcrud/IParticipant.h>

#define PERSON_NAME_MAX_LENGTH 40

typedef struct Person_s {

   char           forname[PERSON_NAME_MAX_LENGTH];
   char           name   [PERSON_NAME_MAX_LENGTH];
   unsigned short birthdate_year;
   byte           birthdate_month;
   byte           birthdate_day;

} Person;

void Person_initFactories(
   dcrudIParticipant     participant,
   dcrudLocalFactory **  localFactory,
   dcrudRemoteFactory ** remoteFactory );

bool Person_print( collForeach * context );
