#pragma once

#include "Arguments.hpp"

namespace dcrud {

   struct ICallback {

      virtual ~ ICallback(){}

      virtual void callback(
         const std::string & intrfc,
         const std::string & operation,
         const Arguments &   results   ) = 0;
   };
}
