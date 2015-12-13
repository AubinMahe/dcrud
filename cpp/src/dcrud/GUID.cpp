#include <dcrud/GUID.hpp>

using namespace dcrud;

void GUID::set( const GUID & id ) {
   _publisher = id._publisher;
   _cache     = id._cache;
   _instance  = id._instance;
}

GUID GUID::unserialize( io::ByteBuffer & buffer ) {
   GUID guid;
   guid._publisher = buffer.getShort();
   guid._cache     = buffer.getByte();
   guid._instance  = buffer.getInt();
   return guid;
}

void GUID::serialize( io::ByteBuffer & buffer ) const {
   buffer.putShort( _publisher );
   buffer.putByte ( _cache );
   buffer.putInt  ( _instance );
}

int GUID::compareTo( const GUID & right ) const {
   int diff = 0;
   if( diff == 0 ) {
      diff = _publisher - right._publisher;
   }
   if( diff == 0 ) {
      diff = _cache     - right._cache;
   }
   if( diff == 0 ) {
      diff = _instance  - right._instance;
   }
   return diff;
}

std::string GUID::toString( void ) const {
   char buffer[100];
   sprintf( buffer, "%04X-%02X-%08X", _publisher, _cache, _instance );
   return buffer;
}
