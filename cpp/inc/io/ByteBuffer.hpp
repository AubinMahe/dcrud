#pragma once

#include <util/types.h>
#include <io/ByteBuffer.h>
#include <io/socket.h>

#include <math.h>

#include <iostream>
#include <string>

namespace io {

   enum ByteOrder {

      ioByteOrder_BIG_ENDIAN,
      ioByteOrder_LITTLE_ENDIAN
   };

   class ByteBuffer {
   private:

      ByteBuffer( ioByteBuffer buffer ) {
         _buffer = buffer;
      }

      ioByteBuffer _buffer;

   public:

      ByteBuffer( unsigned int capacity, byte * array = 0 ) {
         if( array ) {
            _buffer = ioByteBuffer_wrap( capacity, array );
         }
         else {
            _buffer = ioByteBuffer_new( capacity );
         }
      }

      ~ ByteBuffer() {
         ioByteBuffer_delete( &_buffer );
      }

      ByteBuffer * copy( unsigned int length ) const {
         return new io::ByteBuffer( ioByteBuffer_copy( _buffer, length ));
      }

      void clear( void ) {
         ioByteBuffer_clear( _buffer );
      }

      void mark( void ) {
         ioByteBuffer_mark( _buffer );
      }

      void reset( void ) {
         ioByteBuffer_reset( _buffer );
      }

      void flip( void ) {
         ioByteBuffer_flip( _buffer );
      }

      unsigned int position( void ) const {
         return ioByteBuffer_getPosition( _buffer );
      }

      void position( unsigned int position ) {
         ioByteBuffer_setPosition( _buffer, position );
      }

      unsigned int limit( void ) const {
         return ioByteBuffer_getLimit( _buffer );
      }

      unsigned int remaining( void ) const {
         return ioByteBuffer_remaining( _buffer );
      }

      void put( const byte * src, unsigned int from, unsigned int to ) {
         ioByteBuffer_put( _buffer, src, from, to );
      }

      void get( byte * target, unsigned int from, unsigned int to ) {
         ioByteBuffer_get( _buffer, target, from, to );
      }

      void putByte( byte value ) {
         ioByteBuffer_putByte( _buffer, value );
      }

      byte getByte( void ) {
         byte value = 0;
         ioByteBuffer_getByte( _buffer, &value );
         return value;
      }

      void putBoolean( bool value ) {
         ioByteBuffer_putByte( _buffer, value ? 1 : 0 );
      };

      bool getBoolean( void ) {
         byte value = 0;
         ioByteBuffer_getByte( _buffer, &value );
         return value != 0;
      }

      void putShort( unsigned short value ) {
         ioByteBuffer_putShort( _buffer, value );
      }

      unsigned short getShort( void ) {
         unsigned short value = 0;
         ioByteBuffer_getShort( _buffer, &value );
         return value;
      }

      void putInt( unsigned int value ) {
         ioByteBuffer_putInt( _buffer, value );
      }

      void putIntAt( unsigned int value, unsigned int index ) {
         ioByteBuffer_putIntAt( _buffer, value, index );
      }

      unsigned int getInt( void ) {
         unsigned int value = 0;
         ioByteBuffer_getInt( _buffer, &value );
         return value;
      }

      void putLong( uint64_t value ) {
         ioByteBuffer_putLong( _buffer, value );
      }

      uint64_t getLong( void ) {
         uint64_t value = 0;
         ioByteBuffer_getLong( _buffer, &value );
         return value;
      }

      void putFloat( float value ) {
         ioByteBuffer_putFloat( _buffer, value );
      }

      float getFloat( void ) {
         float value = 0.0f;
         ioByteBuffer_getFloat( _buffer, &value );
         return value;
      }

      void putDouble( double value ) {
         ioByteBuffer_putDouble( _buffer, value );
      }

      double getDouble( void ) {
         double value = NAN;
         if( ioByteBuffer_getDouble( _buffer, &value )) {
            return value;
         }
         return NAN;
      }

      void putString( const char * value ) {
         ioByteBuffer_putString( _buffer, value );
      }

      void getString( char * target, unsigned int sizeOfTarget ) {
         ioByteBuffer_getString( _buffer, target, sizeOfTarget );
      }

      void putString( const std::string & value ) {
         ioByteBuffer_putString( _buffer, value.c_str());
      }

      std::string getString() {
         static const unsigned sizeOfTarget = 64*1024;
         char target[sizeOfTarget];
         ioByteBuffer_getString( _buffer, target, sizeOfTarget );
         return std::string( target );
      }

      void put( ByteBuffer & source ) {
         ioByteBuffer_putBuffer( _buffer, source._buffer );
      }

      void send( SOCKET sckt, struct sockaddr_in & target ) {
         ioByteBuffer_send( _buffer, sckt, &target );
      }

      bool receive( SOCKET sckt ) {
         return ioByteBuffer_receive( _buffer, sckt ) == IO_STATUS_NO_ERROR;
      }

      byte * array( void ) {
         return ioByteBuffer_array( _buffer );
      }

      void dump( FILE * target ) const {
         ioByteBuffer_dump( _buffer, target );
      }

   private:
      ByteBuffer( const ByteBuffer & right );
      ByteBuffer & operator = ( const ByteBuffer & );
   };
}
