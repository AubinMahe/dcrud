#include <io/ByteBuffer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ioTests( int argc, char * argv[] ) {
   ioByteBuffer buffer = ioByteBuffer_allocate( sizeof( double ));
   double       ref    = 3.141592653;
   ioError      status = ioByteBuffer_putDouble( buffer, ref );
   if( status != ioError_NO_ERROR ) {
      fprintf( stderr, "%s\n", ioErrorMessages[status] );
   }
   else {
      double value = 0.0;
      ioByteBuffer_flip( buffer );
      status = ioByteBuffer_getDouble( buffer, &value );
      if( status != ioError_NO_ERROR ) {
         fprintf( stderr, "%s\n", ioErrorMessages[status] );
      }
      else {
         printf( "read value: %f, expected: %f\n", value, ref );
      }
   }
   return 0;
   (void)argc;
   (void)argv;
}
