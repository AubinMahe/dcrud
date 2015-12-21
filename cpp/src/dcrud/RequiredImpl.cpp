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

RequiredImpl::RequiredImpl( const std::string & name, ParticipantImpl & participant ) :
   _name       ( name        ),
   _participant( participant )
{}

void RequiredImpl::call( const std::string & opName ) {
   _participant.call( _name, opName, 0, 0 );
}

void RequiredImpl::call( const std::string & opName, const Arguments & arguments ) {
   _participant.call( _name, opName, &arguments, 0 );
}

int RequiredImpl::call( const std::string & opName, const Arguments & arguments, ICallback & callback ) {
   return _participant.call( _name, opName, &arguments, callback );
}
