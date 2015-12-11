#pragma once

namespace dcrud {

   struct IOperation;

   struct IProvided {

      virtual ~ IProvided() {}

      virtual void addOperation(
         const char * operationName,
         IOperation & executor      ) = 0;
   };
}
