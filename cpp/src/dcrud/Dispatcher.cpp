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
   const Arguments &   arguments,
   int                 callId,
   byte                queueNdx,
   byte                callMode )
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
   IOperation * operation = provided->getOperation( opName );
   if( ! operation ) {
      return false;
   }
   if( callMode == IRequired::SYNCHRONOUS ) {
      Arguments * results = operation->execute( _participant, arguments );
      if( callId ) {
         _participant.call( intrfcName, opName, results, -callId );
      }
   }
   else {
      {
         os::Synchronized sync(_operationQueuesMutex);
         _operationQueues[queueNdx].push_back(
            new Operation( *operation, arguments, intrfcName, opName, callId ));
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
      operations_t & queue = _operationQueues[i];
      for( operationsIter_t it = queue.begin(); it != queue.end(); ++it ) {
         Operation *  op      = *it;
         IOperation & iOp     = op->_operation;
         Arguments *  results = iOp.execute( _participant, op->_arguments );
         if( op->_callId > 0 ) {
            try {
               _participant.call( op->_intrfcName, op->_opName, results, -op->_callId );
            }
            catch( const std::exception & x ) {
               fprintf( stderr, "%s\n", x.what());
            }
         }
         delete op;
      }
      queue.clear();
   }
}

IRequired & Dispatcher::require( const char * name ) {
   return *new RequiredImpl( name, _participant );
}
