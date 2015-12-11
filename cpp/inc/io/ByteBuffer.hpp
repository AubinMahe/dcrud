#pragma once

#include <util/types.h>
#include <io/socket.h>

#include <iostream>
#include <string>

namespace io {

   enum ByteOrder {

      ioByteOrder_BIG_ENDIAN,
      ioByteOrder_LITTLE_ENDIAN
   };

   struct ByteBuffer {

      static ByteBuffer & allocate( unsigned int capacity );

      virtual ~ ByteBuffer() {}

      virtual ByteBuffer & duplicate( void ) const = 0;

      virtual void release( void ) = 0;

      virtual void clear( void ) = 0;

      virtual void mark( void ) = 0;

      virtual void reset( void ) = 0;

      virtual void flip( void ) = 0;

      virtual unsigned int position( void ) const = 0;

      virtual void position( unsigned int position ) = 0;

      virtual unsigned int limit( void ) const = 0;

      virtual unsigned int remaining( void ) const = 0;

      virtual void put( const byte * src, unsigned int from, unsigned int to ) = 0;

      virtual void get( byte * target, unsigned int from, unsigned int to ) = 0;

      virtual void putByte( byte value ) = 0;

      virtual byte getByte( void ) = 0;

      virtual void putShort( unsigned short value ) = 0;

      virtual unsigned short getShort( void ) = 0;

      virtual void putInt( unsigned int value ) = 0;

      virtual void putIntAt( unsigned int value, unsigned int index ) = 0;

      virtual int getInt( void ) = 0;

      virtual void putLong( uint64_t value ) = 0;

      virtual uint64_t getLong( void ) = 0;

      virtual void putFloat( float value ) = 0;

      virtual float getFloat( void ) = 0;

      virtual void putDouble( double value ) = 0;

      virtual double getDouble( void ) = 0;

      virtual void putString( const char * src ) = 0;

      virtual void getString( char * target, unsigned int sizeOfTarget ) = 0;

      virtual void putString( const std::string & src ) = 0;

      virtual std::string getString() = 0;

      virtual void put( ByteBuffer & source ) = 0;

      virtual void send( SOCKET sckt, struct sockaddr_in * target ) = 0;

      virtual void receive( SOCKET sckt ) = 0;

      virtual byte * getBytes( void ) = 0;

      virtual ByteBuffer & copy( unsigned int length ) = 0;

      virtual std::ostream & operator >> ( std::ostream & stream ) const = 0;
   };
}

inline std::ostream & operator << ( std::ostream & stream, const io::ByteBuffer & buffer ) {
   return buffer >> stream;
}
