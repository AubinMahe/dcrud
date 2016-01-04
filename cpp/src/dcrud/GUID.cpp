#include <dcrud/GUID.hpp>

void dcrud::GUID::set( const dcrud::GUID & id ) {
   _publisher = id._publisher;
   _instance  = id._instance;
}

dcrud::GUID dcrud::GUID::unserialize( io::ByteBuffer & buffer ) {
   dcrud::GUID guid;
   guid._publisher = buffer.getInt();
   guid._instance  = buffer.getInt();
   return guid;
}

void dcrud::GUID::serialize( io::ByteBuffer & buffer ) const {
   buffer.putInt( _publisher );
   buffer.putInt( _instance );
}

int dcrud::GUID::compareTo( const dcrud::GUID & right ) const {
   int diff = 0;
   if( diff == 0 ) {
      diff = _publisher - right._publisher;
   }
   if( diff == 0 ) {
      diff = _instance  - right._instance;
   }
   return diff;
}

std::string dcrud::GUID::toString( void ) const {
   char buffer[100];
   sprintf( buffer, "%08X-%08X", _publisher, _instance );
   return buffer;
}
