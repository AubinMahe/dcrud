#pragma once

#include "Arguments.hpp"

namespace dcrud {

   struct IOperation {

      virtual ~ IOperation() {}

      virtual void execute( const Arguments & in, args_t & out ) = 0;
   };
}
