package org.hpms.mw.distcrud;

import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.hpms.mw.distcrud.IRequired.CallMode;

class Dispatcher implements IDispatcher {

   private final ParticipantImpl           _participant;
   private final Map<String, ProvidedImpl> _provided        = new HashMap<>();
   @SuppressWarnings("unchecked")
   private final List<Runnable>[]          _operationQueues = new List[256];

   private static class Operation implements Runnable {

      private final IOperation          _operation;
      private final Map<String, Object> _arguments;
      private final Map<String, Object> _results;

      public Operation(
         IOperation          operation,
         Map<String, Object> arguments,
         Map<String, Object> results    )
      {
         _operation = operation;
         _arguments = arguments;
         _results   = results;
      }

      @Override
      public void run() {
         _operation.execute( _arguments, _results );
      }
   }

   final static class ProvidedImpl implements IProvided {

      private final Map<String, IOperation> _opsInOut = new HashMap<>();

      @Override
      public boolean addOperation( String opName, IOperation executor ) {
         return _opsInOut.put( opName, executor ) == null;
      }
   }

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

   void execute(
      String              intrfcName,
      String              opName,
      Map<String, Object> arguments,
      Map<String, Object> results,
      int                 queueNdx,
      CallMode            callMode )
   {
      final ProvidedImpl provided  = _provided.get( intrfcName );
      final IOperation   operation = provided._opsInOut.get( opName );
      if( callMode == CallMode.SYNCHRONOUS ) {
         operation.execute( arguments, results );
      }
      else {
         synchronized( _operationQueues ) {
            _operationQueues[queueNdx].add(
               new Operation( operation, arguments, results ));
         }
         if( callMode == CallMode.ASYNCHRONOUS_IMMEDIATE ) {
            handleRequests();
         }
      }
   }

   @Override
   public void handleRequests() {
      synchronized( _operationQueues ) {
         for( final List<Runnable> queue : _operationQueues ) {
            for( final Runnable operation : queue ) {
               operation.run();
            }
            queue.clear();
         }
      }
   }

   @Override
   public IRequired require( String name ) {
      return new IRequired() {

         @Override
         public int call( String opName ) throws IOException {
            final Map<String, Object> arguments = new HashMap<>();
            arguments.put( "@mode" , CallMode.ASYNCHRONOUS_DEFERRED );
            arguments.put( "@queue", IRequired.DEFAULT_QUEUE );
            return _participant.call( name, opName, arguments, null );
         }

         @Override
         public int call( String opName, Map<String, Object> arguments ) throws IOException {
            if( ! arguments.containsKey( "@mode" )) {
               arguments.put( "@mode", CallMode.ASYNCHRONOUS_DEFERRED );
            }
            if( ! arguments.containsKey( "@queue" )) {
               arguments.put( "@queue", IRequired.DEFAULT_QUEUE );
            }
            return _participant.call( name, opName, arguments, null );
         }

         @Override
         public int call( String opName, Map<String, Object> arguments, ICallback callback ) throws IOException {
            if( ! arguments.containsKey( "@mode" )) {
               arguments.put( "@mode", CallMode.ASYNCHRONOUS_DEFERRED );
            }
            if( ! arguments.containsKey( "@queue" )) {
               arguments.put( "@queue", IRequired.DEFAULT_QUEUE );
            }
            return _participant.call( name, opName, arguments, callback );
         }
      };
   }
}
