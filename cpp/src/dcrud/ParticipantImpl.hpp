#pragma once

#include <dcrud/IParticipant.hpp>
#include <dcrud/Arguments.hpp>

namespace dcrud {

   struct ICallback;
   struct ParticipantImpl : public IParticipant {

      ParticipantImpl(
         unsigned short id,
         const char *   address,
         unsigned short port,
         const char *   intrfc );

      virtual void registerClass( const ClassID & id, factory_t factory );

      virtual ICache & createCache();

      virtual ICache & getCache( byte ID );

      virtual IDispatcher & getDispatcher();

      virtual int call(
         const std::string & intrfcName,
         const std::string & opName,
         const Arguments &   in,
         int                 callId );

      virtual int call(
         const std::string & intrfcName,
         const std::string & opName,
         const Arguments &   in,
         ICallback *         callback );
   };
}
