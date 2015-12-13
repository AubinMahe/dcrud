#pragma once

#include "Status.hpp"

#include <set>
#include <algorithm>

namespace dcrud {

   struct ClassID;
   struct GUID;
   struct Shareable;

   typedef bool ( * shareablePredicate_t)( const Shareable & shareable );

   struct ICache {

      virtual ~ ICache() {}

      virtual void setOwnership( bool enabled ) = 0;

      virtual bool owns( const GUID & id ) const = 0;

      virtual Status      create( Shareable & item )       = 0;
      virtual Shareable * read  ( const GUID & id  ) const = 0;
      virtual Status      update( Shareable & item )       = 0;
      virtual Status      deleTe( Shareable & item )       = 0;

      virtual void values( std::set<Shareable *> & snapshot ) const = 0;
      virtual bool select( shareablePredicate_t query, std::set<Shareable *> & snapshot ) const = 0;

      virtual void publish( void ) = 0;

      virtual void subscribe( const ClassID & id ) = 0;

      virtual void refresh( void ) = 0;

      template<class T>
      T * get( const GUID & id ) const {
         return dynamic_cast<T *>( read( id ));
      }
   };
}
