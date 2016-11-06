#include <io/ByteBuffer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_003( void ) {
   ioByteBuffer buffer = ioByteBuffer_new( sizeof( double ));
   double       ref    = 3.141592653;
   ioStatus      status = ioByteBuffer_putDouble( buffer, ref );
   if( status != IO_STATUS_NO_ERROR ) {
      fprintf( stderr, "%s\n", ioStatusMessages[status] );
   }
   else {
      double value = 0.0;
      ioByteBuffer_flip( buffer );
      status = ioByteBuffer_getDouble( buffer, &value );
      if( status != IO_STATUS_NO_ERROR ) {
         fprintf( stderr, "%s\n", ioStatusMessages[status] );
      }
      else {
         printf( "read value: %f, expected: %f\n", value, ref );
      }
   }
}
