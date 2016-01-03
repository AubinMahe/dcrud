#pragma once

#include <string>

namespace dcrud {

   class IOperation;

   class IProvided {
   public:

      virtual ~ IProvided() {}

      virtual IProvided & addOperation(
         const std::string & operationName,
         IOperation &        executor      ) = 0;
   };
}
