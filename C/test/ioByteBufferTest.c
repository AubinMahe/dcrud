#include <io/ByteBuffer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ioByteBufferTest( void ) {
   utilStatus   status = UTIL_STATUS_NO_ERROR;
   ioByteBuffer buffer = NULL;
   double       ref    = 3.141592653;

   status = ioByteBuffer_new( &buffer, sizeof( double ));
   if( status != UTIL_STATUS_NO_ERROR ) {
      fprintf( stderr, "%s\n", utilStatusMessages[status] );
   }
   else {
      status = ioByteBuffer_putDouble( buffer, ref );
      if( status != UTIL_STATUS_NO_ERROR ) {
         fprintf( stderr, "%s\n", utilStatusMessages[status] );
      }
      else {
         double value = 0.0;
         ioByteBuffer_flip( buffer );
         status = ioByteBuffer_getDouble( buffer, &value );
         if( status != UTIL_STATUS_NO_ERROR ) {
            fprintf( stderr, "%s\n", utilStatusMessages[status] );
         }
         else {
            double delta = ref-value;
            if( delta > 0.000001 ) {
               printf( "FAIL: read value: %f, expected: %f, delta: %f\n", value, ref, delta );
            }
            else {
               printf( "OK  : read value: %f, expected: %f, delta: %f\n", value, ref, delta );
            }
         }
      }
      ioByteBuffer_delete( &buffer );
   }
}
