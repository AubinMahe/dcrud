#include "RequiredImpl.hpp"
#include "ParticipantImpl.hpp"

using namespace dcrud;

const byte IRequired::VERY_URGENT_QUEUE;
const byte IRequired::URGENT_QUEUE;
const byte IRequired::DEFAULT_QUEUE;
const byte IRequired::NON_URGENT_QUEUE;

const byte IRequired::SYNCHRONOUS;
const byte IRequired::ASYNCHRONOUS_DEFERRED;
const byte IRequired::ASYNCHRONOUS_IMMEDIATE;

int RequiredImpl::call( const char * opName ) {
   Arguments arguments;
   arguments.put( "@mode" , IRequired::ASYNCHRONOUS_DEFERRED );
   arguments.put( "@queue", IRequired::DEFAULT_QUEUE );
   return _participant.call( _name, opName, arguments, (ICallback *)0 );
}

int RequiredImpl::call( const char * opName, Arguments & arguments ) {
   byte v;
   if( ! arguments.get( "@mode", v )) {
      arguments.put( "@mode" , IRequired::ASYNCHRONOUS_DEFERRED );
   }
   if( ! arguments.get( "@queue", v )) {
      arguments.put( "@queue", IRequired::DEFAULT_QUEUE );
   }
   return _participant.call( _name, opName, arguments, (ICallback *)0 );
}

int RequiredImpl::call( const char * opName, Arguments & arguments, ICallback & callback ) {
   byte v;
   if( ! arguments.get( "@mode", v )) {
      arguments.put( "@mode" , IRequired::ASYNCHRONOUS_DEFERRED );
   }
   if( ! arguments.get( "@queue", v )) {
      arguments.put( "@queue", IRequired::DEFAULT_QUEUE );
   }
   return _participant.call( _name, opName, arguments, &callback );
}
