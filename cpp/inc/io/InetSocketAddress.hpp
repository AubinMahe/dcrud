#pragma once

#include <string>

namespace io {

   struct InetSocketAddress {

      std::string    _inetAddress;
      unsigned short _port;

      InetSocketAddress( const std::string & host, unsigned short port ) :
         _inetAddress( host ),
         _port       ( port )
      {}
   };

   inline bool operator < ( const io::InetSocketAddress & left, const io::InetSocketAddress & right ) {
      bool lt = left._inetAddress < right._inetAddress;
      if( lt ) {
         return true;
      }
      bool gt = left._inetAddress > right._inetAddress;
      if( gt ) {
         return false;
      }
      lt = left._port < right._port;
      return lt;
   }
}
