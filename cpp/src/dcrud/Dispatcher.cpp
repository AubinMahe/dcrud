#include "Dispatcher.hpp"
#include "ParticipantImpl.hpp"
#include "RequiredImpl.hpp"
#include "ProvidedImpl.hpp"
#include "Operation.hpp"

using namespace dcrud;

IProvided & Dispatcher::provide( const char * interfaceName ) {
   ProvidedImpl * provided = new ProvidedImpl();
   _provided[interfaceName] = provided;
   return *provided;
}

bool Dispatcher::execute(
   const std::string & intrfcName,
   const std::string & opName,
   const Arguments & arguments,
   args_t &          results,
   int               queueNdx,
   byte              callMode )
{
   providedIter_t it = _provided.find( intrfcName );
   if( it == _provided.end()) {
      return false;
   }
   ProvidedImpl * provided  = it->second;
   opsInOutIter_t it2 = provided->_opsInOut.find( opName );
   if( it2 == provided->_opsInOut.end()) {
      return false;
   }
   IOperation * operation = it2->second;
   if( callMode == IRequired::SYNCHRONOUS ) {
      operation->execute( arguments, results );
   }
   else {
      {
         os::Synchronized sync(_operationQueuesMutex);
         Operation * op = new Operation( *operation, arguments, results );
         _operationQueues[queueNdx].push_back( op );
      }
      if( callMode == IRequired::ASYNCHRONOUS_IMMEDIATE ) {
         handleRequests();
      }
   }
   return true;
}

void Dispatcher::handleRequests() {
   os::Synchronized sync(_operationQueuesMutex);
   for( unsigned i = 0; i < OPERATION_QUEUE_COUNT; ++i ) {
      operations_t queue = _operationQueues[i];
      for( operationsIter_t it = queue.begin(); it != queue.end(); ++it ) {
         Operation * operation = *it;
         operation->run();
      }
      queue.clear();
   }
}

IRequired & Dispatcher::require( const char * name ) {
   return *new RequiredImpl( name, _participant );
}
