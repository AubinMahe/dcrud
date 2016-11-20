#pragma once

#include <io/ByteBuffer.hpp>
#include <io/InetSocketAddress.hpp>

namespace dcrud {

   class ParticipantImpl;
   class Dispatcher;

   class NetworkReceiver {
   public:

      NetworkReceiver(
         ParticipantImpl &             participant,
         const io::InetSocketAddress & addr,
         const std::string &           intrfc );

      ~ NetworkReceiver();

      void dataDelete( void );
      void dataUpdate( void );
      void operation ( void );
      void run       ( void ) __attribute__ ((noreturn));

   private:

      ParticipantImpl & _participant;
      Dispatcher      & _dispatcher;
      io::ByteBuffer    _inBuf;
      SOCKET            _in;
#ifdef WIN32
      HANDLE            _thread;
#else
      pthread_t         _thread;
#endif
   };
}
