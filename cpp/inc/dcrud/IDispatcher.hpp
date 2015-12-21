#pragma once

namespace dcrud {

   class IProvided;
   class IRequired;

   class IDispatcher {
   public:

      virtual ~ IDispatcher() {}

      virtual IProvided & provide( const char * name ) = 0;

      virtual IRequired & require( const char * name ) = 0;

      virtual void handleRequests( void ) = 0;
   };
}
