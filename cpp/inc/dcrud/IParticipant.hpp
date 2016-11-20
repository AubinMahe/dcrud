#pragma once

#include <util/types.h>
#include <io/InetSocketAddress.hpp>

namespace dcrud {

   class ClassID;
   class ICache;
   class IDispatcher;
   class Shareable;
   class ICRUD;
   class IRegistry;

   typedef Shareable * (* localFactory_t )( void );

   class IParticipant {
   public:

      virtual ~ IParticipant() {}

      virtual void listen( const IRegistry & registry, const std::string & networkInterface ) = 0;

      virtual void registerLocalFactory ( const ClassID & id, localFactory_t factory ) = 0;

      virtual void registerRemoteFactory( const ClassID & id, ICRUD * factory ) = 0;

      virtual ICache & getDefaultCache() = 0;

      virtual ICache & createCache( byte & cacheIndex ) = 0;

      virtual ICache & getCache( byte ID ) = 0;

      virtual IDispatcher & getDispatcher() = 0;
   };
}
