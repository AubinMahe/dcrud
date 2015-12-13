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
   unsigned short id,
   const char *   address,
   unsigned short port,
   const char *   intrfc )
 :
   _publisherId( id ),
   _header     ( HEADER_SIZE  ),
   _payload    ( PAYLOAD_SIZE ),
   _message    ( 64*1024 ), /* UDP MAX packet size */
   _itemCount  ( 0 ),
   _callId     ( 0 )
{
#ifdef _WIN32
   WSADATA wsaData;
   if( ! utilCheckSysCall( 0 ==
      WSAStartup( MAKEWORD( 2, 2 ), &wsaData ),
      __FILE__, __LINE__, "WSAStartup" ))
   {
      *target = NULL;
      throw std::runtime_error( "WSAStartup" );
   }
   atexit( exitHook );
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
   const Arguments &   in,
   int                 callId )
{
   os::Synchronized sync( _outMutex );
   _message.clear();
   _header.clear();
   _header.put( SIGNATURE, 0, SIGNATURE_SIZE );
   _header.putByte( OPERATION );
   _header.putByte((byte)in._args.size());
   _header.flip();
   _message.put( _header );
   _message.putString( intrfcName );
   _message.putString( opName );
   _message.putInt( callId );
   for( argsCstIter_t it = in._args.begin(); it != in._args.end(); ++it ) {
      _payload.clear();
      const std::string & name  = it->first;
      const void *        value = it->second;
      _payload.putString( name );
      if( name == "@queue" ) {
         const byte * queue = (const byte*)value;
         _payload.putByte( *queue );
      }
      else if( name == "@mode" ) {
         const byte * mode = (const byte*)value;
         _payload.putByte( *mode );
      }
      else {
         if( ! value ) {
            ClassID::serializePredefined( ClassID::NullType, _payload );
         }/*
         else if( value instanceof Shareable ) {
            Shareable item = (Shareable)value;
            item._class.serialize( _payload );
            item.serialize( _payload );
         }
         else if( value instanceof Byte ) {
            ClassID.ByteClassID.serialize( _payload );
            _payload.put((Byte)value );
         }
         else if( value instanceof Short ) {
            ClassID.ShortClassID.serialize( _payload );
            _payload.putShort((Short)value );
         }
         else if( value instanceof Integer ) {
            ClassID.IntegerClassID.serialize( _payload );
            _payload.putInt((Integer)value );
         }
         else if( value instanceof Long ) {
            ClassID.LongClassID.serialize( _payload );
            _payload.putLong((Long)value );
         }
         else if( value instanceof Float ) {
            ClassID.FloatClassID.serialize( _payload );
            _payload.putFloat((Float)value );
         }
         else if( value instanceof Double ) {
            ClassID.DoubleClassID.serialize( _payload );
            _payload.putDouble((Double)value );
         }
         else if( value instanceof ClassID ) {
            ClassID.ClassIDClassID.serialize( _payload );
            ((ClassID)value).serialize( _payload );
         }
         else if( value instanceof GUID ) {
            ClassID.GUIDClassID.serialize( _payload );
            ((GUID)value).serialize( _payload );
         }
         else {
            throw std::out_of_range(
               name + " is of type " + value.getClass().getName() +
               " which isn't null, primitive nor derived from " + Shareable.class.getName());
         }*/
         break;
      }
      _payload.flip();
      _message.put( _payload );
   }
   _message.flip();
   _message.send( _out, _target );
}

int ParticipantImpl::call(
   const std::string & intrfcName,
   const std::string & opName,
   const Arguments &   in,
   ICallback *         callback )
{
   call( intrfcName, opName, in, callback ? _callId : 0 );
   if( callback ) {
      _callbacks[_callId] = callback;
      return _callId++;
   }
   return 0;
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

void ParticipantImpl::execute(
   const std::string & intrfcName,
   const std::string & opName,
   const Arguments &   arguments,
   args_t &            results,
   byte                queueNdx,
   byte                callMode )
{
   _dispatcher->execute( intrfcName, opName, arguments, results, queueNdx, callMode );
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
