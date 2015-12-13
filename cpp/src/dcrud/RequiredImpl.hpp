#pragma once

#include <dcrud/Arguments.hpp>
#include <dcrud/IRequired.hpp>

namespace dcrud {

   struct ParticipantImpl;

   struct RequiredImpl : public IRequired {

      std::string       _name;
      ParticipantImpl & _participant;

      RequiredImpl( const char * name, ParticipantImpl & participant ) :
         _name       ( name        ),
         _participant( participant )
      {}

      int call( const char * opName );

      int call( const char * opName, Arguments & arguments );

      int call( const char * opName, Arguments & arguments, ICallback & callback );
   };
}
