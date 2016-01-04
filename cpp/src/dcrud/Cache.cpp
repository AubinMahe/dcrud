#include "Cache.hpp"

#include <dcrud/Shareable.hpp>

#include <stdio.h>

using namespace dcrud;

#ifdef NO_ERROR
#  undef NO_ERROR
#endif

Cache::Cache( ParticipantImpl & participant ) :
   _nextInstance  ( 1U          ),
   _ownershipCheck( false       ),
   _participant   ( participant )
{}

Cache::~ Cache() {
}

bool Cache::owns( const dcrud::GUID & id ) const {
   return _participant.getPublisherId() == id._publisher;
}

Status Cache::create( Shareable & item ) {
   if( item._id.isShared()) {
      return ALREADY_CREATED;
   }
   item._id._publisher = _participant.getPublisherId();
   item._id._instance  = _nextInstance++;
   {
      os::Synchronized sync( _localMutex );
      _local[item._id] = &item;
   }
   {
      os::Synchronized sync( _updatedMutex );
      _updated.insert( &item );
   }
   return NO_ERROR;
}

Shareable * Cache::read( const dcrud::GUID & id ) const {
   localCstIter_t it = _local.find( id );
   if( it == _local.end()) {
      return 0;
   }
   return it->second;
}


Status Cache::update( Shareable & item ) {
   if( _ownershipCheck && ! owns( item._id )) {
      return NOT_OWNER;
   }
   if( item._id._instance == 0 ) {
      return NOT_CREATED;
   }
   {
      os::Synchronized sync( _localMutex );
      localCstIter_t it = _local.find( item._id );
      if( it == _local.end()) {
         return NOT_IN_THIS_CACHE;
      }
   }
   {
      os::Synchronized sync( _updatedMutex );
      _updated.insert( &item );
   }
   return NO_ERROR;
}

Status Cache::deleTe( Shareable & item ) {
   if( _ownershipCheck && ! owns( item._id )) {
      return NOT_OWNER;
   }
   {
      os::Synchronized sync( _localMutex );
      if( _local.erase( item._id ) < 1 ) {
         return NOT_IN_THIS_CACHE;
      }
   }
   {
      os::Synchronized sync( _deletedMutex );
      _deleted.insert( &item );
   }
   return NO_ERROR;
}

void Cache::values( shareables_t & snapshot ) const {
   os::Synchronized sync( _localMutex );
   snapshot.clear();
   for( localCstIter_t it = _local.begin(); it != _local.end(); ++it ) {
      snapshot.insert( it->second );
   }
}

bool Cache::select( shareablePredicate_t query, shareables_t & snapshot ) const {
   os::Synchronized sync( _localMutex );
   snapshot.clear();
   for( localCstIter_t it = _local.begin(); it != _local.end(); ++it ) {
      if( query( *it->second )) {
         snapshot.insert( it->second );
      }
   }
   return ! snapshot.empty();
}

void Cache::publish( void ) {
   {
      os::Synchronized sync( _updatedMutex );
      _participant.publishUpdated( _updated );
      _updated.clear();
   }
   {
      os::Synchronized sync( _deletedMutex );
      _participant.publishDeleted( _deleted );
      _deleted.clear();
   }
}

void Cache::subscribe( const ClassID & id ) {
   _classes.insert( id );
}

void Cache::refresh( void ) {
   {
      os::Synchronized sync( _localMutex );
      {
         os::Synchronized sync2( _toUpdateMutex );
         for( byteBuffersIter_t it = _toUpdate.begin(); it != _toUpdate.end(); ++it ) {
            io::ByteBuffer * update   = *it;
            dcrud::GUID             id       = dcrud::GUID   ::unserialize( *update );
            ClassID          classId  = ClassID::unserialize( *update );
            localIter_t      itemIter = _local.find( id );
            if( itemIter == _local.end()) {
               Shareable * item = _participant.newInstance( classId, *update );
               if( item ) {
                  item->_id.set( id );
                  _local[id] = item;
               }
               else {
                  fprintf( stderr, "Unknown %s of %s\n",
                     classId.toString().c_str(), id.toString().c_str());
               }
            }
            else if( ! _ownershipCheck || ! owns( id )) {
               itemIter->second->unserialize( *update );
            }
            delete update;
         }
         _toUpdate.clear();
      }
      {
         os::Synchronized sync3( _toDeleteMutex );
         for( guidsIter_t it = _toDelete.begin(); it != _toDelete.end(); ++it ) {
            _local.erase( *it );
         }
         _toDelete.clear();
      }
   }
}
void Cache::updateFromNetwork( io::ByteBuffer * source ) {
   os::Synchronized sync( _toUpdateMutex );
   _toUpdate.insert( source );
}

void Cache::deleteFromNetwork( const dcrud::GUID & id ) {
   os::Synchronized sync( _toDeleteMutex );
   _toDelete.insert( id );
}
