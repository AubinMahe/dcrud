#include <dcrud/ClassID.hpp>
#include <io/ByteBuffer.hpp>
#include <iomanip>

using namespace dcrud;

std::ostream & ClassID::operator >> ( std::ostream & stream ) const {
   return
      stream
         << std::hex << std::setw(2) << std::setfill('0')
         << _p1 << '-' << _p2 << '-' << _p3 << '-' << _p4;
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
      diff = _p4 -right._p4;
   }
   return diff;
}

void ClassID::serialize( io::ByteBuffer & buffer ) const {
   buffer.putByte( _p1 );
   buffer.putByte( _p2 );
   buffer.putByte( _p3 );
   buffer.putByte( _p4 );
}

ClassID ClassID::unserialize( io::ByteBuffer & buffer ) {
   byte p1 = buffer.getByte();
   byte p2 = buffer.getByte();
   byte p3 = buffer.getByte();
   byte p4 = buffer.getByte();
   return ClassID( p1, p2, p3, p4 );
}
