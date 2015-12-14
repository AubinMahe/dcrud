#pragma once

#include <dcrud/Arguments.hpp>
#include <dcrud/IOperation.hpp>

namespace dcrud {

   struct Operation {

      IParticipant &    _participant;
      IOperation &      _operation;
      const Arguments & _arguments;
      args_t &          _results;

      Operation(
         IParticipant &    participant,
         IOperation &      operation,
         const Arguments & arguments,
         args_t &          results   )
       :
         _participant( participant ),
         _operation  ( operation   ),
         _arguments  ( arguments   ),
         _results    ( results     )
      {}

      void run() {
         _operation.execute( _participant, _arguments, _results );
      }
   };
}
