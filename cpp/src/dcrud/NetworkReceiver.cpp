#include "NetworkReceiver.hpp"
#include "Dispatcher.hpp"
#include "Cache.hpp"
#include "IProtocol.hpp"

#include <dcrud/ClassID.hpp>
#include <dcrud/GUID.hpp>
#include <dcrud/IRequired.hpp>

#include <io/ByteBuffer.hpp>

#include <os/System.h>

#include <util/CheckSysCall.h>
#include <util/Dump.h>
#include <util/Performance.h>

#include <stdexcept>

#if defined( WIN32 ) || defined( _WIN32 )
#  include <windows.h>
#  pragma warning(disable : 4996)
#else
#  include <pthread.h>
#  include <unistd.h>
   typedef void * ( * pthread_routine_t )( void * );
#endif
#include <stdio.h>

using namespace dcrud;

static void * run( void * This ) {
   static_cast<NetworkReceiver *>( This )->run();
   return 0;
}

NetworkReceiver::NetworkReceiver(
   ParticipantImpl &             participant,
   const io::InetSocketAddress & addr,
   const std::string &           intrfc,
   bool                          dumpReceivedBuffer /* = false */ )
 :
   _participant       ( participant                             ),
   _dispatcher        ((Dispatcher &)participant.getDispatcher()),
   _inBuf             ( 64*1024                                 ),
   _in                ( socket( AF_INET, SOCK_DGRAM, 0 )        ),
   _dumpReceivedBuffer( dumpReceivedBuffer                      )
{
   if( ! utilCheckSysCall( _in != INVALID_SOCKET, __FILE__, __LINE__, "socket" )) {
      throw std::runtime_error( "socket" );
   }
   int trueValue = 1;
   if( ! utilCheckSysCall( 0 ==
      setsockopt( _in, SOL_SOCKET, SO_REUSEADDR, (char*)&trueValue, sizeof( trueValue )),
      __FILE__, __LINE__, "setsockopt(SO_REUSEADDR)" ))
   {
      throw std::runtime_error( "setsockopt(SO_REUSEADDR)" );
   }
   struct sockaddr_in local_sin;
   memset( &local_sin, 0, sizeof( local_sin ));
   local_sin.sin_family      = AF_INET;
   local_sin.sin_port        = htons( addr._port );
   local_sin.sin_addr.s_addr = htonl( INADDR_ANY );
   if( ! utilCheckSysCall( 0 ==
      bind( _in, (struct sockaddr *)&local_sin, sizeof( local_sin )),
      __FILE__, __LINE__, "bind(%s,%d)", intrfc.c_str(), addr._port ))
   {
      throw std::runtime_error( "bind" );
   }
   struct ip_mreq mreq;
   memset( &mreq, 0, sizeof( mreq ));
   mreq.imr_multiaddr.s_addr = inet_addr( addr._inetAddress.c_str());
   mreq.imr_interface.s_addr = inet_addr( intrfc.c_str());
   if( ! utilCheckSysCall( 0 ==
      setsockopt( _in, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof( mreq )),
      __FILE__, __LINE__, "setsockopt(IP_ADD_MEMBERSHIP,%s)", addr._inetAddress.c_str()))
   {
      throw std::runtime_error( "setsockopt(IP_ADD_MEMBERSHIP)");
   }
   printf( "receiving from %s, bound to %s:%d\n",
      addr._inetAddress.c_str(), intrfc.c_str(), addr._port );
#ifdef WIN32
   DWORD tid;
   _thread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)&::run, this, 0, &tid );
   if( ! utilCheckSysCall( _thread != NULL, __FILE__, __LINE__, "CreateThread" ))
#else
   if( ! utilCheckSysCall( 0 ==
      pthread_create( &_thread, NULL, (pthread_routine_t)&::run, this ),
      __FILE__, __LINE__, "pthread_create" ))
#endif
   {
      throw std::runtime_error( "pthread_create" );
   }
}

NetworkReceiver:: ~ NetworkReceiver() {
#ifdef WIN32
   WSACancelBlockingCall();
   WaitForSingleObject( _thread, INFINITE );
   closesocket( _in );
#else
   pthread_cancel( _thread ); /* break io::ByteBuffer::receive and cause the thread to exit */
   void * retVal = 0;
   pthread_join( _thread, &retVal );
   close( _in );
#endif
}

void NetworkReceiver::dataUpdate() {
   unsigned int payloadSize = GUID::SIZE + ClassID::SIZE + _inBuf.getInt();
   unsigned int pos         = _inBuf.position();
   _participant.dataUpdate( _inBuf, payloadSize );
   _inBuf.position( pos + payloadSize );
}

void NetworkReceiver::dataDelete() {
   _participant.dataDelete( GUID::unserialize( _inBuf ));
}

void NetworkReceiver::operation() {
   Arguments   args;
   byte        queueNdx   = IRequired::DEFAULT_QUEUE;
   byte        callMode   = IRequired::ASYNCHRONOUS_DEFERRED;
   std::string intrfcName = _inBuf.getString();
   std::string opName     = _inBuf.getString();
   int         callId     = _inBuf.getInt();
   byte        count      = _inBuf.getByte();
   for( unsigned i = 0; i < count; ++i ) {
      std::string   name    = _inBuf.getString();
      ClassID       classID = ClassID::unserialize( _inBuf );
      ClassID::Type type    = classID.getType();
      switch( type ) {
      case ClassID::TYPE_NULL       : args.putNull( name ); break;
      case ClassID::TYPE_BYTE       : args.put( name, _inBuf.getByte   ()); break;
      case ClassID::TYPE_BOOLEAN    : args.put( name, _inBuf.getBoolean()); break;
      case ClassID::TYPE_SHORT      : args.put( name, _inBuf.getShort  ()); break;
      case ClassID::TYPE_INTEGER    : args.put( name, _inBuf.getInt    ()); break;
      case ClassID::TYPE_LONG       : args.put( name, _inBuf.getLong   ()); break;
      case ClassID::TYPE_FLOAT      : args.put( name, _inBuf.getFloat  ()); break;
      case ClassID::TYPE_DOUBLE     : args.put( name, _inBuf.getDouble ()); break;
      case ClassID::TYPE_STRING     : args.put( name, _inBuf.getString ()); break;
      case ClassID::TYPE_CLASS_ID   : args.put( name, ClassID::unserialize( _inBuf )); break;
      case ClassID::TYPE_GUID       : args.put( name, GUID   ::unserialize( _inBuf )); break;
      case ClassID::TYPE_CALL_MODE  : args.setMode ( _inBuf.getByte()); break;
      case ClassID::TYPE_QUEUE_INDEX: args.setQueue( _inBuf.getByte()); break;
      case ClassID::TYPE_SHAREABLE  : args.put( name, _participant.newInstance( classID, _inBuf )); break;
      default:
         fprintf( stderr, "%s:%d: Unexpected class ID: %d\n", __FILE__, __LINE__, (int)type );
      break;
      }
   }
   if( intrfcName == ICRUD_INTERFACE_NAME ) {
      _dispatcher.executeCrud( opName, args );
   }
   else if( callId >= 0 ) {
      _dispatcher.execute( intrfcName, opName, args, callId, queueNdx, callMode );
   }
   else if( callId < 0 ) {
      _participant.callback( intrfcName, opName, Arguments( args ), -callId );
   }
}

void NetworkReceiver::run() {
   uint64_t atStart = 0;
   while( true ) {
      if( atStart > 0 ) {
         utilPerformance_record( "network", osSystem_nanotime() - atStart );
      }
      if( _inBuf.receive( _in )) {
         atStart = osSystem_nanotime();
         _inBuf.flip();
         if( _dumpReceivedBuffer ) {
            _inBuf.dump( stderr );
         }
         char signa[ParticipantImpl::SIGNATURE_SIZE];
         _inBuf.get((byte *)signa, 0, sizeof( signa ));
         if( 0 == strncmp(
               signa, (const char *)ParticipantImpl::SIGNATURE, ParticipantImpl::SIGNATURE_SIZE ))
         {
            byte frameType = _inBuf.getByte();
            switch( frameType ) {
            case ParticipantImpl::DATA_CREATE_OR_UPDATE: dataUpdate(); break;
            case ParticipantImpl::DATA_DELETE          : dataDelete(); break;
            case ParticipantImpl::OPERATION            : operation();  break;
            default:
               fprintf( stderr, "%d isn't a valid FrameType\n", frameType );
               break;
            }
            if( _inBuf.remaining()) {
               fprintf( stderr, "%d received bytes ignored\n", _inBuf.remaining());
            }
         }
         else {
            fprintf( stderr, "Garbage received, %d bytes discarded!\n", _inBuf.limit());
         }
      }
      else {
         break;
      }
   }
}
