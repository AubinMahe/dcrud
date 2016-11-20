#pragma once

#include <dcrud/Arguments.hpp>
#include <dcrud/IDispatcher.hpp>
#include <dcrud/IProvided.hpp>
#include <dcrud/IRequired.hpp>

#include <os/Mutex.hpp>

#include <vector>

namespace dcrud {

   class ParticipantImpl;
   class ProvidedImpl;
   class Operation;

   class Dispatcher : public IDispatcher {
   public:

      Dispatcher( ParticipantImpl & participant ) :
         _participant         ( participant ),
         _providedMutex       (),
         _provided            (),
         _operationQueuesMutex()
      {}

      virtual IProvided & provide( const std::string & interfaceName );

      virtual IRequired & require( const std::string & interfaceName );

      virtual ICRUD &     requireCRUD( const ClassID & classID );

      virtual bool execute(
         const std::string & intrfcName,
         const std::string & opName,
         const Arguments &   arguments,
         int                 callId,
         byte                queueNdx,
         byte                callMode );

      virtual void executeCrud( const std::string & opName, const Arguments & arguments );

      virtual void handleRequests();

   private:

      typedef std::map<std::string, ProvidedImpl *> provided_t;
      typedef provided_t::iterator                  providedIter_t;

      typedef std::vector<Operation *>              operations_t;
      typedef operations_t::iterator                operationsIter_t;

      static const unsigned int OPERATION_QUEUE_COUNT = 256;

      ParticipantImpl & _participant;
      os::Mutex         _providedMutex;
      provided_t        _provided;
      operations_t      _operationQueues[OPERATION_QUEUE_COUNT];
      os::Mutex         _operationQueuesMutex;
   };
}
