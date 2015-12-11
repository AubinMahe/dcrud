#pragma once

#include <map>
#include <string>
#include <stdexcept>

namespace dcrud {

   typedef std::map<std::string, const void *> args_t;

   struct Arguments {

      args_t args;

      template<class T> bool get( const std::string & name, T * & target ) const {
         auto it = args.find( name );
         if( it == args.end()) {
            return false;
         }
         target = static_cast<const T *>( it->second );
         return true;
      }

      template<class T> bool get( const std::string & name, T & target ) const {
         auto it = args.find( name );
         if( it == args.end()) {
            return false;
         }
         const T * ptr = static_cast<const T *>( it->second );
         if( ! ptr ) {
            return false;
         }
         target = *ptr;
         return true;
      }
   };
}
