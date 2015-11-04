#include <crtdbg.h>

#include <coll/List.h>
#include <coll/Set.h>
#include <coll/Map.h>

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

static int personMapCompare( const char * * left, const char * * right ) {
   return strncmp( *left, *right, sizeof( name_t ));
}

static int personCompare( const Person * * left, const Person * * right ) {
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

static bool printPersonEntry( collForeachMap * context ) {
   const char   * forname = (const char *  )context->key;
   const Person * person  = (const Person *)context->value;
   const char *   test    = (const char *  )context->user;
   printf( "|%s|    |%d: %s -> { %s, %s, %d }\n",
      test, context->index, forname, person->forname, person->name, person->age );
   context->retVal = NULL;
   return true;
}
#ifdef COLLECTIONS_UNIT_TEST
int main( void ) {
   _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
   Person * aubin  = Person_create( "Mahe", "Aubin", 49 );
   Person * muriel = Person_create( "Le Nain", "Muriel", 42 );
   Person * eve    = Person_create( "Mahe"   , "Eve"   ,  7 );
   printf( "+----+----+----------------------------------------\n" );
   {
      collList persons = collList_reserve();
      collList_add( persons, muriel );
      collList_add( persons, aubin );
      collList_add( persons, eve );
      collList_foreach( persons, printPerson, "List" );
      bool removed = collList_remove( persons, aubin );
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
      collList_release( &persons );
   }
   printf( "+----+----+----------------------------------------\n" );
   {
      collSet persons = collSet_reserve((collComparator)personCompare );
      collSet_add( persons, muriel );
      collSet_add( persons, aubin );
      collSet_add( persons, eve );
      collSet_foreach( persons, printPerson, "Set " );
      bool add = collSet_add( persons, aubin );
      if( add == false ) {
         printf( "|Set |OK  |add: doublon refused\n" );
      }
      else {
         printf( "|Set |FAIL|add: doublon accepted\n" );
      }
      bool removed = collSet_remove( persons, aubin );
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
      collSet_release( &persons );
   }
   printf( "+----+----+----------------------------------------\n" );
   {
      collMap persons = collMap_reserve((collComparator)personMapCompare );
      collMap_put( persons, muriel->forname, muriel );
      collMap_put( persons, aubin ->forname, aubin );
      collMap_put( persons, eve   ->forname, eve );
      collMap_foreach( persons, printPersonEntry, "Map " );
      Person * person = collMap_get( persons, "Eve" );
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
      person = collMap_put( persons, "Aubin", aubin );
      if( person == NULL ) {
         printf( "|Map |FAIL|put: doublon accepted!\n" );
      }
      else if( person == aubin ) {
         printf( "|Map |OK  |put: doublon refused\n" );
      }
      else {
         printf( "|Map |FAIL|put: { %s, %s, %d }\n",
            person->forname, person->name, person->age );
      }
      Person * removed = collMap_remove( persons, "Aubin" );
      if( removed == aubin ) {
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
      collMap_foreach( persons, printPersonEntry, "Map " );
      collMap_release( &persons );
   }
   printf( "+----+----+----------------------------------------\n" );
   free( aubin );
   free( muriel );
   free( eve );
   return 0;
}
#endif
