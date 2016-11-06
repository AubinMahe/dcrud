/*
 *
 * TEST 2
 *
 */
#include "Person.h"

#include <coll/List.h>
#include <coll/Map.h>
#include <coll/Set.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static utilStatus printPerson( collForeach * context ) {
   const char *   test   = (const char *  )context->user;
   const Person * person = (const Person *)context->value;
   if( context->key ) {
      if( test[3] == '1' ) {
         const char * key = (const char * )context->key;
         printf( "|%s|    |%d: %s -> { %s, %s, %d }\n",
            test, context->index, key, person->forname, person->name, person->age );
      }
      else {
         const unsigned * key = (const unsigned *)context->key;
         printf( "|%s|    |%d: %d -> { %s, %s, %d }\n",
            test, context->index, *key, person->forname, person->name, person->age );
      }
   }
   else {
      printf( "|%s|    |%d: { %s, %s, %d }\n",
         test, context->index, person->forname, person->name, person->age );
   }
   context->retVal = NULL;
   return UTIL_STATUS_NO_ERROR;
}

utilStatus coll( void ) {
   utilStatus  status = UTIL_STATUS_NO_ERROR;
   Person *    aubin  = NULL;
   Person *    muriel = NULL;
   Person *    eve    = NULL;
   Person *    person;
   collMapPair previous;
   unsigned    size;

   CHK(__FILE__,__LINE__,Person_new( &aubin , "Mahe"   , "Aubin" , 49 ));
   CHK(__FILE__,__LINE__,Person_new( &muriel, "Le Nain", "Muriel", 42 ));
   CHK(__FILE__,__LINE__,Person_new( &eve   , "Mahe"   , "Eve"   ,  7 ));
   printf( "+----+----+----------------------------------------\n" );
   {
      collList persons;
      unsigned count;

      CHK(__FILE__,__LINE__,collList_new( &persons ));
      CHK(__FILE__,__LINE__,collList_add( persons, muriel ));
      CHK(__FILE__,__LINE__,collList_add( persons, aubin ));
      CHK(__FILE__,__LINE__,collList_add( persons, eve ));
      CHK(__FILE__,__LINE__,collList_foreach( persons, printPerson, "List", NULL ));
      status = collList_remove( persons, aubin );
      if( UTIL_STATUS_NO_ERROR == status ) {
         printf( "|List|OK  |remove\n" );
      }
      else {
         printf( "|List|FAIL|remove: %s\n", utilStatusMessages[status] );
      }
      CHK(__FILE__,__LINE__,collList_size( persons, &count ));
      if( count == 2 ) {
         printf( "|List|OK  |remove: size() == 2\n" );
      }
      else {
         printf( "|List|FAIL|remove: size() != 2\n" );
      }
      CHK(__FILE__,__LINE__,collList_foreach( persons, printPerson, "List", NULL ));
      CHK(__FILE__,__LINE__,collList_delete( &persons ));
   }
   printf( "+----+----+----------------------------------------\n" );
   {
      unsigned size;
      collSet persons;
      CHK(__FILE__,__LINE__,collSet_new( &persons, (collComparator)Person_fornameCmp ));
      CHK(__FILE__,__LINE__,collSet_add( persons, muriel ));
      CHK(__FILE__,__LINE__,collSet_add( persons, aubin ));
      CHK(__FILE__,__LINE__,collSet_add( persons, eve ));
      CHK(__FILE__,__LINE__,collSet_foreach( persons, printPerson, "Set ", NULL ));
      status = collSet_add( persons, aubin );
      if( UTIL_STATUS_DUPLICATE == status ) {
         printf( "|Set |OK  |add: doublon refused\n" );
      }
      else {
         printf( "|Set |FAIL|add: doublon accepted\n" );
      }
      status = collSet_remove( persons, aubin );
      if( UTIL_STATUS_NO_ERROR == status ) {
         printf( "|Set |OK  |remove\n" );
      }
      else {
         printf( "|Set |FAIL|remove\n" );
      }
      status = collSet_size( persons, &size );
      if( size == 2 ) {
         printf( "|Set |OK  |remove\n" );
      }
      else {
         printf( "|Set |FAIL|remove: size() != 2\n" );
      }
      CHK(__FILE__,__LINE__,collSet_foreach( persons, printPerson, "Set ", NULL ));
      CHK(__FILE__,__LINE__,collSet_delete( &persons ));
   }
   printf( "+----+----+----------------------------------------\n" );
   {
      collMap persons;
      CHK(__FILE__,__LINE__,collMap_new( &persons, (collComparator)collStringCompare ));
      CHK(__FILE__,__LINE__,collMap_put( persons, muriel->forname, muriel, &previous ));
      if( UTIL_STATUS_DUPLICATE == status ) {
         printf( "|Map1|FAIL|put: entry already exists in an empty map!\n" );
      }
      else {
         printf( "|Map1|OK  |put\n" );
      }
      status = collMap_put( persons, aubin ->forname, aubin, &previous );
      if( UTIL_STATUS_DUPLICATE == status ) {
         printf( "|Map1|FAIL|put: entry already exists in an empty map!\n" );
      }
      else {
         printf( "|Map1|OK  |put\n" );
      }
      status = collMap_put( persons, eve   ->forname, eve, &previous );
      if( UTIL_STATUS_DUPLICATE == status ) {
         printf( "|Map1|FAIL|put: entry already exists in an empty map!\n" );
      }
      else {
         printf( "|Map1|OK  |put\n" );
      }
      CHK(__FILE__,__LINE__,collMap_foreach( persons, printPerson, "Map1", NULL ));
      CHK(__FILE__,__LINE__,collMap_get( persons, "Eve", &person ));
      if( UTIL_STATUS_NOT_FOUND == status ) {
         printf( "|Map1|FAIL|get: not found!\n" );
      }
      else if( person == eve ) {
         printf( "|Map1|OK  |get\n" );
      }
      else {
         printf( "|Map1|FAIL|get: { %s, %s, %d }\n",
            person->forname, person->name, person->age );
      }
      CHK(__FILE__,__LINE__,collMap_put( persons, "Aubin", aubin, &previous ));
      if( previous.value != aubin ) {
         printf( "|Map1|FAIL|put: previous entry is not aubin!\n" );
      }
      else {
         printf( "|Map1|OK  |put: previous is aubin\n" );
      }
      status = collMap_remove( persons, "Aubin", &previous );
      if( UTIL_STATUS_NO_ERROR == status ) {
         printf( "|Map1|OK  |remove\n" );
         if( previous.value != aubin ) {
            person = previous.value;
            if( person ) {
               printf( "|Map1|FAIL|remove: previous entry is not aubin: { %s, %s, %d }!\n",
                  person->forname, person->name, person->age );
            }
            else {
               printf( "|Map1|FAIL|remove: previous entry is null!\n" );
            }
         }
         else {
            printf( "|Map1|OK  |remove: previous is aubin\n" );
         }
      }
      else {
         printf( "|Map1|FAIL|remove: %s\n", utilStatusMessages[status] );
      }
      CHK(__FILE__,__LINE__,collMap_size( persons, &size ));
      if( size == 2 ) {
         printf( "|Map1|OK  |remove: size == 2\n" );
      }
      else {
         printf( "|Map1|FAIL|remove: size() != 2\n" );
      }
      CHK(__FILE__,__LINE__,collMap_foreach( persons, printPerson, "Map1", NULL ));
      CHK(__FILE__,__LINE__,collMap_delete( &persons ));
   }
   printf( "+----+----+----------------------------------------\n" );
   {
      collMap persons;
      static unsigned keyMuriel = 12;
      static unsigned keyAubin  = 24;
      static unsigned keyEve    = 36;

      CHK(__FILE__,__LINE__,collMap_new( &persons, (collComparator)collUintCompare ));
      CHK(__FILE__,__LINE__,collMap_put( persons, &keyMuriel, muriel, &previous ));
      CHK(__FILE__,__LINE__,collMap_put( persons, &keyAubin , aubin, &previous ));
      CHK(__FILE__,__LINE__,collMap_put( persons, &keyEve   , eve, &previous ));
      CHK(__FILE__,__LINE__,collMap_foreach( persons, printPerson, "Map2", NULL ));
      status = collMap_get( persons, &keyEve, &person );
      if( UTIL_STATUS_NO_ERROR == status ) {
         if( person == eve ) {
            printf( "|Map2|OK  |get\n" );
         }
         else {
            printf( "|Map2|FAIL|get: found but not as expected { %s, %s, %d }\n",
               person->forname, person->name, person->age );
         }
      }
      else {
         printf( "|Map2|FAIL|get: not found!\n" );
      }
      status = collMap_put( persons, &keyAubin, aubin, &previous );
      if( previous.value == aubin ) {
         printf( "|Map2|OK  |put: previous association replaced\n" );
      }
      else {
         person = (Person *)previous.value;
         printf( "|Map2|FAIL|put: { %s, %s, %d }\n",
            person->forname, person->name, person->age );
      }
      status = collMap_remove( persons, &keyAubin, &previous );
      if(( UTIL_STATUS_NO_ERROR == status )&&( previous.value == aubin )) {
         printf( "|Map2|OK  |remove\n" );
      }
      else {
         printf( "|Map2|FAIL|remove: unexpected removed item\n" );
      }
      CHK(__FILE__,__LINE__,collMap_size( persons, &size ));
      if( size == 2 ) {
         printf( "|Map2|OK  |remove\n" );
      }
      else {
         printf( "|Map2|FAIL|remove: size() != 2\n" );
      }
      CHK(__FILE__,__LINE__,collMap_foreach( persons, printPerson, "Map2", NULL ));
      CHK(__FILE__,__LINE__,collMap_delete( &persons ));
   }
   printf( "+----+----+----------------------------------------\n" );
   CHK(__FILE__,__LINE__,Person_delete( &aubin ));
   CHK(__FILE__,__LINE__,Person_delete( &muriel ));
   CHK(__FILE__,__LINE__,Person_delete( &eve ));
   return status;
}
