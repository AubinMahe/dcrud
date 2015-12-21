#include "Dispatcher.h"
#include <coll/List.h>

#include <os/Mutex.h>
#include <util/CheckSysCall.h>

#include "ParticipantImpl.h"

#define OPERATION_QUEUE_COUNT 256

typedef struct Dispatcher_s {

   ParticipantImpl * participant;
   collMap           provided;
   collList          operationQueues[OPERATION_QUEUE_COUNT];
   osMutex           operationQueuesMutex;

} Dispatcher;

typedef struct Operation_s {

   void *          context;
   dcrudIOperation function;

} Operation;

typedef struct OperationCall_s {

   Operation *    operation;
   const char *   intrfcName;
   const char *   opName;
   dcrudArguments arguments;
   int            callId;

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
   Operation *    operation,
   const char *   intrfcName,
   const char *   opName,
   int            callId,
   dcrudArguments arguments )
{
   OperationCall * This = (OperationCall *)malloc( sizeof( OperationCall ));
   This->operation  = operation;
   This->intrfcName = intrfcName;
   This->opName     = opName;
   This->callId     = callId;
   This->arguments  = arguments;
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

static bool runAllPendingOperations( collForeach * context ) {
   OperationCall *   opCall      = (OperationCall *  )context->item;
   ParticipantImpl * participant = (ParticipantImpl *)context->user;
   Operation *       op          = opCall->operation;
   dcrudArguments    results     = op->function( op->context, opCall->arguments );
   if( opCall->callId ) {
      ParticipantImpl_sendCall(
         participant, opCall->intrfcName, opCall->opName, results, -opCall->callId );
   }
   free( opCall );
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

void dcrudIRequired_call(
   dcrudIRequired self,
   const char *   opName,
   dcrudArguments arguments,
   dcrudICallback callback  )
{
   Required * This = (Required *)self;
   ParticipantImpl_call( This->participant, This->name, opName, arguments, callback );
}

dcrudIDispatcher dcrudIDispatcher_new( ParticipantImpl * participant ) {
   Dispatcher * This = (Dispatcher *)malloc( sizeof( Dispatcher ));
   unsigned int i;

   memset( This, 0, sizeof( Dispatcher ));
   This->participant = participant;
   This->provided    = collMap_new((collComparator)collStringComparator );
   for( i = 0; i < OPERATION_QUEUE_COUNT; ++i ) {
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
      for( i = 0; i < OPERATION_QUEUE_COUNT; ++i ) {
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

typedef struct CRUD_s {

   dcrudClassID      classID;
   ParticipantImpl * participant;

} CRUD;

static const char * ICRUD_INTERFACE_NAME    = "dcrud.ICRUD";
static const char * ICRUD_INTERFACE_CREATE  = "create";
static const char * ICRUD_INTERFACE_UPDATE  = "update";
static const char * ICRUD_INTERFACE_DELETE  = "delete";
static const char * ICRUD_INTERFACE_CLASSID = "class-id";
static const char * ICRUD_INTERFACE_GUID    = "guid";

void dcrudICRUD_create( dcrudICRUD self, dcrudArguments how ) {
   CRUD * This = (CRUD *)self;
   dcrudArguments_putClassID( how, ICRUD_INTERFACE_CLASSID, This->classID );
   ParticipantImpl_sendCall(
      This->participant, ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_CREATE, how, 0 );
}

void dcrudICRUD_update( dcrudICRUD self, dcrudShareable what, dcrudArguments how ) {
   CRUD * This = (CRUD *)self;
   dcrudArguments_putGUID( how, ICRUD_INTERFACE_GUID, dcrudShareable_getGUID( what ));
   ParticipantImpl_sendCall(
      This->participant, ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_UPDATE, how, 0 );
}

void dcrudICRUD_delete( dcrudICRUD self, dcrudShareable what ) {
   CRUD *         This = (CRUD *)self;
   dcrudArguments how  = dcrudArguments_new();
   dcrudArguments_putGUID( how, ICRUD_INTERFACE_GUID, dcrudShareable_getGUID( what ));
   ParticipantImpl_sendCall(
      This->participant, ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_DELETE, how, 0 );
}

dcrudICRUD dcrudIDispatcher_requireCRUD( dcrudIDispatcher self, dcrudClassID classID ) {
   Dispatcher * This = (Dispatcher *)self;
   CRUD *       crud = (CRUD *)malloc( sizeof( CRUD ));
   crud->classID     = classID;
   crud->participant = This->participant;
   return (dcrudICRUD)crud;
}

void dcrudIDispatcher_handleRequests( dcrudIDispatcher self ) {
   Dispatcher * This = (Dispatcher *)self;
   unsigned     queueNdx;

   osMutex_take( This->operationQueuesMutex );
   for( queueNdx = 0; queueNdx < OPERATION_QUEUE_COUNT; ++queueNdx ) {
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
   dcrudArguments   args,
   int              callId,
   unsigned         queueNdx,
   dcrudCallMode    callMode )
{
   Dispatcher * This     = (Dispatcher *)self;
   Provided *   provided = collMap_get( This->provided, (collMapKey)intrfcName );
   if( provided == NULL ) {
      fprintf( stderr, "%s:%d:Unexpected operation: '%s.%s'\n",
         __FILE__, __LINE__, intrfcName, opName );
   }
   else {
      Operation * operation = collMap_get( provided->opsInOut, (collMapKey)opName );
      if( operation == NULL ) {
         fprintf( stderr, "%s:%d:Unexpected operation: '%s.%s'\n",
            __FILE__, __LINE__, intrfcName, opName );
      }
      else if( callMode == DCRUD_SYNCHRONOUS ) {
         dcrudArguments results = operation->function( operation->context, args );
         if( callId ) {
            ParticipantImpl_sendCall( This->participant, intrfcName, opName, results, -callId );
         }
      }
      else {
         osMutex_take( This->operationQueuesMutex );
         collList_add( This->operationQueues[queueNdx],
            OperationCall_new( operation, intrfcName, opName, callId, args ));
         osMutex_release( This->operationQueuesMutex );
         if( callMode == DCRUD_ASYNCHRONOUS_IMMEDIATE ) {
            dcrudIDispatcher_handleRequests( self );
         }
      }
   }
}
