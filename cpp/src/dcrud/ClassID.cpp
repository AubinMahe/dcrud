#include <dcrud/ClassID.hpp>

#include <iomanip>
#include <stdexcept>

using namespace dcrud;

void ClassID::serialize( Type type, io::ByteBuffer & buffer ) {
   buffer.putByte( 0 );
   buffer.putByte( 0 );
   buffer.putByte( 0 );
   buffer.putByte( type );
}

int ClassID::compareTo( const ClassID & right ) const {
   int diff = 0;
   if( diff == 0 ) {
      diff = _p1 -right._p1;
   }
   if( diff == 0 ) {
      diff = _p2 -right._p2;
   }
   if( diff == 0 ) {
      diff = _p3 -right._p3;
   }
   if( diff == 0 ) {
      diff = _class -right._class;
   }
   return diff;
}

ClassID ClassID::unserialize( io::ByteBuffer & buffer ) {
   byte p1 = buffer.getByte();
   byte p2 = buffer.getByte();
   byte p3 = buffer.getByte();
   byte p4 = buffer.getByte();
   return ClassID( p1, p2, p3, p4 );
}

void ClassID::serialize( io::ByteBuffer & buffer ) const {
   buffer.putByte( _p1 );
   buffer.putByte( _p2 );
   buffer.putByte( _p3 );
   buffer.putByte( _class );
}

ClassID::Type ClassID::getType() const {
   if( _p1 > 0 || _p2 > 0 || _p3 > 0 ) {
      return TYPE_SHAREABLE;
   }
   if( _class < LAST_TYPE ) {
      return (Type)_class;
   }
   throw std::out_of_range( "Bad kind of ClassID" );
}

std::string ClassID::toString( void ) const {
   char buffer[100];
   sprintf( buffer, "%02X-%02X-%02X-%02X", _p1, _p2, _p3, _class );
   return buffer;
}
