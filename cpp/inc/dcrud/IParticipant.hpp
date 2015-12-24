#pragma once

#include <util/types.h>
#include <string>

namespace dcrud {

   class ClassID;
   class ICache;
   class IDispatcher;
   class Shareable;
   class ICRUD;

   typedef Shareable * (* localFactory_t )( void );

   class IParticipant {
   public:

      virtual ~ IParticipant() {}

      virtual void listen(
         const std::string & mcastAddr,
         unsigned short      port,
         const std::string & networkInterface ) = 0;

      virtual void registerLocalFactory ( const ClassID & id, localFactory_t factory ) = 0;

      virtual void registerRemoteFactory( const ClassID & id, ICRUD * factory ) = 0;

      virtual ICache & getDefaultCache() = 0;

      virtual ICache & createCache( byte & cacheIndex ) = 0;

      virtual ICache & getCache( byte ID ) = 0;

      virtual IDispatcher & getDispatcher() = 0;
   };
}
