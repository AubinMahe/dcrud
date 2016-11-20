#pragma once

#include <util/types.h>
#include <io/ByteBuffer.h>
#include <io/sockets.h>

#include <math.h>

#include <iostream>
#include <string>

#include <util/check.hpp>

namespace io {

   enum ByteOrder {

      ioByteOrder_BIG_ENDIAN,
      ioByteOrder_LITTLE_ENDIAN
   };

   class ByteBuffer {
   private:

      ByteBuffer( ioByteBuffer buffer ) : _buffer( buffer ) {}

      ioByteBuffer _buffer;

   public:

      ByteBuffer( size_t capacity, byte * array = 0 ) : _buffer( 0 ) {
         if( array ) {
            ioByteBuffer_wrap( &_buffer, capacity, array );
         }
         else {
            ioByteBuffer_new( &_buffer, capacity );
         }
      }

      ~ ByteBuffer() {
         ioByteBuffer_delete( &_buffer );
      }

      ByteBuffer * copy( size_t length ) const {
         ioByteBuffer copy;
         ioByteBuffer_copy( _buffer, &copy, length );
         return new io::ByteBuffer( copy );
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

      size_t position( void ) const {
         size_t pos = -1U;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getPosition( _buffer, &pos ))
         return pos;
      }

      void position( size_t position ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_setPosition( _buffer, position ))
      }

      size_t limit( void ) const {
         size_t limit;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getLimit( _buffer, &limit ))
         return limit;
      }

      size_t remaining( void ) const {
         size_t remaining;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_remaining( _buffer, &remaining ))
         return remaining;
      }

      void put( const byte * src, size_t from, size_t to ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_put( _buffer, src, from, to ))
      }

      void get( byte * target, size_t from, size_t to ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_get( _buffer, target, from, to ))
      }

      void putByte( byte value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putByte( _buffer, value ))
      }

      byte getByte( void ) {
         byte value = 0;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getByte( _buffer, &value ))
         return value;
      }

      void putBoolean( bool value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putByte( _buffer, value ? 1 : 0 ))
      };

      bool getBoolean( void ) {
         byte value = 0;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getByte( _buffer, &value ))
         return value != 0;
      }

      void putShort( short value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putShort( _buffer, value ))
      }

      void putUShort( unsigned short value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putUShort( _buffer, value ))
      }

      short getShort( void ) {
         short value = 0;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getShort( _buffer, &value ))
         return value;
      }

      unsigned short getUshort( void ) {
         unsigned short value = 0;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getUShort( _buffer, &value ))
         return value;
      }

      void putInt( int value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putInt( _buffer, value ))
      }

      void putUInt( unsigned int value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putUInt( _buffer, value ))
      }

      void putUIntAt( unsigned int value, size_t index ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putUIntAt( _buffer, value, index ))
      }

      unsigned int getInt( void ) {
         int value = 0;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getInt( _buffer, &value ))
         return value;
      }

      void putLong( int64_t value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putLong( _buffer, value ))
      }

      void putULong( uint64_t value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putULong( _buffer, value ))
      }

      int64_t getLong( void ) {
         int64_t value = 0;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getLong( _buffer, &value ))
         return value;
      }

      uint64_t getULong( void ) {
         uint64_t value = 0;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getULong( _buffer, &value ))
         return value;
      }

      void putFloat( float value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putFloat( _buffer, value ))
      }

      float getFloat( void ) {
         float value = 0.0f;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getFloat( _buffer, &value ))
         return value;
      }

      void putDouble( double value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putDouble( _buffer, value ))
      }

      double getDouble( void ) {
         double value;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getDouble( _buffer, &value ))
         return value;
      }

      void putString( const char * value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putString( _buffer, value ))
      }

      void getString( char * target, size_t sizeOfTarget ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getString( _buffer, target, sizeOfTarget ))
      }

      void putString( const std::string & value ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putString( _buffer, value.c_str()))
      }

      std::string getString() {
         static const unsigned sizeOfTarget = 64*1024;
         char target[sizeOfTarget];
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_getString( _buffer, target, sizeOfTarget ))
         return std::string( target );
      }

      void put( ByteBuffer & source ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_putBuffer( _buffer, source._buffer ))
      }

      void send( SOCKET sckt ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_send( _buffer, sckt ))
      }

      void sendTo( SOCKET sckt, struct sockaddr_in & target ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_sendTo( _buffer, sckt, &target ))
      }

      void receive( SOCKET sckt ) {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_receive( _buffer, sckt ))
      }

      byte * array( void ) {
         byte * array = 0;
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_array( _buffer, &array ))
         return array;
      }

      void dump( FILE * target ) const {
         CPPCHK(__FILE__,__LINE__, ioByteBuffer_dump( _buffer, target ))
      }

   private:
      ByteBuffer( const ByteBuffer & right );
      ByteBuffer & operator = ( const ByteBuffer & );
   };
}
