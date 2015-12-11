#pragma once

namespace dcrud {

   struct IProvided;
   struct IRequired;

   struct IDispatcher {

      virtual ~ IDispatcher() {}

      virtual IProvided & provide( const char * name ) = 0;

      virtual IRequired & require( const char * name ) = 0;

      virtual void handleRequests( void ) = 0;
   };
}
