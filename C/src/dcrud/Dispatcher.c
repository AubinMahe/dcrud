#include "Dispatcher.h"
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
   collSet                 required;
   collSet                 dcrud;
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

utilStatus dcrudOperation_new(
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

utilStatus dcrudOperation_delete( dcrudOperation ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      UTIL_RELEASE( dcrudOperation )
   }
   return status;
}

utilStatus dcrudOperationCall_new(
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
         CHK(__FILE__,__LINE__,utilString_clone( &This->intrfcName, intrfcName ))
         CHK(__FILE__,__LINE__,utilString_clone( &This->opName    , opName     ))
         This->callId     = callId;
         This->arguments  = arguments;
      }
   }
   return status;
}

utilStatus dcrudOperationCall_delete( dcrudOperationCall ** self ) {
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

utilStatus dcrudIRequiredImpl_new(
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

utilStatus dcrudIRequiredImpl_delete( dcrudIRequired * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   if( self == NULL ) {
      status = UTIL_STATUS_NULL_ARGUMENT;
   }
   else {
      UTIL_RELEASE( dcrudIRequiredImpl )
   }
   return status;
}

utilStatus dcrudIProvidedImpl_new( dcrudIProvided * self ) {
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

utilStatus dcrudIProvidedImpl_deleteOperation( collForeach * context ) {
   dcrudOperation * op = (dcrudOperation *)context->value;
   return dcrudOperation_delete( &op );
}

utilStatus dcrudIProvidedImpl_delete( dcrudIProvided * self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   dcrudIProvidedImpl * This = dcrudIProvided_safeCast( *self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      collMap_foreach( This->opsInOut, dcrudIProvidedImpl_deleteOperation, NULL );
      collMap_delete( &This->opsInOut );
      UTIL_RELEASE(dcrudIProvidedImpl);
   }
   return status;
}

utilStatus dcrudICRUDImpl_delete( dcrudICRUDImpl ** self ) {
   utilStatus status = UTIL_STATUS_NO_ERROR;
   UTIL_RELEASE(dcrudICRUDImpl);
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
      bool allocatedHere = false;
      if( NULL == arguments ) {
         allocatedHere = true;
         CHK(__FILE__,__LINE__,dcrudArguments_new( &arguments ))
      }
      CHK(__FILE__,__LINE__,dcrudIParticipantImpl_call(
         This->participant, This->name, opName, arguments, callback ))
      if( allocatedHere ) {
         CHK(__FILE__,__LINE__,dcrudArguments_delete( &arguments ))
      }
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
         CHK(__FILE__,__LINE__,collSet_new( &(This->required), (collComparator)collPointerCompare ))
         CHK(__FILE__,__LINE__,collSet_new( &(This->dcrud   ), (collComparator)collPointerCompare ))
         for( i = 0; (status == UTIL_STATUS_NO_ERROR) && (i < OPERATION_QUEUE_COUNT); ++i ) {
            CHK(__FILE__,__LINE__,collList_new(&(This->operationQueues[i])))
         }
         CHK(__FILE__,__LINE__,osMutex_new( &This->operationQueuesMutex ))
      }
   }
   return status;
}

utilStatus dcrudIDispatcher_deleteProvided( collForeach * context ) {
   dcrudIProvided self = context->value;
   return dcrudIProvidedImpl_delete( &self );
}

utilStatus dcrudIDispatcher_deleteRequired( collForeach * context ) {
   dcrudIRequired self = context->value;
   return dcrudIRequiredImpl_delete( &self );
}

utilStatus dcrudIDispatcher_deleteDcrud( collForeach * context ) {
   dcrudICRUDImpl * dcrud = context->value;
   return dcrudICRUDImpl_delete( &dcrud );
}

utilStatus dcrudIDispatcher_deleteOpCall( collForeach * context ) {
   dcrudOperationCall * opCall = context->value;
   return dcrudOperationCall_delete( &opCall );
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
         collMap_foreach( This->provided, dcrudIDispatcher_deleteProvided, NULL );
         collMap_delete( &This->provided );
         collSet_foreach( This->required, dcrudIDispatcher_deleteRequired, NULL );
         collSet_delete( &This->required );
         collSet_foreach( This->dcrud   , dcrudIDispatcher_deleteDcrud, NULL );
         collSet_delete( &This->dcrud );
         for( i = 0; i < OPERATION_QUEUE_COUNT; ++i ) {
            collList_foreach( This->operationQueues[i], dcrudIDispatcher_deleteOpCall, NULL );
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
   dcrudIDispatcherImpl * This = dcrudIDispatcher_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      dcrudIProvided provided = NULL;
      CHK(__FILE__,__LINE__,dcrudIProvidedImpl_new( &provided ))
      CHK(__FILE__,__LINE__,collMap_put( This->provided, (collMapKey)interfaceName, provided, NULL ))
      *result = (dcrudIProvided)provided;
   }
   return status;
}

utilStatus dcrudIDispatcher_require(
   dcrudIDispatcher self,
   const char *     name,
   dcrudIRequired * result )
{
   utilStatus status = UTIL_STATUS_NO_ERROR;
   dcrudIDispatcherImpl * This = dcrudIDispatcher_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      dcrudIRequired required = NULL;
      CHK(__FILE__,__LINE__,dcrudIRequiredImpl_new( &required, This->participant, name ))
      CHK(__FILE__,__LINE__,collSet_add( This->required, required ))
      *result = (dcrudIRequired)required;
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
      CHK(__FILE__,__LINE__,collSet_add( This->dcrud, crud ))
   }
   return status;
}

utilStatus dcrudICRUD_create( dcrudICRUD self, dcrudArguments how ) {
   utilStatus       status = UTIL_STATUS_NO_ERROR;
   dcrudICRUDImpl * This   = dcrudICRUD_safeCast( self, &status );
   if( UTIL_STATUS_NO_ERROR == status ) {
      CHK(__FILE__,__LINE__,dcrudArguments_putClassID( how, ICRUD_INTERFACE_CLASSID, This->classID ))
      CHK(__FILE__,__LINE__,dcrudIParticipantImpl_sendCall(
         This->participant, ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_CREATE, how, 0 ))
   }
   return status;
}

utilStatus dcrudICRUD_update( dcrudICRUD self, dcrudShareable what, dcrudArguments how ) {
   utilStatus       status = UTIL_STATUS_NO_ERROR;
   dcrudICRUDImpl * This   = dcrudICRUD_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      dcrudGUID guid;
      CHK(__FILE__,__LINE__,dcrudShareable_getGUID( what, &guid ))
      CHK(__FILE__,__LINE__,dcrudArguments_putGUID( how, ICRUD_INTERFACE_GUID, guid ))
      CHK(__FILE__,__LINE__,dcrudIParticipantImpl_sendCall(
         This->participant, ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_UPDATE, how, 0 ))
   }
   return status;
}

utilStatus dcrudICRUD_delete( dcrudICRUD self, dcrudShareable what ) {
   utilStatus       status = UTIL_STATUS_NO_ERROR;
   dcrudICRUDImpl * This   = dcrudICRUD_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      dcrudArguments how = NULL;
      dcrudGUID      guid;
      CHK(__FILE__,__LINE__,dcrudArguments_new( &how ))
      CHK(__FILE__,__LINE__,dcrudShareable_getGUID( what, &guid ))
      CHK(__FILE__,__LINE__,dcrudArguments_putGUID( how, ICRUD_INTERFACE_GUID, guid ))
      CHK(__FILE__,__LINE__,dcrudIParticipantImpl_sendCall(
         This->participant, ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_DELETE, how, 0 ))
      CHK(__FILE__,__LINE__,dcrudArguments_delete( &how ))
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
   int              callId )
{
   utilStatus             status = UTIL_STATUS_NO_ERROR;
   dcrudIDispatcherImpl * This   = dcrudIDispatcher_safeCast( self, &status );
   if( status == UTIL_STATUS_NO_ERROR ) {
      dcrudIProvidedImpl * provided = NULL;
      status = collMap_get( This->provided, (collMapKey)intrfcName, &provided );
      if( status == UTIL_STATUS_NOT_FOUND ) {
         status = UTIL_STATUS_NO_ERROR;
         dcrudArguments_delete( &args );
      }
      else if( UTIL_STATUS_NO_ERROR == status ) {
         dcrudOperation * operation = NULL;
         dcrudCallMode    callMode;
         dcrudQueueIndex  queueNdx;
         CHK(__FILE__,__LINE__,collMap_get( provided->opsInOut, (collMapKey)opName, &operation ))
         CHK(__FILE__,__LINE__,dcrudArguments_getMode ( args, &callMode ))
         CHK(__FILE__,__LINE__,dcrudArguments_getQueue( args, &queueNdx ))
         if( DCRUD_SYNCHRONOUS == callMode ) {
            dcrudArguments results = operation->function( operation->context, args );
            if( callId ) {
               dcrudIParticipantImpl_sendCall(
                  This->participant, intrfcName, opName, results, -callId );
            }
            dcrudArguments_delete( &args );
         }
         else {
            dcrudOperationCall * opCall = NULL;
            CHK(__FILE__,__LINE__,osMutex_take( This->operationQueuesMutex ))
            status = dcrudOperationCall_new( &opCall, operation, intrfcName, opName, callId, args );
            if( UTIL_STATUS_NO_ERROR != status ) {
               utilStatus_checkAndLog( status, __FILE__,__LINE__,
                  "dcrudOperationCall_new( %s.%s, %d )", intrfcName, opName, callId );
            }
            status = collList_add( This->operationQueues[queueNdx], opCall );
            if( UTIL_STATUS_NO_ERROR != status ) {
               utilStatus_checkAndLog( status, __FILE__,__LINE__,
                  "collList_add( This->operationQueues[%d], opCall )", queueNdx );
            }
            osMutex_release( This->operationQueuesMutex );
            if(( UTIL_STATUS_NO_ERROR == status )&&( callMode == DCRUD_ASYNCHRONOUS_IMMEDIATE )) {
               CHK(__FILE__,__LINE__,dcrudIDispatcher_handleRequests( self ))
            }
         }
      }
   }
   return status;
}

utilStatus dcrudIDispatcher_runAllPendingOperations( collForeach * context ) {
   utilStatus              status      = UTIL_STATUS_NO_ERROR;
   dcrudOperationCall *    opCall      = (dcrudOperationCall *   )context->value;
   dcrudIParticipantImpl * participant = (dcrudIParticipantImpl *)context->user;
   dcrudOperation *        op          = opCall ? opCall->operation : NULL;

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
         fprintf( stderr, "%s:%d: Unexpected operation '"ICRUD_INTERFACE_NAME".%s'\n",
            __FILE__, __LINE__, opCall->opName );
      }
   }
   else {
      dcrudArguments results = op->function( op->context, opCall->arguments );
      if( opCall->callId ) {
         CHK(__FILE__,__LINE__,dcrudIParticipantImpl_sendCall(
            participant, opCall->intrfcName, opCall->opName, results, -opCall->callId ))
      }
      if( results ) {
         CHK(__FILE__,__LINE__,dcrudArguments_delete( &results ))
      }
   }
   CHK(__FILE__,__LINE__,dcrudOperationCall_delete( &opCall ))
   return status;
}

utilStatus dcrudIDispatcher_handleRequests( dcrudIDispatcher self ) {
   utilStatus             status = UTIL_STATUS_NO_ERROR;
   dcrudIDispatcherImpl * This   = dcrudIDispatcher_safeCast( self, &status );

   if( UTIL_STATUS_NO_ERROR == status ) {
      unsigned q;
      bool     alive = true;

      CHK(__FILE__,__LINE__,osMutex_take( This->operationQueuesMutex ))
      for( q = 0, alive = true;
           alive &&( UTIL_STATUS_NO_ERROR == status )&&( q < OPERATION_QUEUE_COUNT );
           ++q )
      {
         if( alive && ( UTIL_STATUS_NO_ERROR == status )) {
            status = collList_foreach(
               This->operationQueues[q],
               dcrudIDispatcher_runAllPendingOperations,
               This->participant );
            collList_clear( This->operationQueues[q] );
         }
         status = dcrudIParticipant_isAlive((dcrudIParticipant)This->participant, &alive );
      }
      CHK(__FILE__,__LINE__,osMutex_release( This->operationQueuesMutex ))
   }
   return status;
}
