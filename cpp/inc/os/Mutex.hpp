#pragma once

namespace os {

   class Mutex {
   private:

      void * _impl;

      Mutex( const Mutex & );
      Mutex & operator = ( const Mutex & );

   public:

      Mutex();
      ~ Mutex();

   public:

      int take();
      int release();
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
