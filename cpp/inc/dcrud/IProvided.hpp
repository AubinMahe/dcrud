#pragma once

namespace dcrud {

   struct IOperation;

   struct IProvided {

      virtual ~ IProvided() {}

      virtual IProvided & addOperation(
         const char * operationName,
         IOperation & executor      ) = 0;
   };
}
