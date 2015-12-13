#pragma once

#include <util/types.h>

#include <io/ByteBuffer.hpp>

namespace dcrud {

   struct GUID {

      static const unsigned int SIZE = 2 + 1 + 4;

      short _publisher;
      byte  _cache;
      int   _instance;

      static GUID unserialize( io::ByteBuffer & buffer );

      GUID() :
         _publisher( 0 ),
         _cache    ( 0 ),
         _instance ( 0 )
      {}

      GUID( const GUID & right ) :
         _publisher( right._publisher ),
         _cache    ( right._cache     ),
         _instance ( right._instance  )
      {}

      GUID & operator = ( const GUID & right ) {
         _publisher = right._publisher;
         _cache     = right._cache;
         _instance  = right._instance;
         return *this;
      }

      bool operator < ( const GUID & right ) const {
         return compareTo( right ) < 0;
      }

      bool isShared() const {
         return _instance != 0;
      }

      int compareTo( const GUID & right ) const;

      void set( const GUID & id );

      void serialize( io::ByteBuffer & buffer ) const;

      std::string toString( void ) const;
   };
}
