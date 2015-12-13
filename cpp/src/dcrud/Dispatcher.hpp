#pragma once

#include <dcrud/Arguments.hpp>
#include <dcrud/IDispatcher.hpp>
#include <dcrud/IProvided.hpp>
#include <dcrud/IRequired.hpp>

#include <os/Mutex.hpp>

#include <vector>

namespace dcrud {

   struct ParticipantImpl;
   struct ProvidedImpl;
   struct Operation;

   struct Dispatcher : public IDispatcher {

      typedef std::map<std::string, ProvidedImpl *> provided_t;
      typedef provided_t::iterator                  providedIter_t;

      typedef std::vector<Operation *>              operations_t;
      typedef operations_t::iterator                operationsIter_t;

      static const unsigned int OPERATION_QUEUE_COUNT = 256;

      ParticipantImpl & _participant;
      provided_t        _provided;
      operations_t      _operationQueues[OPERATION_QUEUE_COUNT];
      os::Mutex         _operationQueuesMutex;

      Dispatcher( ParticipantImpl & participant ) :
         _participant( participant )
      {}

      IProvided & provide( const char * interfaceName );

      bool execute(
         const std::string & intrfcName,
         const std::string & opName,
         const Arguments &   arguments,
         args_t &            results,
         int                 queueNdx,
         byte                callMode );

      void handleRequests();

      IRequired & require( const char * name );
   };
}
