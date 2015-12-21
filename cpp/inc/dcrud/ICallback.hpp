#pragma once

#include "Arguments.hpp"

namespace dcrud {

   class ICallback {
   public:

      virtual ~ ICallback(){}

      virtual void callback(
         const std::string & intrfc,
         const std::string & operation,
         const Arguments &   results   ) = 0;
   };
}
