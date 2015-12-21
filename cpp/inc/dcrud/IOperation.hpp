#pragma once

#include "Arguments.hpp"

namespace dcrud {

   class IParticipant;

   class IOperation {
   public:

      virtual ~ IOperation() {}

      virtual Arguments * execute( IParticipant & participant, const Arguments & args ) = 0;
   };
}
