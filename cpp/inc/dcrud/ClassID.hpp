#pragma once

#include <iostream>
#include <util/types.h>

namespace io {
   struct ByteBuffer;
}

namespace dcrud {

   struct ClassID {

      enum Predefined {
         NullType,
         ByteType,
         BooleanType,
         ShortType,
         IntegerType,
         LongType,
         FloatType,
         DoubleType,
         StringType,
         ClassIDType,
         GUIDType,

         LAST_TYPE
      };

      static const unsigned int SIZE = 4;

      const byte _p1;
      const byte _p2;
      const byte _p3;
      const byte _class;

      ClassID( byte p1, byte p2, byte p3, byte clazz ) :
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

      static void serializePredefined( Predefined predefined, io::ByteBuffer & buffer );

      static ClassID unserialize( io::ByteBuffer & buffer );

      bool operator == ( const ClassID & right ) const {
         return compareTo( right ) == 0;
      }

      bool operator < ( const ClassID & right ) const {
         return compareTo( right ) < 0;
      }

      bool isPredefined() const {
         return ( _p1 == 0 ) && ( _p2 == 0 ) && ( _p3 == 0 );
      }

      int compareTo( const ClassID & right ) const;

      Predefined getPredefined() const;

      void serialize( io::ByteBuffer & buffer ) const;

      std::string toString( void ) const;

   private:
      ClassID & operator = ( const ClassID & right );
   };
}
