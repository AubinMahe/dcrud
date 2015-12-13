#pragma once

#include <dcrud/IOperation.hpp>
#include <dcrud/IProvided.hpp>

namespace dcrud {

   typedef std::map<std::string, IOperation *> opsInOut_t;
   typedef opsInOut_t::iterator                opsInOutIter_t;

   struct ProvidedImpl : public IProvided {

      opsInOut_t _opsInOut;

      IProvided & addOperation( const char * opName, IOperation & executor ) {
         _opsInOut[opName] = &executor;
         return *this;
      }
   };
}
