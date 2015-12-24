#pragma once

#include <dcrud/Arguments.hpp>
#include <dcrud/IOperation.hpp>
#include "IProtocol.hpp"

namespace dcrud {

   struct Operation {

      IOperation *      _operation;
      std::string       _intrfcName;
      std::string       _opName;
      const Arguments & _arguments;
      int               _callId;

      Operation(
         IOperation *        operation,
         const std::string & intrfcName,
         const std::string & opName,
         const Arguments &   arguments,
         int                 callId  )
       :
         _operation ( operation  ),
         _intrfcName( intrfcName ),
         _opName    ( opName     ),
         _arguments ( arguments  ),
         _callId    ( callId     )
      {}

      Operation( const std::string & opName, const Arguments & arguments ) :
         _operation ( 0                    ),
         _intrfcName( ICRUD_INTERFACE_NAME ),
         _opName    ( opName               ),
         _arguments ( arguments            ),
         _callId    ( 0                    )
      {}

   private:
      Operation( const Operation & );
      Operation & operator = ( const Operation & );
   };
}
