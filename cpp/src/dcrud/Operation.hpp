#pragma once

#include <dcrud/Arguments.hpp>
#include <dcrud/IOperation.hpp>

namespace dcrud {

   struct Operation {

      IOperation &      _operation;
      const Arguments & _arguments;
      args_t &          _results;

      Operation( IOperation & operation, const Arguments & arguments, args_t & results ) :
         _operation( operation ),
         _arguments( arguments ),
         _results  ( results   )
      {}

      void run() {
         _operation.execute( _arguments, _results );
      }
   };
}
