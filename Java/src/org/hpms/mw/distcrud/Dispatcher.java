package org.hpms.mw.distcrud;

import java.io.IOException;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;
import java.util.function.BiConsumer;

class Dispatcher implements IDispatcher {

   private final Repositories              _repositories;
   private final Map<String, ProvidedImpl> _provided   = new HashMap<>();
   private final Set<Runnable>             _urgent     = new LinkedHashSet<>();
   private final Set<Runnable>             _operations = new LinkedHashSet<>();

   private class Operation implements Runnable {

      private final IOperation _operation;

      public Operation( String intrfcName, String opName ) {
         _operation = _provided.get( intrfcName )._ops.get( opName );
      }

      @Override
      public void run() {
         _operation.execute();
      }
   }

   private class OperationIn implements Runnable {

      private final IOperationIn        _operation;
      private final Map<String, Object> _arguments;

      public OperationIn( String intrfcName, String opName, Map<String, Object> arguments ) {
         _operation = _provided.get( intrfcName )._opsIn.get( opName );
         _arguments = arguments;
      }

      @Override
      public void run() {
         _operation.execute( _arguments );
      }
   }

   private class OperationInOut implements Runnable {

      private final IOperationInOut     _operation;
      private final Map<String, Object> _arguments;
      private final Map<String, Object> _results;

      public OperationInOut(
         String              intrfcName,
         String              opName,
         Map<String, Object> arguments,
         Map<String, Object> results    )
      {
         _operation = _provided.get( intrfcName )._opsInOut.get( opName );
         _arguments = arguments;
         _results   = results;
      }

      @Override
      public void run() {
         _operation.execute( _arguments, _results );
      }
   }

   final class ProvidedImpl implements IProvided {

      private final Map<String, IOperation>      _ops      = new HashMap<>();
      private final Map<String, IOperationIn>    _opsIn    = new HashMap<>();
      private final Map<String, IOperationInOut> _opsInOut = new HashMap<>();

      @Override
      public boolean addOperation( String opName, IOperation executor ) {
         return _ops.put( opName, executor ) == null;
      }

      @Override
      public boolean addOperation( String opName, IOperationIn executor ) {
         return _opsIn.put( opName, executor ) == null;
      }

      @Override
      public boolean addOperation( String opName, IOperationInOut executor ) {
         return _opsInOut.put( opName, executor ) == null;
      }
   }

   public Dispatcher( Repositories repositories ) {
      _repositories = repositories;
   }

   @Override
   public IProvided provide( String interfaceName ) {
      final ProvidedImpl provided = new ProvidedImpl();
      _provided.put( interfaceName, provided );
      return provided;
   }

   void execute( String intrfcName, String opName, boolean activate ) {
      if( activate ) {
         _provided.get( intrfcName )._ops.get( opName ).execute();
      }
      else {
         _operations.add( new Operation( intrfcName, opName ));
      }
   }

   void execute(
      String              intrfcName,
      String              opName,
      Map<String, Object> arguments,
      boolean             activate   )
   {
      if( activate ) {
         _provided.get( intrfcName )._opsIn.get( opName ).execute( arguments );
      }
      else {
         _operations.add( new OperationIn( intrfcName, opName, arguments ));
      }
   }

   void execute(
      String              intrfcName,
      String              opName,
      Map<String, Object> arguments,
      Map<String, Object> results,
      boolean             activate )
   {
      if( activate ) {
         _provided.get( intrfcName )._opsInOut.get( opName ).execute( arguments, results );
      }
      else {
         _operations.add( new OperationInOut( intrfcName, opName, arguments, results ));
      }
   }

   @Override
   public void handleRequests() {
      synchronized( _urgent ){
         for( final Runnable operation : _urgent ) {
            operation.run();
         }
         _urgent.clear();
      }
      synchronized( _operations ) {
         for( final Runnable operation : _operations ) {
            operation.run();
         }
         _operations.clear();
      }
   }

   @Override
   public IRequired require( String name ) {
      return new IRequired() {

         @Override
         public void enqueue( String opName ) throws IOException {
            _repositories.call( name, opName );
         }

         @Override
         public void execute( String opName ) throws IOException {
            final Map<String, Object> arguments = new HashMap<>();
            arguments.put( "@activate", Boolean.TRUE );
            _repositories.call( name, opName, arguments );
         }

         @Override
         public void enqueue( String opName, Map<String, Object> arguments ) throws IOException {
            _repositories.call( name, opName, arguments );
         }

         @Override
         public void execute( String opName, Map<String, Object> arguments ) throws IOException {
            arguments.put( "@activate", Boolean.TRUE );
            _repositories.call( name, opName, arguments );
         }

         @Override
         public int enqueue(
            String                                   opName,
            Map<String, Object>                      arguments,
            BiConsumer<Integer, Map<String, Object>> callback  ) throws IOException
         {
            return _repositories.call( name, opName, arguments, callback );
         }

         @Override
         public int execute(
            String                                   opName,
            Map<String, Object>                      arguments,
            BiConsumer<Integer, Map<String, Object>> callback  ) throws IOException
         {
            arguments.put( "@activate", Boolean.TRUE );
            return _repositories.call( name, opName, arguments, callback );
         }
      };
   }
}
