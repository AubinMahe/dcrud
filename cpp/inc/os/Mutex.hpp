#pragma once

#include <os/Mutex.h>

namespace os {

   class Mutex {
   private:

      osMutex _mutex;

      Mutex( const Mutex & );
      Mutex & operator = ( const Mutex & );

   public:

      Mutex() {
         osMutex_new( &_mutex );
      }

      ~ Mutex() {
         osMutex_delete( &_mutex );
      }

   public:

      int take() {
         return osMutex_take( _mutex );
      }

      int release() {
         return osMutex_release( _mutex );
      }
   };

   struct Synchronized {

      Mutex & _mutex;

      Synchronized( Mutex & mutex ) :
         _mutex( mutex )
      {
         _mutex.take();
      }

      ~ Synchronized() {
         _mutex.release();
      }
   };
}
