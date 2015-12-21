#pragma once

#include <dcrud/Arguments.hpp>
#include <dcrud/IRequired.hpp>

namespace dcrud {

   class ParticipantImpl;

   class RequiredImpl : public IRequired {
   public:

      RequiredImpl( const std::string & name, ParticipantImpl & participant );

      void call( const std::string & opName );

      void call( const std::string & opName, const Arguments & arguments );

      int  call( const std::string & opName, const Arguments & arguments, ICallback & callback );

   private:

      std::string       _name;
      ParticipantImpl & _participant;

   private:
      RequiredImpl( const RequiredImpl & );
      RequiredImpl & operator = ( const RequiredImpl & );
   };
}
