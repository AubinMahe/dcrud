#pragma once

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

      virtual void create( Shareable & item ) = 0;

      virtual Shareable & read( const GUID & id ) const = 0;

      virtual void update( Shareable & item ) = 0;

      virtual void deleTe( Shareable & item ) = 0;

      virtual std::set<Shareable *> & values( void ) const = 0;

      virtual std::set<Shareable *> & select( shareablePredicate_t query ) = 0;

      virtual void publish( void ) = 0;

      virtual void subscribe( const ClassID & id ) = 0;

      virtual void refresh( void ) = 0;
   };
}
