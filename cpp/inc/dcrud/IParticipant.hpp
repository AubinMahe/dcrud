#pragma once

#include <util/types.h>
#include <functional>

namespace dcrud {

   class ClassID;
   class ICache;
   class IDispatcher;
   class Shareable;

   typedef Shareable * (* factory_t )( void );

   class IParticipant {
   public:

      virtual ~ IParticipant() {}

      virtual void registerClass( const ClassID & id, factory_t factory ) = 0;

      virtual ICache & createCache() = 0;

      virtual ICache & getCache( byte ID ) = 0;

      virtual IDispatcher & getDispatcher() = 0;
   };
}
