#include <coll/List.h>
#include <coll/Map.h>
#include <coll/Set.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char name_t[40];

typedef struct Person_s {

   name_t name;
   name_t forname;
   int    age;

} Person;

static Person * Person_create(
   const char * name,
   const char * forname,
   int          age      )
{
   Person * person = (Person *)malloc( sizeof( Person ));
   memset( person, 0 , sizeof( Person ));
   strncpy( person->name, name, sizeof( person->name ));
   strncpy( person->forname, forname, sizeof( person->forname ));
   person->age = age;
   return person;
}

static int unsignedMapComparator( const unsigned * * left, const unsigned * * right ) {
   return (int)( *left - *right );
}

static int personComparator( const Person * * left, const Person * * right ) {
   const char * l = (*left )->forname;
   const char * r = (*right)->forname;
   return strncmp( l, r, sizeof( name_t ));
}

static bool printPerson( collForeach * context ) {
   const Person * person = (const Person *)context->item;
   const char *   test   = (const char *  )context->user;
   printf( "|%s|    |%d: { %s, %s, %d }\n",
      test, context->index, person->forname, person->name, person->age );
   context->retVal = NULL;
   return true;
}

static bool printPersonEntry( collForeach * context ) {
   collMapPair * pair   = (collMapPair *)context->item;
   const char *         test   = (const char *        )context->user;
   const char   *       key    = (const char *        )pair->key;
   const Person *       person = (const Person *      )pair->value;
   printf( "|%s|    |%d: %s -> { %s, %s, %d }\n",
      test, context->index, key, person->forname, person->name, person->age );
   context->retVal = NULL;
   return true;
}

static bool printPersonEntry2( collForeach * context ) {
   collMapPair * pair   = (collMapPair *)context->item;
   const char *         test   = (const char *        )context->user;
   const char   *       key    = (const char *        )pair->key;
   const Person *       person = (const Person *      )pair->value;
   printf( "|%s|    |%d: %u -> { %s, %s, %d }\n",
      test, context->index, *key, person->forname, person->name, person->age );
   context->retVal = NULL;
   return true;
}

void test_002( void ) {
   Person *    aubin  = Person_create( "Mahe"   , "Aubin" , 49 );
   Person *    muriel = Person_create( "Le Nain", "Muriel", 42 );
   Person *    eve    = Person_create( "Mahe"   , "Eve"   ,  7 );
   bool        add;
   bool        removed;
   Person *    person;
   bool        alreadyExists;
   collMapPair previous;
   printf( "+----+----+----------------------------------------\n" );
   {
      collList persons = collList_new();
      collList_add( persons, muriel );
      collList_add( persons, aubin );
      collList_add( persons, eve );
      collList_foreach( persons, printPerson, "List" );
      removed = collList_remove( persons, aubin );
      if( removed ) {
         printf( "|List|OK  |remove\n" );
      }
      else {
         printf( "|List|FAIL|remove\n" );
      }
      if( collList_size( persons ) == 2 ) {
         printf( "|List|OK  |remove: size() == 2\n" );
      }
      else {
         printf( "|List|FAIL|remove: size() != 2\n" );
      }
      collList_foreach( persons, printPerson, "List" );
      collList_delete( &persons );
   }
   printf( "+----+----+----------------------------------------\n" );
   {
      collSet persons = collSet_new((collComparator)personComparator );
      collSet_add( persons, muriel );
      collSet_add( persons, aubin );
      collSet_add( persons, eve );
      collSet_foreach( persons, printPerson, "Set " );
      add = collSet_add( persons, aubin );
      if( add == false ) {
         printf( "|Set |OK  |add: doublon refused\n" );
      }
      else {
         printf( "|Set |FAIL|add: doublon accepted\n" );
      }
      removed = collSet_remove( persons, aubin );
      if( removed ) {
         printf( "|Set |OK  |remove\n" );
      }
      else {
         printf( "|Set |FAIL|remove\n" );
      }
      if( collSet_size( persons ) == 2 ) {
         printf( "|Set |OK  |remove\n" );
      }
      else {
         printf( "|Set |FAIL|remove: size() != 2\n" );
      }
      collSet_foreach( persons, printPerson, "Set " );
      collSet_delete( &persons );
   }
   printf( "+----+----+----------------------------------------\n" );
   {
      collMap persons = collMap_new((collComparator)collStringComparator );
      alreadyExists = collMap_put( persons, muriel->forname, muriel, &previous );
      if( alreadyExists ) {
         printf( "|Map |FAIL|put: entry already exists in an empty map!\n" );
      }
      else {
         printf( "|Map |OK  |put\n" );
      }
      alreadyExists = collMap_put( persons, aubin ->forname, aubin, &previous );
      if( alreadyExists ) {
         printf( "|Map |FAIL|put: entry already exists in an empty map!\n" );
      }
      else {
         printf( "|Map |OK  |put\n" );
      }
      alreadyExists = collMap_put( persons, eve   ->forname, eve, &previous );
      if( alreadyExists ) {
         printf( "|Map |FAIL|put: entry already exists in an empty map!\n" );
      }
      else {
         printf( "|Map |OK  |put\n" );
      }
      collMap_foreach( persons, printPersonEntry, "Map " );
      person = collMap_get( persons, "Eve" );
      if( person == NULL ) {
         printf( "|Map |FAIL|get: not found!\n" );
      }
      else if( person == eve ) {
         printf( "|Map |OK  |get\n" );
      }
      else {
         printf( "|Map |FAIL|get: { %s, %s, %d }\n",
            person->forname, person->name, person->age );
      }
      alreadyExists = collMap_put( persons, "Aubin", aubin, &previous );
      if( alreadyExists ) {
         printf( "|Map |OK  |put: previous entry already exist\n" );
         if( previous.value != aubin ) {
            printf( "|Map |FAIL|put: previous entry is not aubin!\n" );
         }
         else {
            printf( "|Map |OK  |put: previous is aubin\n" );
         }
      }
      else {
         printf( "|Map |FAIL|put: { %s, %s, %d }\n",
            person->forname, person->name, person->age );
      }
      alreadyExists = collMap_remove( persons, "Aubin", &previous );
      if( alreadyExists ) {
         printf( "|Map |OK  |remove\n" );
         if( previous.value != aubin ) {
            printf( "|Map |FAIL|remove: previous entry is not aubin!\n" );
         }
         else {
            printf( "|Map |OK  |remove: previous is aubin\n" );
         }
      }
      else {
         printf( "|Map |FAIL|remove: not found\n" );
      }
      if( collMap_size( persons ) == 2 ) {
         printf( "|Map |OK  |remove: size == 2\n" );
      }
      else {
         printf( "|Map |FAIL|remove: size() != 2\n" );
      }
      collMap_foreach( persons, printPersonEntry, "Map " );
      collMap_delete( &persons );
   }
   printf( "+----+----+----------------------------------------\n" );
   {
      collMap persons = collMap_new((collComparator)unsignedMapComparator );
      static unsigned keyMuriel = 12;
      static unsigned keyAubin  = 24;
      static unsigned keyEve    = 36;

      collMap_put( persons, &keyMuriel, muriel, &previous );
      collMap_put( persons, &keyAubin , aubin, &previous );
      collMap_put( persons, &keyEve   , eve, &previous );
      collMap_foreach( persons, printPersonEntry2, "Map " );
      person = collMap_get( persons, &keyEve );
      if( person == NULL ) {
         printf( "|Map |FAIL|get: not found!\n" );
      }
      else if( person == eve ) {
         printf( "|Map |OK  |get\n" );
      }
      else {
         printf( "|Map |FAIL|get: { %s, %s, %d }\n",
            person->forname, person->name, person->age );
      }
      alreadyExists = collMap_put( persons, &keyAubin, aubin, &previous );
      if( ! alreadyExists ) {
         printf( "|Map |FAIL|put: doublon accepted!\n" );
      }
      else if( previous.value == aubin ) {
         printf( "|Map |OK  |put: doublon refused\n" );
      }
      else {
         person = (Person *)previous.value;
         printf( "|Map |FAIL|put: { %s, %s, %d }\n",
            person->forname, person->name, person->age );
      }
      alreadyExists = collMap_remove( persons, &keyAubin, &previous );
      if( alreadyExists && previous.value == aubin ) {
         printf( "|Map |OK  |remove\n" );
      }
      else {
         printf( "|Map |FAIL|remove: unexpected removed item\n" );
      }
      if( collMap_size( persons ) == 2 ) {
         printf( "|Map |OK  |remove\n" );
      }
      else {
         printf( "|Map |FAIL|remove: size() != 2\n" );
      }
      collMap_foreach( persons, printPersonEntry2, "Map " );
      collMap_delete( &persons );
   }
   printf( "+----+----+----------------------------------------\n" );
   free( aubin );
   free( muriel );
   free( eve );
}
