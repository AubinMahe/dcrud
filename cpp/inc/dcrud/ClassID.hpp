#pragma once

#include <iostream>
#include <util/types.h>

namespace io {
   struct ByteBuffer;
}

namespace dcrud {

   struct ClassID {

      const byte _p1;
      const byte _p2;
      const byte _p3;
      const byte _p4;

      ClassID( byte p1, byte p2, byte p3, byte p4 ) :
         _p1( p1 ),
         _p2( p2 ),
         _p3( p3 ),
         _p4( p4 )
      {}

      static ClassID unserialize( io::ByteBuffer & buffer );

      bool operator == ( const ClassID & right ) const {
         return compareTo( right ) == 0;
      }

      std::ostream & operator >> ( std::ostream & stream ) const;

      int compareTo( const ClassID & right ) const;

      void serialize( io::ByteBuffer & buffer ) const;
   };
}

inline std::ostream & operator << ( std::ostream & stream, const dcrud::ClassID & classID ) {
   return classID >> stream;
}
