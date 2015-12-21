#pragma once

#include <dcrud/GUID.hpp>
#include <dcrud/ClassID.hpp>
#include <io/ByteBuffer.hpp>
#include <util/types.h>

namespace dcrud {

   class Cache;
   class ParticipantImpl;

   class Shareable {
   public:

      Shareable( const ClassID & classId ) :
         _class( classId )
      {}

      virtual ~ Shareable() {}

      virtual const GUID & getGUID( void ) const { return _id; }

      virtual const ClassID & getClassID( void ) const { return _class; }

      virtual void serialize( io::ByteBuffer & target ) const = 0;

      virtual void serializeClass( io::ByteBuffer & target ) const {
         _class.serialize( target );
      }

      virtual void unserialize( io::ByteBuffer & source ) = 0;

   private:

      GUID          _id;
      const ClassID _class;

      friend class Cache;
      friend class ParticipantImpl;
   };
}
