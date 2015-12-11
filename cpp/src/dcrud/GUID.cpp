#include <dcrud/GUID.hpp>

using namespace dcrud;

void GUID::set( const GUID & id ) {
   _publisher = id._publisher;
   _cache     = id._cache;
   _instance  = id._instance;
}

void GUID::unserialize( io::ByteBuffer & buffer ) {
   _publisher = buffer.getShort();
   _cache     = buffer.getByte();
   _instance  = buffer.getInt();
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
