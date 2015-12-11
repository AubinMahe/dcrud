#include "Dispatcher.hpp"
#include "ParticipantImpl.hpp"

#include <dcrud/IOperation.hpp>
#include <dcrud/IDispatcher.hpp>
#include <dcrud/IProvided.hpp>
#include <dcrud/IRequired.hpp>

#include <os/Mutex.hpp>

#include <vector>

#define OPERATION_QUEUE_COUNT          256

namespace dcrud {

   typedef void ( * runnable_t )( void );

   struct ProvidedImpl : public IProvided {

      typedef std::map<std::string, IOperation *> opsInOut_t;
      typedef opsInOut_t::iterator                opsInOutIter_t;

      opsInOut_t _opsInOut;

      void addOperation( const char * opName, IOperation & executor ) {
         _opsInOut[opName] = &executor;
      }
   };

   struct RequiredImpl : public IRequired {

      std::string       _name;
      ParticipantImpl & _participant;

      RequiredImpl( const char * name, ParticipantImpl & participant ) :
         _name       ( name        ),
         _participant( participant )
      {}

      int call( const char * opName ) {
         Arguments arguments;
         arguments.args["@mode" ] = &IRequired::ASYNCHRONOUS_DEFERRED;
         arguments.args["@queue"] = &IRequired::DEFAULT_QUEUE;
         return _participant.call( _name, opName, arguments, (ICallback *)0 );
      }

      int call( const char * opName, Arguments & arguments ) {
         if( arguments.args.find( "@mode" ) == arguments.args.end()) {
            arguments.args["@mode" ] = &IRequired::ASYNCHRONOUS_DEFERRED;
         }
         if( arguments.args.find( "@queue" ) == arguments.args.end()) {
            arguments.args["@queue"] = &IRequired::DEFAULT_QUEUE;
         }
         return _participant.call( _name, opName, arguments, (ICallback *)0 );
      }

      int call( const char * opName, Arguments & arguments, ICallback & callback ) {
         if( arguments.args.find( "@mode" ) == arguments.args.end()) {
            arguments.args["@mode" ] = &IRequired::ASYNCHRONOUS_DEFERRED;
         }
         if( arguments.args.find( "@queue" ) == arguments.args.end()) {
            arguments.args["@queue"] = &IRequired::DEFAULT_QUEUE;
         }
         return _participant.call( _name, opName, arguments, &callback );
      }
   };

   struct Operation {

      IOperation &      _operation;
      const Arguments & _arguments;
      args_t &          _results;

      Operation( IOperation & operation, const Arguments & arguments, args_t & results ) :
         _operation( operation ),
         _arguments( arguments ),
         _results  ( results   )
      {}

      void run() {
         _operation.execute( _arguments, _results );
      }
   };

   struct Dispatcher : IDispatcher {

      typedef std::map<std::string, ProvidedImpl *> provided_t;
      typedef provided_t                            providedIter_t;

      typedef std::vector<Operation *>              operations_t;
      typedef operations_t                          operationsIter_t;

      ParticipantImpl & _participant;
      provided_t        _provided;
      operations_t      _operationQueues[OPERATION_QUEUE_COUNT];
      os::Mutex         _operationQueuesMutex;

      Dispatcher( ParticipantImpl & participant ) :
         _participant( participant )
      {}

      IProvided & provide( const char * interfaceName ) {
         ProvidedImpl * provided = new ProvidedImpl();
         _provided[interfaceName] = provided;
         return *provided;
      }

      bool execute(
         const char *      intrfcName,
         const char *      opName,
         const Arguments & arguments,
         args_t &          results,
         int               queueNdx,
         byte              callMode )
      {
         auto it = _provided.find( intrfcName );
         if( it == _provided.end()) {
            return false;
         }
         ProvidedImpl * provided  = it->second;
         auto it2 = provided->_opsInOut.find( opName );
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

      void handleRequests() {
         os::Synchronized sync(_operationQueuesMutex);
         for( unsigned i = 0; i < OPERATION_QUEUE_COUNT; ++i ) {
            auto queue = _operationQueues[i];
            for( auto it = queue.begin(); it != queue.end(); ++it ) {
               auto operation = *it;
               operation->run();
            }
            queue.clear();
         }
      }

      IRequired & require( const char * name ) {
         return *new RequiredImpl( name, _participant );
      }
   };
}
