#pragma once

#include <dcrud/GUID.hpp>
#include <dcrud/ClassID.hpp>
#include <util/types.h>

namespace io {
   struct ByteBuffer;
}

namespace dcrud {

   struct Shareable {

      GUID          _id;
      const ClassID _class;

      Shareable( const ClassID & classId ) :
         _class( classId )
      {}

      virtual ~ Shareable() {}

      virtual const GUID & getGUID( void ) const { return _id; }

      virtual const ClassID & getClassID( void ) const { return _class; }

      virtual void serialize( io::ByteBuffer & target ) const = 0;

      virtual void unserialize( io::ByteBuffer & source ) = 0;
   };
}
