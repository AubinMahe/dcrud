#pragma once

#include <dcrud/Arguments.hpp>
#include <dcrud/IOperation.hpp>

namespace dcrud {

   struct Operation {

      IOperation &      _operation;
      const Arguments & _arguments;
      std::string       _intrfcName;
      std::string       _opName;
      int               _callId;

      Operation(
         IOperation &        operation,
         const Arguments &   arguments,
         const std::string & intrfcName,
         const std::string & opName,
         int                 callId  )
       :
         _operation ( operation  ),
         _arguments ( arguments  ),
         _intrfcName( intrfcName ),
         _opName    ( opName     ),
         _callId    ( callId     )
      {}

   private:
      Operation( const Operation & );
      Operation & operator = ( const Operation & );
   };
}
