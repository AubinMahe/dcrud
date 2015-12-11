#pragma once

#include <util/types.h>

#include <io/ByteBuffer.hpp>

namespace dcrud {

   class GUID {

      short _publisher;
      byte  _cache;
      int   _instance;

   public:

      GUID() :
         _publisher( 0 ),
         _cache    ( 0 ),
         _instance ( 0 )
      {}

      bool isShared() const {
         return _instance != 0;
      }

      void set( const GUID & id );

      void unserialize( io::ByteBuffer & buffer );

      void serialize( io::ByteBuffer & buffer ) const;

      int compareTo( const GUID & right ) const;
   };
}
