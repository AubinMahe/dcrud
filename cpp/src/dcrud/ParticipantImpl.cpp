#include "ParticipantImpl.hpp"
#include "Dispatcher.hpp"
#include "Cache.hpp"

#include <dcrud/Shareable.hpp>
#include <dcrud/ICallback.hpp>

#include <util/CheckSysCall.h>

#include <stdio.h>

#include <stdexcept>
#include <algorithm>

using namespace dcrud;

const byte ParticipantImpl::SIGNATURE[] = { 'H','P','M','S'};

#define FRAME_TYPE_SIZE 1U
#define SIZE_SIZE       4U
#define GUID_SIZE       ( 1U + 1U + 1U + 4U )
#define CLASS_ID_SIZE   ( 1U + 1U + 1U + 1U )
#define HEADER_SIZE     ( SIZE_SIZE + FRAME_TYPE_SIZE + SIZE_SIZE + GUID_SIZE + CLASS_ID_SIZE )
#define PAYLOAD_SIZE    ( 64U*1024U - HEADER_SIZE )

ParticipantImpl::ParticipantImpl(
   unsigned short publisherId,
   const char *   address,
   unsigned short port,
   const char *   intrfc )
 :
   _publisherId( publisherId ),
   _header     ( HEADER_SIZE  ),
   _payload    ( PAYLOAD_SIZE ),
   _message    ( 64*1024 ), // UDP MAX packet size
   _itemCount  ( 0 ),
   _callId     ( 0 )
{
#ifdef _WIN32
   WSADATA wsaData;
   if( ! utilCheckSysCall( 0 ==
      WSAStartup( MAKEWORD( 2, 2 ), &wsaData ),
      __FILE__, __LINE__, "WSAStartup" ))
   {
      throw std::runtime_error( "WSAStartup" );
   }
#endif
   _out = socket( AF_INET, SOCK_DGRAM, 0 );
   if( ! utilCheckSysCall( _out != INVALID_SOCKET, __FILE__, __LINE__, "socket" )) {
      throw std::runtime_error( "socket" );
   }
   memset( &_target, 0, sizeof( _target ));
   _target.sin_family      = AF_INET;
   _target.sin_port        = htons( port );
   _target.sin_addr.s_addr = inet_addr( address );
   int trueValue = 1;
   if( ! utilCheckSysCall( 0 ==
      setsockopt( _out, SOL_SOCKET, SO_REUSEADDR, (char*)&trueValue, sizeof( trueValue )),
      __FILE__, __LINE__, "setsockopt(SO_REUSEADDR)" ))
   {
      throw std::runtime_error( "setsockopt(SO_REUSEADDR)" );
   }
   struct in_addr lIntrfc;
   memset( &lIntrfc, 0, sizeof( lIntrfc ));
   lIntrfc.s_addr = inet_addr( intrfc );
   if( ! utilCheckSysCall( 0 ==
      setsockopt( _out, IPPROTO_IP, IP_MULTICAST_IF, (char *)&lIntrfc, sizeof( lIntrfc )),
      __FILE__, __LINE__, "setsockopt(IP_MULTICAST_IF,%s)", intrfc ))
   {
      throw std::runtime_error( "setsockopt(IP_MULTICAST_IF)" );
   }
   printf( "sending to %s:%d via interface %s\n", address, port, intrfc );
   _dispatcher = new Dispatcher( *this );
}

ParticipantImpl:: ~ ParticipantImpl() {
   delete _dispatcher;
   for( unsigned i = 0U; i < CACHE_COUNT; ++i ) {
      if( _caches[i] ) {
         delete _caches[i];
      }
      else {
         break;
      }
   }
}

void ParticipantImpl::registerClass( const ClassID & id, factory_t factory ) {
   os::Synchronized sync( _factoriesMutex );
   _factories[id] = factory;
}

ICache & ParticipantImpl::createCache() {
   os::Synchronized sync( _cachesMutex );
   Cache * cache = new Cache( *this );
   _caches[cache->getId()-1] = cache;
   return *cache;
}

ICache & ParticipantImpl::getCache( byte cacheId ) {
   os::Synchronized sync( _cachesMutex );
   return *_caches[cacheId-1];
}

IDispatcher & ParticipantImpl::getDispatcher() {
   return *_dispatcher;
}

void ParticipantImpl::pushCreateOrUpdateItem( Shareable * item ) {
   os::Synchronized sync( _outMutex );
   _payload.clear();
   item->serialize( _payload );
   _payload.flip();
   int size = _payload.remaining();
   _header.clear();
   _header.put( SIGNATURE, 0, SIGNATURE_SIZE );
   _header.putByte( DATA_CREATE_OR_UPDATE );
   _header.putInt( size );
   item->_id   .serialize( _header );
   item->_class.serialize( _header );
   _header.flip();
   _message.clear();
   _message.put( _header  );
   _message.put( _payload );
   _message.flip();
   _message.send( _out, _target );
}

void ParticipantImpl::publishUpdated( shareables_t & updated ) {
   for( shareablesIter_t it = updated.begin(); it != updated.end(); ++it ) {
      pushCreateOrUpdateItem( *it );
   }
}

void ParticipantImpl::pushDeleteItem( Shareable * item ) {
   os::Synchronized sync( _outMutex );
   _header.put( SIGNATURE, 0, SIGNATURE_SIZE );
   _header.putByte( DATA_DELETE );
   item->_id.serialize( _header );
   _header.flip();
   _header.send( _out, _target );
}

void ParticipantImpl::publishDeleted( shareables_t & deleted ) {
   for( shareablesIter_t it = deleted.begin(); it != deleted.end(); ++it ) {
      pushDeleteItem( *it );
   }
}

void ParticipantImpl::call(
   const std::string & intrfcName,
   const std::string & opName,
   const Arguments *   args,
   int                 callId )
{
   os::Synchronized sync( _outMutex );
   _message.clear();
   _header.clear();
   _header.put( SIGNATURE, 0, SIGNATURE_SIZE );
   _header.putByte( OPERATION );
   _header.putByte( args ? (byte)args->getCount() : 0 );
   _header.flip();
   _message.put( _header );
   _message.putString( intrfcName );
   _message.putString( opName );
   _message.putInt( callId );
   if( args ) {
      args->serialize( _message );
   }
   _message.flip();
   _message.send( _out, _target );
}

int ParticipantImpl::call(
   const std::string & intrfcName,
   const std::string & opName,
   const Arguments *   args,
   ICallback &         callback )
{
   call( intrfcName, opName, args, _callId );
   _callbacks[_callId] = &callback;
   return _callId++;
}

Shareable * ParticipantImpl::newInstance( const ClassID & classId, io::ByteBuffer & frame ) {
   os::Synchronized sync( _factoriesMutex );
   factoriesIter_t it = _factories.find( classId );
   if( it == _factories.end()) {
      return 0;
   }
   factory_t factory = it->second;
   if( ! factory ) {
      return 0;
   }
   Shareable * item = factory();
   item->unserialize( frame );
   return item;
}

void ParticipantImpl::dataDelete( const GUID & id ) {
   os::Synchronized sync( _cachesMutex );
   for( unsigned i = 0U; i < CACHE_COUNT; ++i ) {
      Cache * cache = _caches[i];
      if( cache ) {
         cache->deleteFromNetwork( id );
      }
      else {
         break;
      }
   }
}

void ParticipantImpl::dataUpdate( io::ByteBuffer & frame, int payloadSize ) {
   os::Synchronized sync( _cachesMutex );
   for( unsigned i = 0U; i < CACHE_COUNT; ++i ) {
      Cache * cache = _caches[i];
      if( cache ) {
         cache->updateFromNetwork( frame.copy( GUID_SIZE + CLASS_ID_SIZE + payloadSize ));
      }
      else {
         break;
      }
   }
}

void ParticipantImpl::callback(
   const std::string & intrfcName,
   const std::string & opName,
   const Arguments &   results,
   int                 callId  )
{
   callbacksIter_t it = _callbacks.find( callId );
   if( it == _callbacks.end()) {
      fprintf( stderr, "Unknown Callback received: %s.%s, id: %d\n",
         intrfcName.c_str(), opName.c_str(), -callId );
   }
   else {
      ICallback * callback = it->second;
      if( callback ) {
         callback->callback( intrfcName, opName, results );
      }
      else {
         fprintf( stderr, "Unknown Callback received: %s.%s, id: %d\n",
            intrfcName.c_str(), opName.c_str(), -callId );
      }
   }
}
