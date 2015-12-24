#pragma once

#include <dcrud/IParticipant.hpp>
#include <dcrud/Arguments.hpp>

#include <dcrud/GUID.hpp>

#include <io/ByteBuffer.hpp>

#include <os/Mutex.hpp>

#include <list>
#include <set>

namespace dcrud {

   class ICallback;
   class Cache;
   class Dispatcher;
   class NetworkReceiver;

   typedef std::set<Shareable *>             shareables_t;
   typedef shareables_t::iterator            shareablesIter_t;

   typedef std::map<ClassID, localFactory_t> localFactories_t;
   typedef localFactories_t::iterator        localFactoriesIter_t;

   typedef std::map<ClassID, ICRUD *>        remoteFactories_t;
   typedef remoteFactories_t::iterator       remoteFactoriesIter_t;

   typedef std::map<int, ICallback *>        callbacks_t;
   typedef callbacks_t::iterator             callbacksIter_t;

   typedef std::list<NetworkReceiver *>      networkReceivers_t;
   typedef networkReceivers_t::iterator      networkReceiversIter_t;

   class ParticipantImpl : public IParticipant {
   public:

      static const byte NO_OP                 = 0;
      static const byte DATA_CREATE_OR_UPDATE = 1;
      static const byte DATA_DELETE           = 2;
      static const byte OPERATION             = 3;

      static const unsigned SIGNATURE_SIZE = 5U;
      static const byte     SIGNATURE[SIGNATURE_SIZE];

   public:

      ParticipantImpl(
         unsigned int        publisherId,
         const std::string & address,
         unsigned short      port,
         const std::string & intrfc );

      ~ ParticipantImpl();

      virtual void listen(
         const std::string & mcastAddr,
         unsigned short      port,
         const std::string & networkInterface );

      virtual void registerLocalFactory( const ClassID & id, localFactory_t factory );

      virtual void registerRemoteFactory( const ClassID & id, ICRUD * factory );

      virtual ICache & getDefaultCache();

      virtual ICache & createCache( byte & id );

      virtual ICache & getCache( byte id );

      virtual IDispatcher & getDispatcher();

      void call(
         const std::string & intrfcName,
         const std::string & opName,
         const Arguments *   args,
         int                 callId );

      int call(
         const std::string & intrfcName,
         const std::string & opName,
         const Arguments *   in,
         ICallback &         callback );

      unsigned int getPublisherId() const {
         return _publisherId;
      }

      Shareable * newInstance( const ClassID & classId, io::ByteBuffer & frame );

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

      bool create( const ClassID & clsId, const Arguments & how );

      bool update( const GUID & id, const Arguments & how );

      bool deleTe( const GUID & id );

   private:

      static const unsigned CACHE_COUNT = 256;

      unsigned int       _publisherId;
      io::ByteBuffer     _header;
      io::ByteBuffer     _payload;
      io::ByteBuffer     _message;
      os::Mutex          _outMutex;
      SOCKET             _out;
      os::Mutex          _cachesMutex;
      Cache *            _caches[CACHE_COUNT];
      os::Mutex          _factoriesMutex;
      localFactories_t   _localFactories;
      remoteFactories_t  _remoteFactories;
      callbacks_t        _callbacks;
      struct sockaddr_in _target;
      Dispatcher *       _dispatcher;
      byte               _cacheCount;
      int                _callId;
      networkReceivers_t _receivers;
   };
}
