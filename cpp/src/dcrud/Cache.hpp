#pragma once

#include <dcrud/ICache.hpp>
#include <dcrud/ClassID.hpp>
#include <dcrud/GUID.hpp>
#include "ParticipantImpl.hpp"
#include <io/ByteBuffer.hpp>
#include <os/Mutex.hpp>

#include <stdexcept>

namespace dcrud {

   class Cache : public ICache {
   public:

      Cache( ParticipantImpl & participant );

      virtual ~ Cache();

      bool matches( short publisherId, byte cacheId ) const;

      unsigned getId() const {
         return _cacheId;
      }

      void updateFromNetwork( io::ByteBuffer * item );

      void deleteFromNetwork( const GUID & id );

      virtual void setOwnership( bool enabled ) {
         _ownershipCheck = enabled;
      }

      virtual bool owns( const GUID & id ) const;

      virtual Status      create( Shareable & item );
      virtual Shareable * read  ( const GUID & id ) const;
      virtual Status      update( Shareable & item );
      virtual Status      deleTe( Shareable & item );

      virtual void values( shareables_t & snapshot ) const;

      virtual bool select( shareablePredicate_t query, shareables_t & snapshot ) const;

      virtual void publish( void );

      virtual void subscribe( const ClassID & id );

      virtual void refresh( void );

   private:

      static byte _NextCacheId;

      typedef std::map<GUID, Shareable*>    local_t;
      typedef local_t::iterator             localIter_t;
      typedef local_t::const_iterator       localCstIter_t;

      typedef std::set<io::ByteBuffer *>    byteBuffers_t;
      typedef byteBuffers_t::iterator       byteBuffersIter_t;
      typedef byteBuffers_t::const_iterator byteBuffersCstIter_t;

      typedef std::set<GUID>                guids_t;
      typedef guids_t::iterator             guidsIter_t;
      typedef guids_t::const_iterator       guidsCstIter_t;

      std::set<ClassID>     _classes;
      mutable os::Mutex     _updatedMutex;
      std::set<Shareable *> _updated;
      mutable os::Mutex     _deletedMutex;
      std::set<Shareable *> _deleted;
      mutable os::Mutex     _toUpdateMutex;
      byteBuffers_t         _toUpdate;
      mutable os::Mutex     _toDeleteMutex;
      guids_t               _toDelete;
      mutable os::Mutex     _localMutex;
      local_t               _local;
      int                   _nextInstance;
      bool                  _ownershipCheck;
      ParticipantImpl &     _participant;
      byte                  _cacheId;
   };
}
