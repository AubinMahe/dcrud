#pragma once

#include "Arguments.hpp"
#include <util/types.h>

namespace dcrud {

   struct ICallback;

   struct IRequired {

      static const byte VERY_URGENT_QUEUE      =   0;
      static const byte URGENT_QUEUE           =  50;
      static const byte DEFAULT_QUEUE          = 100;
      static const byte NON_URGENT_QUEUE       = 255;

      static const byte SYNCHRONOUS            =   0;
      static const byte ASYNCHRONOUS_DEFERRED  =   1;
      static const byte ASYNCHRONOUS_IMMEDIATE =   2;

      virtual ~ IRequired() {}

      virtual int call( const char * opName ) = 0;

      virtual int call( const char * opName, Arguments & arguments ) = 0;

      virtual int call( const char * opName, Arguments & arguments, ICallback & callback ) = 0;
   };
}
