#pragma once

#include <stdexcept>

#define CPPCHK(F,L,E) {\
      utilStatus status = E;\
      if( UTIL_STATUS_NO_ERROR != status ) {\
         utilStatus_checkAndLog(status,F,L,#E);\
         throw std::runtime_error( #E );\
      }\
   }
