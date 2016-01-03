#pragma once

#include <string>

namespace dcrud {

   class IProvided;
   class IRequired;
   class ICRUD;

   class IDispatcher {
   public:

      virtual ~ IDispatcher() {}

      virtual IProvided & provide       ( const std::string & name ) = 0;
      virtual IRequired & require       ( const std::string & name ) = 0;
      virtual ICRUD &     requireCRUD   ( const ClassID & classId ) = 0;
      virtual void        handleRequests( void ) = 0;
   };
}
