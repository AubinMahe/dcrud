#pragma once

#include <map>
#include <string>
#include <stdexcept>

namespace dcrud {

   typedef std::map<std::string, const void *> args_t;
   typedef args_t::iterator                    argsIter_t;
   typedef args_t::const_iterator              argsCstIter_t;

   struct Arguments {

      args_t _args;

      Arguments() {}

      Arguments( const args_t & args ) :
         _args( args )
      {}

      template<class T> void put( const std::string & key, const T & value ) {
         _args[key] = new T( value );
      }

      template<class T> bool get( const std::string & name, T * & target ) const {
         argsCstIter_t it = _args.find( name );
         if( it == _args.end()) {
            return false;
         }
         target = static_cast<const T *>( it->second );
         return true;
      }

      template<class T> bool get( const std::string & name, T & target ) const {
         argsCstIter_t it = _args.find( name );
         if( it == _args.end()) {
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
