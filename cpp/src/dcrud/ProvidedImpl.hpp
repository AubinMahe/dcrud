#pragma once

#include <dcrud/IOperation.hpp>
#include <dcrud/IProvided.hpp>

namespace dcrud {

   typedef std::map<std::string, IOperation *> opsInOut_t;
   typedef opsInOut_t::iterator                opsInOutIter_t;

   class ProvidedImpl : public IProvided {
   public:

      ProvidedImpl( void ) {}

      virtual IProvided & addOperation( const std::string & opName, IOperation & executor ) {
         _opsInOut[opName] = &executor;
         return *this;
      }

      IOperation * getOperation( const std::string & opName ) {
         opsInOutIter_t it = _opsInOut.find( opName );
         if( it == _opsInOut.end()) {
            return 0;
         }
         return it->second;
      }

   private:

      opsInOut_t _opsInOut;

   private:
      ProvidedImpl( const ProvidedImpl & );
      ProvidedImpl & operator = ( const ProvidedImpl & );
   };
}
