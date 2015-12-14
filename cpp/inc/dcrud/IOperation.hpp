#pragma once

#include "Arguments.hpp"

namespace dcrud {

   struct IParticipant;
   struct IOperation {

      virtual ~ IOperation() {}

      virtual void execute( IParticipant & participant, const Arguments & in, args_t & out ) = 0;
   };
}
