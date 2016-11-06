#include "Dispatcher.h"
#include <dcrud/Network.h>
#include "ParticipantImpl.h"
#include "IProtocol.h"
#include "magic.h"
#include "poolSizes.h"

#include <util/Pool.h>
#include <util/String.h>
#include <coll/List.h>
#include <os/Mutex.h>

#include <string.h>

#define OPERATION_QUEUE_COUNT    256

typedef struct dcrudIDispatcherImpl_s {

   unsigned int            magic;
   dcrudIParticipantImpl * participant;
   collMap                 provided;
   collList                operationQueues[OPERATION_QUEUE_COUNT];
   osMutex                 operationQueuesMutex;

} dcrudIDispatcherImpl;

UTIL_DEFINE_SAFE_CAST( dcrudIDispatcher )
UTIL_POOL_DECLARE    ( dcrudIDispatcherImpl )

typedef struct dcrudOperation_s {

   void *          context;
   dcrudIOperation function;

} dcrudOperation;

UTIL_POOL_DECLARE(dcrudOperation)

typedef struct dcrudOperationCall_s {

   dcrudOperation * operation;
   char *           intrfcName;
   char *           opName;
   dcrudArguments   arguments;
   int              callId;

} dcrudOperationCall;

UTIL_POOL_DECLARE(dcrudOperationCall)

typedef struct dcrudIProvidedImpl_s {

   unsigned int magic;
   collMap      opsInOut;

} dcrudIProvidedImpl;

UTIL_DEFINE_SAFE_CAST(dcrudIProvided)
UTIL_POOL_DECLARE(dcrudIProvidedImpl)

typedef struct dcrudIRequiredImpl_s {

   unsigned int            magic;
   dcrudIParticipantImpl * participant;
   const char *            name;

} dcrudIRequiredImpl;

UTIL_DEFINE_SAFE_CAST(dcrudIRequired)
UTIL_POOL_DECLARE(dcrudIRequiredImpl)

typedef struct dcrudICRUDImpl_s {

   unsigned int            magic;
   dcrudClassID            classID;
   dcrudIParticipantImpl * participant;

} dcrudICRUDImpl;

UTIL_DEFINE_SAFE_CAST(dcrudICRUD)
UTIL_POOL_DECLARE(dcrudICRUDImpl)

static utilStatus dcrudOperation_new(
   dcrudOperation ** self,
   void *            context,
   dcrudIOperation   function )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudOperation * This = NULL;
      UTIL_ALLOCATE( dcrudOperation );
      if( UTIL_STATUS_NO_ERROR == status ) {
         This->context  = context;
         This->function = function;
      }
   }
   return status;
}

static utilStatus dcrudOperation_delete( dcrudOperation ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      UTIL_RELEASE( dcrudOperation )
   }
   return status;
}

static utilStatus dcrudOperationCall_new(
   dcrudOperationCall ** self,
   dcrudOperation *      operation,
   const char *          intrfcName,
   const char *          opName,
   int                   callId,
   dcrudArguments        arguments )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudOperationCall * This = NULL;
      UTIL_ALLOCATE( dcrudOperationCall );
      if( UTIL_STATUS_NO_ERROR == status ) {
         This->operation  = operation;
         CHK(__FILE__,__LINE__,utilString_dup( &This->intrfcName, intrfcName ))
         CHK(__FILE__,__LINE__,utilString_dup( &This->opName    , opName     ))
         This->callId     = callId;
         This->arguments  = arguments;
      }
   }
   return status;
}

static utilStatus dcrudOperationCall_delete( dcrudOperationCall ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudOperationCall * This = *self;
      utilString_delete    ( &This->intrfcName );
      utilString_delete    ( &This->opName     );
      dcrudArguments_delete( &This->arguments  );
      UTIL_RELEASE( dcrudOperationCall )
   }
   return status;
}

static utilStatus dcrudIRequiredImpl_new(
   dcrudIRequired *        self,
   dcrudIParticipantImpl * participant,
   const char *            name        )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIRequiredImpl * This = NULL;
      UTIL_ALLOCATE_ADT( dcrudIRequired, self, This );
      if( UTIL_STATUS_NO_ERROR == status ) {
         This->participant = participant;
         This->name        = name;
      }
   }
   return status;
}

static utilStatus dcrudIRequiredImpl_delete( dcrudIRequired * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      UTIL_RELEASE( dcrudIRequiredImpl )
   }
   return status;
}

static utilStatus dcrudIProvidedImpl_new( dcrudIProvided * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIProvidedImpl * This = NULL;
      UTIL_ALLOCATE_ADT( dcrudIProvided, self, This );
      if( status == UTIL_STATUS_NO_ERROR ) {
         CHK(__FILE__,__LINE__,collMap_new( &(This->opsInOut), (collComparator)collStringCompare ))
      }
   }
   return status;
}

static utilStatus deleteOperation( collForeach * context ) {
   dcrudOperation * op = (dcrudOperation *)context->value;
   return dcrudOperation_delete( &op );
}

static utilStatus dcrudIProvidedImpl_delete( dcrudIProvided * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   dcrudIProvidedImpl * This = dcrudIProvided_safeCast( *self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      collMap_foreach( This->opsInOut, deleteOperation, NULL, NULL );
      collMap_delete( &This->opsInOut );
      UTIL_RELEASE(dcrudIProvidedImpl);
   }
   return status;
}

utilStatus dcrudIProvided_addOperation(
   dcrudIProvided  self,
   const char *    name,
   void *          context,
   dcrudIOperation operation )
{
   utilStatus           status = UTIL_STATUS_NO_ERROR;
   dcrudIProvidedImpl * This   = dcrudIProvided_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      dcrudOperation * op = NULL;
      status = dcrudOperation_new( &op, context, operation );
      if( status == UTIL_STATUS_NO_ERROR ) {
         status = collMap_put( This->opsInOut, (collMapKey)name, op, NULL );
         if( status != UTIL_STATUS_NO_ERROR ) {
            dcrudOperation_delete( &op );
         }
      }
   }
   return status;
}

utilStatus dcrudIRequired_call(
   dcrudIRequired self,
   const char *   opName,
   dcrudArguments arguments,
   dcrudICallback callback  )
{
   utilStatus           status = UTIL_STATUS_NO_ERROR;
   dcrudIRequiredImpl * This   = dcrudIRequired_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      status = dcrudIParticipantImpl_call(
         This->participant, This->name, opName, arguments, callback );
   }
   return status;
}

utilStatus dcrudIDispatcher_new( dcrudIDispatcher * self, dcrudIParticipantImpl * participant ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIDispatcherImpl * This = NULL;
      UTIL_ALLOCATE_ADT( dcrudIDispatcher, self, This );
      if( UTIL_STATUS_NO_ERROR == status ) {
         unsigned int i;
         This->participant = participant;
         CHK(__FILE__,__LINE__,collMap_new( &(This->provided), (collComparator)collStringCompare ))
         for( i = 0; (status == UTIL_STATUS_NO_ERROR) && (i < OPERATION_QUEUE_COUNT); ++i ) {
            CHK(__FILE__,__LINE__,collList_new(&(This->operationQueues[i])))
         }
         CHK(__FILE__,__LINE__,osMutex_new( &This->operationQueuesMutex ))
      }
   }
   return status;
}

utilStatus deleteProvided( collForeach * context ) {
   dcrudIProvided self = context->value;
   return dcrudIProvidedImpl_delete( &self );
}

utilStatus dcrudIDispatcher_delete( dcrudIDispatcher * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIDispatcherImpl * This = dcrudIDispatcher_safeCast( *self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         unsigned int i;
         collMap_foreach( This->provided, deleteProvided, NULL, NULL );
         collMap_delete( &This->provided );
         for( i = 0; i < OPERATION_QUEUE_COUNT; ++i ) {
            collList_delete(&(This->operationQueues[i]));
         }
         osMutex_delete(&(This->operationQueuesMutex));
         UTIL_RELEASE( dcrudIDispatcherImpl )
      }
   }
   return status;
}

utilStatus dcrudIDispatcher_provide(
   dcrudIDispatcher self,
   const char *     interfaceName,
   dcrudIProvided * result )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIDispatcherImpl * This = dcrudIDispatcher_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         dcrudIProvided provided = NULL;
         status = dcrudIProvidedImpl_new( &provided );
         if( status == UTIL_STATUS_NO_ERROR ) {
            status = collMap_put( This->provided, (collMapKey)interfaceName, provided, NULL );
         }
         if( status != UTIL_STATUS_NO_ERROR ) {
            dcrudIProvidedImpl_delete( &provided );
            provided = NULL;
         }
         *result = (dcrudIProvided)provided;
      }
   }
   return status;
}

utilStatus dcrudIDispatcher_require(
   dcrudIDispatcher self,
   const char *     name,
   dcrudIRequired * result )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( result == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      dcrudIDispatcherImpl * This = dcrudIDispatcher_safeCast( self, &status );
      if( status == UTIL_STATUS_NO_ERROR ) {
         status = dcrudIRequiredImpl_new( result, This->participant, name );
      }
      if( status != UTIL_STATUS_NO_ERROR ) {
         dcrudIRequiredImpl_delete( result );
      }
   }
   return status;
}

utilStatus dcrudICRUD_create( dcrudICRUD self, dcrudArguments how ) {
   utilStatus       status = UTIL_STATUS_NO_ERROR;
   dcrudICRUDImpl * This   = dcrudICRUD_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      status = dcrudArguments_putClassID( how, ICRUD_INTERFACE_CLASSID, This->classID );
      if( status == UTIL_STATUS_NO_ERROR ) {
         status = dcrudIParticipantImpl_sendCall(
            This->participant, ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_CREATE, how, 0 );
      }
   }
   return status;
}

utilStatus dcrudICRUD_update( dcrudICRUD self, dcrudShareable what, dcrudArguments how ) {
   utilStatus       status = UTIL_STATUS_NO_ERROR;
   dcrudICRUDImpl * This   = dcrudICRUD_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      dcrudGUID guid;
      status = dcrudShareable_getGUID( what, &guid );
      if( status == UTIL_STATUS_NO_ERROR ) {
         status = dcrudArguments_putGUID( how, ICRUD_INTERFACE_GUID, guid );
         if( status == UTIL_STATUS_NO_ERROR ) {
            status = dcrudIParticipantImpl_sendCall(
               This->participant, ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_UPDATE, how, 0 );
         }
      }
   }
   return status;
}

utilStatus dcrudICRUD_delete( dcrudICRUD self, dcrudShareable what ) {
   utilStatus       status = UTIL_STATUS_NO_ERROR;
   dcrudICRUDImpl * This   = dcrudICRUD_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      dcrudArguments how  = NULL;
      status = dcrudArguments_new( &how );
      if( status == UTIL_STATUS_NO_ERROR ) {
         dcrudGUID guid;
         status = dcrudShareable_getGUID( what, &guid );
         if( status == UTIL_STATUS_NO_ERROR ) {
            status = dcrudArguments_putGUID( how, ICRUD_INTERFACE_GUID, guid );
            if( status == UTIL_STATUS_NO_ERROR ) {
               status =
                  dcrudIParticipantImpl_sendCall(
                     This->participant, ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_DELETE, how, 0 );
            }
         }
      }
   }
   return status;
}

utilStatus dcrudIDispatcher_requireCRUD(
   dcrudIDispatcher self,
   dcrudClassID     classID,
   dcrudICRUD *     iCrud   )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   dcrudIDispatcherImpl * This = dcrudIDispatcher_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      dcrudICRUDImpl * crud = NULL;
      UTIL_ALLOCATE_ADT( dcrudICRUD, iCrud, crud );
      crud->classID     = classID;
      crud->participant = This->participant;
      *iCrud = (dcrudICRUD)crud;
   }
   return status;
}

utilStatus dcrudIDispatcher_executeCrud(
   dcrudIDispatcher self,
   const char *     opName,
   dcrudArguments   args   )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   dcrudIDispatcherImpl * This = dcrudIDispatcher_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      dcrudOperationCall * opCall = NULL;
      status = osMutex_take( This->operationQueuesMutex );
      if( status == UTIL_STATUS_NO_ERROR ) {
         status = dcrudOperationCall_new( &opCall, NULL, ICRUD_INTERFACE_NAME, opName, 0, args );
         if( status == UTIL_STATUS_NO_ERROR ) {
            status = collList_add( This->operationQueues[DCRUD_DEFAULT_QUEUE], opCall );
            if( status != UTIL_STATUS_NO_ERROR ) {
               dcrudOperationCall_delete( &opCall );
            }
         }
         osMutex_release( This->operationQueuesMutex );
      }
   }
   return status;
}

utilStatus dcrudIDispatcher_execute(
   dcrudIDispatcher self,
   const char *     intrfcName,
   const char *     opName,
   dcrudArguments   args,
   int              callId,
   unsigned         queueNdx,
   dcrudCallMode    callMode )
{
   utilStatus             status = UTIL_STATUS_NO_ERROR;
   dcrudIDispatcherImpl * This   = dcrudIDispatcher_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      dcrudIProvidedImpl * provided = NULL;
      status = collMap_get( This->provided, (collMapKey)intrfcName, &provided );
      if( status == UTIL_STATUS_NO_ERROR ) {
         dcrudOperation * operation = NULL;
         status = collMap_get( provided->opsInOut, (collMapKey)opName, &operation );
         if( status == UTIL_STATUS_NO_ERROR ) {
            if( callMode == DCRUD_SYNCHRONOUS ) {
               dcrudArguments results = operation->function( operation->context, args );
               if( callId ) {
                  dcrudIParticipantImpl_sendCall(
                     This->participant, intrfcName, opName, results, -callId );
               }
            }
            else {
               status = osMutex_take( This->operationQueuesMutex );
               if( status == UTIL_STATUS_NO_ERROR ) {
                  dcrudOperationCall * opCall = NULL;
                  status =
                     dcrudOperationCall_new(
                        &opCall, operation, intrfcName, opName, callId, args );
                  if( status == UTIL_STATUS_NO_ERROR ) {
                     status = collList_add( This->operationQueues[queueNdx], opCall );
                     osMutex_release( This->operationQueuesMutex );
                     if( callMode == DCRUD_ASYNCHRONOUS_IMMEDIATE ) {
                        dcrudIDispatcher_handleRequests( self );
                     }
                  }
               }
            }
         }
      }
   }
   return status;
}

static utilStatus runAllPendingOperations( collForeach * context ) {
   utilStatus              status      = UTIL_STATUS_NO_ERROR;
   dcrudOperationCall *    opCall;
   dcrudIParticipantImpl * participant;
   dcrudOperation *        op;
   CHK(__FILE__,__LINE__,(NULL == context    ) ? UTIL_STATUS_NULL_ARGUMENT : UTIL_STATUS_NO_ERROR)
   opCall      = (dcrudOperationCall *   )context->value;
   participant = (dcrudIParticipantImpl *)context->user;
   CHK(__FILE__,__LINE__,(NULL == opCall     ) ? UTIL_STATUS_NULL_ARGUMENT : UTIL_STATUS_NO_ERROR)
   CHK(__FILE__,__LINE__,(NULL == participant) ? UTIL_STATUS_NULL_ARGUMENT : UTIL_STATUS_NO_ERROR)
   op = opCall->operation;
   if( op == NULL ) {
      CHK(__FILE__,__LINE__,(NULL == opCall->opName) ? UTIL_STATUS_ILLEGAL_STATE : UTIL_STATUS_NO_ERROR)
      if( 0 == strcmp( opCall->opName, ICRUD_INTERFACE_CREATE )) {
         dcrudClassID classID = NULL;
         CHK(__FILE__,__LINE__,dcrudArguments_getClassID( opCall->arguments, ICRUD_INTERFACE_CLASSID, &classID ))
         CHK(__FILE__,__LINE__,dcrudIParticipantImpl_createData( participant, classID, opCall->arguments ))
      }
      else if( 0 == strcmp( opCall->opName, ICRUD_INTERFACE_UPDATE )) {
         dcrudGUID id = NULL;
         CHK(__FILE__,__LINE__,dcrudArguments_getGUID( opCall->arguments, ICRUD_INTERFACE_GUID, &id ))
         CHK(__FILE__,__LINE__,dcrudIParticipantImpl_updateData( participant, id, opCall->arguments ))
      }
      else if( 0 == strcmp( opCall->opName, ICRUD_INTERFACE_DELETE )) {
         dcrudGUID id = NULL;
         CHK(__FILE__,__LINE__,dcrudArguments_getGUID( opCall->arguments, ICRUD_INTERFACE_GUID, &id ))
         CHK(__FILE__,__LINE__,dcrudIParticipantImpl_deleteData( participant, id ))
      }
      else {
         fprintf( stderr, "%s:%d: Unexpected Publisher operation '"ICRUD_INTERFACE_NAME".%s'\n",
            __FILE__, __LINE__, opCall->opName );
      }
   }
   else {
      dcrudArguments results = op->function( op->context, opCall->arguments );
      if( opCall->callId ) {
         status = dcrudIParticipantImpl_sendCall(
            participant, opCall->intrfcName, opCall->opName, results, -opCall->callId );
      }
      if( results ) {
         dcrudArguments_delete( &results );
      }
   }
   dcrudOperationCall_delete( &opCall );
   return status;
}

utilStatus dcrudIDispatcher_handleRequests( dcrudIDispatcher self ) {
   utilStatus             status = UTIL_STATUS_NO_ERROR;
   dcrudIDispatcherImpl * This   = dcrudIDispatcher_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      unsigned queueNdx;
      status = osMutex_take( This->operationQueuesMutex );
      for( queueNdx = 0;
           dcrudNetwork_isAlive()
           &&( status == UTIL_STATUS_NO_ERROR   )
           &&( queueNdx < OPERATION_QUEUE_COUNT );
           ++queueNdx )
      {
         status = collList_foreach(
            This->operationQueues[queueNdx],
            runAllPendingOperations,
            This->participant,
            NULL );
         collList_clear( This->operationQueues[queueNdx] );
      }
      osMutex_release( This->operationQueuesMutex );
   }
   if( ! dcrudNetwork_isAlive()) {
      dcrudIParticipantImpl * ptr = This->participant;
      dcrudIParticipantImpl_delete( &ptr );
   }
   return status;
}
