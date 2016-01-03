#include "Dispatcher.hpp"
#include "ParticipantImpl.hpp"
#include "RequiredImpl.hpp"
#include "ProvidedImpl.hpp"
#include "Operation.hpp"

#include <dcrud/ICRUD.hpp>
#include <dcrud/Shareable.hpp>

namespace dcrud {

   class CRUD : public ICRUD {
   public:

      CRUD( ParticipantImpl & participant, const ClassID & classID ) :
         _participant( participant ),
         _classID    ( classID     )
      {}

      virtual void create( const Arguments & how ) {
         Arguments args( how );
         args.put( ICRUD_INTERFACE_CLASSID, _classID );
         _participant.call( ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_CREATE, &args, 0 );
      }

      virtual void update( Shareable & what, const Arguments & how ) {
         Arguments args( how );
         args.put( ICRUD_INTERFACE_GUID, what.getGUID());
         _participant.call( ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_UPDATE, &args, 0 );
      }

      virtual void deleTe( Shareable & what ) {
         Arguments args;
         args.put( ICRUD_INTERFACE_GUID, what.getGUID());
         _participant.call( ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_DELETE, &args, 0 );
      }

   private:

      ParticipantImpl & _participant;
      ClassID           _classID;
   };
}
using namespace dcrud;

IProvided & Dispatcher::provide( const std::string & interfaceName ) {
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
         fprintf( stderr, "No provided interface named '%s' found\n", intrfcName.c_str());
         return false;
      }
      provided  = it->second;
   }
   IOperation * operation = provided->getOperation( opName );
   if( ! operation ) {
      fprintf( stderr,
         "Provided interface named '%s' is found but it's null! (internal error)\n",
         intrfcName.c_str());
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
            new Operation( operation, intrfcName, opName, arguments, callId ));
      }
      if( callMode == IRequired::ASYNCHRONOUS_IMMEDIATE ) {
         handleRequests();
      }
   }
   return true;
}

ICRUD & Dispatcher::requireCRUD( const ClassID & classID ) {
   return *new CRUD( this->_participant, classID );
}

void Dispatcher::executeCrud( const std::string & opName, const Arguments & arguments ) {
   os::Synchronized sync(_operationQueuesMutex);
   _operationQueues[IRequired::DEFAULT_QUEUE].push_back( new Operation( opName, arguments ));
}

IRequired & Dispatcher::require( const std::string & name ) {
   return *new RequiredImpl( name, _participant );
}

void Dispatcher::handleRequests() {
   os::Synchronized sync(_operationQueuesMutex);
   for( unsigned i = 0; i < OPERATION_QUEUE_COUNT; ++i ) {
      operations_t & queue = _operationQueues[i];
      for( operationsIter_t it = queue.begin(); it != queue.end(); ++it ) {
         Operation * op  = *it;
         try {
            IOperation * iOp = op->_operation;
            if( iOp ) {
               Arguments * results = iOp->execute( _participant, op->_arguments );
               if( op->_callId > 0 ) {
                  _participant.call( op->_intrfcName, op->_opName, results, -op->_callId );
               }
            }
            else {
               ClassID classID;
               GUID    id;
               if(   op->_opName == ICRUD_INTERFACE_CREATE
                  && op->_arguments.get( ICRUD_INTERFACE_CLASSID, classID ))
               {
                  _participant.create( classID, op->_arguments );
               }
               else if( op->_opName == ICRUD_INTERFACE_UPDATE
                  && op->_arguments.get( ICRUD_INTERFACE_GUID, id ))
               {
                  _participant.update( id, op->_arguments );
               }
               else if( op->_opName == ICRUD_INTERFACE_DELETE
                  && op->_arguments.get( ICRUD_INTERFACE_GUID, id ))
               {
                  _participant.deleTe( id );
               }
               else {
                  fprintf( stderr, "%s:%d: Unexpected Publisher operation '%s'\n",
                     __FILE__, __LINE__, op->_opName.c_str());
               }
            }
         }
         catch( const std::exception & x ) {
            fprintf( stderr, "%s\n", x.what());
         }
         delete op;
      }
      queue.clear();
   }
}
