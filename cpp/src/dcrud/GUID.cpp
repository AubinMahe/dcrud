#include <dcrud/GUID.hpp>

using namespace dcrud;

void GUID::set( const GUID & id ) {
   _publisher = id._publisher;
   _instance  = id._instance;
}

GUID GUID::unserialize( io::ByteBuffer & buffer ) {
   GUID guid;
   guid._publisher = buffer.getInt();
   guid._instance  = buffer.getInt();
   return guid;
}

void GUID::serialize( io::ByteBuffer & buffer ) const {
   buffer.putInt( _publisher );
   buffer.putInt( _instance );
}

int GUID::compareTo( const GUID & right ) const {
   int diff = 0;
   if( diff == 0 ) {
      diff = _publisher - right._publisher;
   }
   if( diff == 0 ) {
      diff = _instance  - right._instance;
   }
   return diff;
}

std::string GUID::toString( void ) const {
   char buffer[100];
   sprintf( buffer, "%08X-%08X", _publisher, _instance );
   return buffer;
}
