#pragma once

namespace dcrud {

   class IOperation;

   class IProvided {
   public:

      virtual ~ IProvided() {}

      virtual IProvided & addOperation(
         const char * operationName,
         IOperation & executor      ) = 0;
   };
}
