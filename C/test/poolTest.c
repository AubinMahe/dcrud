#include <util/Pool.h>
#include <stdio.h>

typedef struct toto_s {

   int    a;
   double b;

} Toto;

#define Toto_POOL_SIZE 100U

UTIL_POOL_DECLARE( Toto )

utilStatus poolTest( void ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
#ifdef STATIC_ALLOCATION
   Toto * t1;
   UTIL_POOL_INIT( Toto );
   status = utilPool_reserve( &TotoPool, &t1 );
   if( UTIL_STATUS_NO_ERROR == status ) {
      Toto * t2;
      status = utilPool_reserve( &TotoPool, &t2 );
      t1->a = 1;
      t1->b = 2.3;
      if( UTIL_STATUS_NO_ERROR == status ) {
         Toto * t3;
         status = utilPool_reserve( &TotoPool, &t3 );
         t2->a = 2;
         t2->b = 3.4;
         if( UTIL_STATUS_NO_ERROR == status ) {
            t3->a = 3;
            t3->b = 4.5;
            status = utilPool_release( &TotoPool, t1 );
            if( UTIL_STATUS_NO_ERROR == status ) {
               status = utilPool_release( &TotoPool, t2 );
               if( UTIL_STATUS_NO_ERROR == status ) {
                  status = utilPool_release( &TotoPool, t3 );
                  if( UTIL_STATUS_NO_ERROR == status ) {
                     printf( "Successful\n" );
                  }
                  else {
                     printf( "Failure, t3 not found\n" );
                  }
               }
               else {
                  printf( "Failure, t2 not found\n" );
               }
            }
            else {
               printf( "Failure, t1 not found\n" );
            }
         }
         else {
            printf( "Failure, can't allocate t3\n" );
         }
      }
      else {
         printf( "Failure, can't allocate t2\n" );
      }
   }
   else {
      printf( "Failure, can't allocate t1\n" );
   }
#endif
   return status;
}
