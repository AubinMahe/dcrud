package org.hpms.mw.distcrud;

import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.hpms.mw.distcrud.IRequired.CallMode;

class Dispatcher implements IDispatcher, IProtocol {

   private static class Operation {

      final IOperation _operation;
      final Arguments  _arguments;
      final String     _intrfcName;
      final String     _opName;
      final int        _callId;

      public Operation(
         IOperation operation,
         Arguments  arguments,
         String     intrfcName,
         String     opName,
         int        callId )
      {
         _operation  = operation;
         _arguments  = arguments;
         _intrfcName = intrfcName;
         _opName     = opName;
         _callId     = callId;
      }
   }

   final static class ProvidedImpl implements IProvided {

      private final Map<String, IOperation> _opsInOut = new HashMap<>();

      @Override
      public boolean addOperation( String opName, IOperation executor ) {
         return _opsInOut.put( opName, executor ) == null;
      }
   }

   private final ParticipantImpl           _participant;
   private final Map<String, ProvidedImpl> _provided        = new HashMap<>();
   @SuppressWarnings("unchecked")
   private final List<Operation>[]         _operationQueues = new List[256];

   public Dispatcher( ParticipantImpl participant ) {
      _participant = participant;
      for( int i = 0; i < _operationQueues.length; ++i ) {
         _operationQueues[i] = new LinkedList<>();
      }
   }

   @Override
   public IProvided provide( String interfaceName ) {
      final ProvidedImpl provided = new ProvidedImpl();
      _provided.put( interfaceName, provided );
      return provided;
   }

   void executeCrud( String opName, Arguments args ) {
      synchronized( _operationQueues ) {
         _operationQueues[IRequired.DEFAULT_QUEUE].add(
            new Operation( null, args, IProtocol.ICRUD_INTERFACE_NAME, opName, 0 ));
      }
   }

   boolean execute(
      String    intrfcName,
      String    opName,
      int       callId,
      Arguments arguments,
      int       queueNdx,
      CallMode  callMode ) throws IOException
   {
      final ProvidedImpl provided = _provided.get( intrfcName );
      if( provided == null ) {
         System.err.printf( "Interface '%s' isn't registered yet, call ignored\n", intrfcName );
         return false;
      }
      final IOperation operation = provided._opsInOut.get( opName );
      if( callMode == CallMode.SYNCHRONOUS ) {
         final Arguments results = operation.execute( arguments );
         if( callId > 0 ) {
            _participant.call( intrfcName, opName, results, -callId );
         }
         return true;
      }
      synchronized( _operationQueues ) {
         _operationQueues[queueNdx].add(
            new Operation( operation, arguments, intrfcName, opName, callId ));
      }
      if( callMode == CallMode.ASYNCHRONOUS_IMMEDIATE ) {
         handleRequests();
      }
      return true;
   }

   @Override
   public void handleRequests() throws IOException{
      synchronized( _operationQueues ) {
         for( final List<Operation> queue : _operationQueues ) {
            for( final Operation op : queue ) {
               final IOperation iOp = op._operation;
               if( iOp == null ) {
                  ClassID classID;
                  GUID    id;
                  switch( op._opName ) {
                  case ICRUD_INTERFACE_CREATE:
                     classID = op._arguments.get( ICRUD_INTERFACE_CLASSID );
                     _participant.create( classID, op._arguments );
                     break;
                  case ICRUD_INTERFACE_UPDATE:
                     id = op._arguments.get( ICRUD_INTERFACE_GUID );
                     _participant.update( id, op._arguments );
                     break;
                  case ICRUD_INTERFACE_DELETE:
                     id = op._arguments.get( ICRUD_INTERFACE_GUID );
                     _participant.delete( id );
                     break;
                  default:
                     System.err.printf( "Unexpected Publisher operation '%s'\n", op._opName );
                     break;
                  }
               }
               else {
                  final Arguments  results = iOp.execute( op._arguments );
                  if( op._callId > 0 ) {
                     _participant.call( op._intrfcName, op._opName, results, -op._callId );
                  }
               }
            }
            queue.clear();
         }
      }
   }

   @Override
   public IRequired require( String name ) {
      return new IRequired() {
         @Override
         public void call( String opName ) throws IOException {
            _participant.call( name, opName, null, 0 );
         }
         @Override
         public void call( String opName, Arguments arguments ) throws IOException {
            _participant.call( name, opName, arguments, 0 );
         }
         @Override
         public void call( String opName, Arguments arguments, ICallback callback ) throws IOException {
            _participant.call( name, opName, arguments, callback );
         }
      };
   }

   @Override
   public ICRUD requireCRUD( ClassID classId ) {
      return new ICRUD(){
         @Override
         public void create( Arguments how ) throws IOException {
            how.put( ICRUD_INTERFACE_CLASSID, classId );
            _participant.call( ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_CREATE, how, 0 );
         }
         @Override
         public void update( Shareable what, Arguments how ) throws IOException {
            how.put( ICRUD_INTERFACE_GUID, what._id );
            _participant.call( ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_UPDATE, how, 0 );
         }
         @Override
         public void delete( Shareable what ) throws IOException {
            final Arguments how = new Arguments();
            how.put( ICRUD_INTERFACE_GUID, what._id );
            _participant.call( ICRUD_INTERFACE_NAME, ICRUD_INTERFACE_DELETE, how, 0 );
         }
      };
   }
}
