#include "Dispatcher.h"
#include <coll/List.h>

#include <os/Mutex.h>
#include <util/CheckSysCall.h>

#include "ParticipantImpl.h"

typedef struct Dispatcher_s {

   ParticipantImpl * participant;
   collMap           provided;
   collList          operationQueues[256];
   osMutex           operationQueuesMutex;

} Dispatcher;

typedef struct Operation_s {

   void *          context;
   dcrudIOperation function;

} Operation;

typedef struct OperationCall_s {

   Operation *  operation;
   const char * intrfcName;
   const char * opName;
   collMap      arguments;
   collMap      results;
   int          callId;

} OperationCall;

typedef struct Provided_s {

   collMap opsInOut;

} Provided;

typedef struct Required_s {

   ParticipantImpl * participant;
   const char *      name;

} Required;

static Operation * Operation_new( void * context, dcrudIOperation function ) {
   Operation * This = (Operation *)malloc( sizeof( Operation ));
   This->context  = context;
   This->function = function;
   return This;
}

static OperationCall * OperationCall_new(
   Operation *  operation,
   const char * intrfcName,
   const char * opName,
   int          callId,
   collMap      arguments,
   collMap      results   )
{
   OperationCall * This = (OperationCall *)malloc( sizeof( OperationCall ));
   This->operation  = operation;
   This->intrfcName = intrfcName;
   This->opName     = opName;
   This->callId     = callId;
   This->arguments  = arguments;
   This->results    = results;
   return This;
}

static dcrudIRequired Required_new( ParticipantImpl * participant, const char * name ) {
   Required * This = (Required *)malloc( sizeof( Required ));
   This->participant = participant;
   This->name        = name;
   return (dcrudIRequired)This;
}

static Provided * Provided_new() {
   Provided * This = (Provided *)malloc( sizeof( Provided ));
   This->opsInOut = collMap_new((collComparator)collStringComparator );
   return This;
}

static dcrudCallMode   AsyncDeferred = DCRUD_ASYNCHRONOUS_DEFERRED;
static dcrudQueueIndex DefaultQueue  = DCRUD_DEFAULT_QUEUE;

static bool runAllPendingOperations( collForeach * context ) {
   OperationCall *   opCall      = (OperationCall *  )context->item;
   ParticipantImpl * participant = (ParticipantImpl *)context->user;
   Operation *       op          = opCall->operation;

   op->function( op->context, opCall->arguments, opCall->results );
   if( collMap_size( opCall->results ) > 0 ) {
      ParticipantImpl_sendCall(
         participant, opCall->intrfcName, opCall->opName, opCall->results, -opCall->callId );
   }
   return true;
}

bool dcrudIProvided_addOperation(
   dcrudIProvided  self,
   const char *    name,
   void *          context,
   dcrudIOperation operation )
{
   Provided *  This = (Provided *)self;
   Operation * op   = Operation_new( context, operation );
   return collMap_put( This->opsInOut, (collMapKey)name, op, NULL );
}

dcrudStatus dcrudIRequired_call(
   dcrudIRequired self,
   const char *   opName,
   collMap        arguments,
   dcrudICallback callback,
   int *          callId    )
{
   Required * This = (Required *)self;
   if( ! collMap_hasKey( arguments, "@mode" )) {
      collMap_put( arguments, "@mode", &AsyncDeferred, NULL );
   }
   if( ! collMap_hasKey( arguments, "@queue" )) {
      collMap_put( arguments, "@mode", &DefaultQueue, NULL );
   }
   *callId = ParticipantImpl_call( This->participant, This->name, opName, arguments, callback );
   return DCRUD_NO_ERROR;
}

dcrudIDispatcher dcrudIDispatcher_new( ParticipantImpl * participant ) {
   Dispatcher * This = (Dispatcher *)malloc( sizeof( Dispatcher ));
   unsigned int i;

   memset( This, 0, sizeof( Dispatcher ));
   This->participant = participant;
   This->provided    = collMap_new((collComparator)collStringComparator );
   for( i = 0; i < 256; ++i ) {
      This->operationQueues[i] = collList_new();
   }
   if( utilCheckSysCall( 0 ==
         osMutex_new( &This->operationQueuesMutex ),
         __FILE__, __LINE__, "osMutex_new" ))
   {
      return (dcrudIDispatcher)This;
   }
   return NULL;
}

void dcrudIDispatcher_delete( dcrudIDispatcher * self ) {
   Dispatcher * This = (Dispatcher *)*self;
   if( This ) {
      unsigned int i;
      for( i = 0; i < 256; ++i ) {
         collList_delete( &(This->operationQueues[i]));
      }
      osMutex_delete( &This->operationQueuesMutex );
      free( This );
      *self = NULL;
   }
}

dcrudIProvided dcrudIDispatcher_provide( dcrudIDispatcher self, const char * interfaceName ) {
   Dispatcher * This     = (Dispatcher *)self;
   Provided *    provided = Provided_new();
   collMap_put( This->provided, (collMapKey)interfaceName, provided, NULL );
   return (dcrudIProvided)provided;
}

dcrudIRequired dcrudIDispatcher_require( dcrudIDispatcher self, const char * name ) {
   Dispatcher * This = (Dispatcher *)self;
   return Required_new( This->participant, name );
}

void dcrudIDispatcher_handleRequests( dcrudIDispatcher self ) {
   Dispatcher * This = (Dispatcher *)self;
   unsigned     queueNdx;

   osMutex_take( This->operationQueuesMutex );
   for( queueNdx = 0; queueNdx < 256; ++queueNdx ) {
      collList_foreach(
         This->operationQueues[queueNdx],
         runAllPendingOperations,
         This->participant );
      collList_clear( This->operationQueues[queueNdx] );
   }
   osMutex_release( This->operationQueuesMutex );
}

void dcrudIDispatcher_execute(
   dcrudIDispatcher self,
   const char *     intrfcName,
   const char *     opName,
   collMap          arguments,
   collMap          results,
   int              callId,
   unsigned         queueNdx,
   dcrudCallMode    callMode )
{
   Dispatcher * This      = (Dispatcher *)self;
   Provided *   provided  = collMap_get( This->provided, (collMapKey)intrfcName );
   Operation *  operation = collMap_get( provided->opsInOut, (collMapKey)opName );
   if( callMode == DCRUD_SYNCHRONOUS ) {
      operation->function( operation->context, arguments, results );
      if( collMap_size( results ) > 0 ) {
         ParticipantImpl_sendCall( This->participant, intrfcName, opName, results, -callId );
      }
   }
   else {
      osMutex_take( This->operationQueuesMutex );
      collList_add(
         This->operationQueues[queueNdx],
         OperationCall_new(
            operation,
            intrfcName,
            opName,
            callId,
            arguments,
            results ));
      osMutex_release( This->operationQueuesMutex );
      if( callMode == DCRUD_ASYNCHRONOUS_IMMEDIATE ) {
         dcrudIDispatcher_handleRequests( self );
      }
   }
}
