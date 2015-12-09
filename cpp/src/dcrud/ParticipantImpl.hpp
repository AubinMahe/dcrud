#pragma once

#include <dcrud/IParticipant.hpp>

namespace dcrud {

   struct ParticipantImpl : public IParticipant {

      ParticipantImpl(
         unsigned short id,
         const char *   address,
         unsigned short port,
         const char *   intrfc );

      virtual void registerClass( const ClassID & id, Supplier<Shareable> & factory );

      virtual ICache & createCache();

      virtual ICache & getCache( byte ID );

      virtual IDispatcher & getDispatcher();

   };
}
