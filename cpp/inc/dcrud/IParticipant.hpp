#pragma once

#include <util/types.h>

namespace dcrud {

   struct ClassID;
   struct ICache;
   struct IDispatcher;
   class Shareable;

   template< class T >
   struct Supplier {

   };

   struct IParticipant {

      virtual ~ IParticipant() {}

      virtual void registerClass( const ClassID & id, Supplier<Shareable> & factory ) = 0;

      virtual ICache & createCache() = 0;

      virtual ICache & getCache( byte ID ) = 0;

      virtual IDispatcher & getDispatcher() = 0;
   };
}
