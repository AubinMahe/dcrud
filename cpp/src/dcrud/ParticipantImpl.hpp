#pragma once

#include <dcrud/IParticipant.hpp>
#include <dcrud/Arguments.hpp>
#include <dcrud/GUID.hpp>

#include <io/ByteBuffer.hpp>

#include <os/Mutex.hpp>

#include <set>

namespace dcrud {

   struct ICallback;
   struct Cache;

   typedef std::set<Shareable *>        shareables_t;
   typedef shareables_t::iterator       shareablesIter_t;

   typedef std::map<ClassID, factory_t> factories_t;
   typedef factories_t::iterator        factoriesIter_t;

   typedef std::map<int, ICallback *>   callbacks_t;
   typedef callbacks_t::iterator        callbacksIter_t;

   struct Dispatcher;

   struct ParticipantImpl : public IParticipant {

      static const byte NO_OP                 = 0;
      static const byte DATA_CREATE_OR_UPDATE = 1;
      static const byte DATA_DELETE           = 2;
      static const byte OPERATION             = 3;

      static const unsigned SIGNATURE_SIZE = 5U;
      static const byte     SIGNATURE[SIGNATURE_SIZE];

      static const unsigned CACHE_COUNT = 256;

      unsigned short     _publisherId;
      io::ByteBuffer     _header;
      io::ByteBuffer     _payload;
      io::ByteBuffer     _message;
      os::Mutex          _outMutex;
      SOCKET             _out;
      os::Mutex          _cachesMutex;
      Cache *            _caches[CACHE_COUNT];
      os::Mutex          _factoriesMutex;
      factories_t        _factories;
      callbacks_t        _callbacks;
      struct sockaddr_in _target;
      Dispatcher *       _dispatcher;
      unsigned int       _itemCount;
      int                _callId;


      ParticipantImpl(
         unsigned short id,
         const char *   address,
         unsigned short port,
         const char *   intrfc );

      short getPublisherId() const {
         return _publisherId;
      }

      virtual void registerClass( const ClassID & id, factory_t factory );

      virtual ICache & createCache();

      virtual ICache & getCache( byte ID );

      virtual IDispatcher & getDispatcher();

      virtual void call(
         const std::string & intrfcName,
         const std::string & opName,
         const Arguments &   in,
         int                 callId );

      virtual int call(
         const std::string & intrfcName,
         const std::string & opName,
         const Arguments &   in,
         ICallback *         callback );

      Shareable * newInstance( const ClassID & classId, io::ByteBuffer & frame );

      void execute(
         const std::string & intrfcName,
         const std::string & opName,
         const Arguments &   arguments,
         args_t &            results,
         byte                queueNdx,
         byte                callMode );

      void callback(
         const std::string & intrfcName,
         const std::string & opName,
         const Arguments &   results,
         int                 callId  );

      void publishUpdated( shareables_t & updated );

      void publishDeleted( shareables_t & updated );

      void dataDelete( const GUID & id );

      void dataUpdate( io::ByteBuffer & frame, int payloadSize );

      void pushCreateOrUpdateItem( Shareable * item );

      void pushDeleteItem( Shareable * item );
   };
}
