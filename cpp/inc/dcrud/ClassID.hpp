#pragma once

#include <io/ByteBuffer.hpp>

namespace dcrud {

   class ClassID {
   public:

      enum Type {

         TYPE_NULL,
         TYPE_BOOLEAN,
         TYPE_BYTE,
         TYPE_CHAR,
         TYPE_SHORT,
         TYPE_UNSIGNED_SHORT,
         TYPE_INTEGER,
         TYPE_UNSIGNED_INTEGER,
         TYPE_LONG,
         TYPE_UNSIGNED_LONG,
         TYPE_FLOAT,
         TYPE_DOUBLE,
         TYPE_STRING,
         TYPE_CLASS_ID,
         TYPE_GUID,
         TYPE_SHAREABLE,

         LAST_TYPE
      };

      static const unsigned int SIZE = 4;

      ClassID( byte p1 = 0, byte p2 = 0, byte p3 = 0, byte clazz = 0 ) :
         _p1   ( p1    ),
         _p2   ( p2    ),
         _p3   ( p3    ),
         _class( clazz )
      {}

      ClassID( const ClassID & right ) :
         _p1   ( right._p1    ),
         _p2   ( right._p2    ),
         _p3   ( right._p3    ),
         _class( right._class )
      {}

      ClassID & operator = ( const ClassID & right ) {
         _p1    = right._p1;
         _p2    = right._p2;
         _p3    = right._p3;
         _class = right._class;
         return *this;
      }

      static void serialize( Type type, io::ByteBuffer & buffer );

      static ClassID unserialize( io::ByteBuffer & buffer );

      bool operator == ( const ClassID & right ) const {
         return compareTo( right ) == 0;
      }

      bool operator < ( const ClassID & right ) const {
         return compareTo( right ) < 0;
      }

      int compareTo( const ClassID & right ) const;

      Type getType() const;

      void serialize( io::ByteBuffer & buffer ) const;

      std::string toString( void ) const;

   private:

      byte _p1;
      byte _p2;
      byte _p3;
      byte _class;
   };
}
