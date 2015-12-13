#include "Dispatcher.hpp"
#include "ParticipantImpl.hpp"
#include "RequiredImpl.hpp"
#include "ProvidedImpl.hpp"
#include "Operation.hpp"

using namespace dcrud;

IProvided & Dispatcher::provide( const char * interfaceName ) {
   os::Synchronized sync( _providedMutex );
   providedIter_t it = _provided.find( interfaceName );
   if( it == _provided.end()) {
      ProvidedImpl * provided = new ProvidedImpl();
      _provided[interfaceName] = provided;
      return *provided;
   }
   return *it->second;
}

bool Dispatcher::execute(
   const std::string & intrfcName,
   const std::string & opName,
   const Arguments & arguments,
   args_t &          results,
   int               queueNdx,
   byte              callMode )
{
   ProvidedImpl * provided = 0;
   {
      os::Synchronized sync( _providedMutex );
      providedIter_t it = _provided.find( intrfcName );
      if( it == _provided.end()) {
         return false;
      }
      provided  = it->second;
   }
   opsInOutIter_t it = provided->_opsInOut.find( opName );
   if( it == provided->_opsInOut.end()) {
      return false;
   }
   IOperation * operation = it->second;
   if( callMode == IRequired::SYNCHRONOUS ) {
      operation->execute( arguments, results );
   }
   else {
      {
         os::Synchronized sync(_operationQueuesMutex);
         _operationQueues[queueNdx].push_back( new Operation( *operation, arguments, results ));
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
