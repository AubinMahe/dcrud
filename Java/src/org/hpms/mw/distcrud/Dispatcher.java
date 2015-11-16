package org.hpms.mw.distcrud;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.function.BiConsumer;

class Dispatcher implements IDispatcher {

   private final Repositories              _repositories;
   private final Map<String, ProvidedImpl> _provided = new HashMap<>();

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

   void call( String intrfcName, String opName ) {
      final ProvidedImpl impl = _provided.get( intrfcName );
      impl._ops.get( opName ).execute();
   }

   void call( String intrfcName, String opName, Map<String, Shareable> arguments ) {
      final ProvidedImpl impl = _provided.get( intrfcName );
      impl._opsIn.get( opName ).execute( arguments );
   }

   void call(
      String                 intrfcName,
      String                 opName,
      Map<String, Shareable> arguments,
      Map<String, Shareable> results    )
   {
      final ProvidedImpl impl = _provided.get( intrfcName );
      impl._opsInOut.get( opName ).execute( arguments, results );
   }

   @Override
   public IRequired require( String name ) {
      return new IRequired() {

         @Override
         public void call( String opName ) throws IOException {
            _repositories.call( name, opName );
         }

         @Override
         public void call( String opName, Map<String, Shareable> arguments ) throws IOException {
            _repositories.call( name, opName, arguments );
         }

         @Override
         public int call(
            String                                   opName,
            Map<String, Shareable>                   in,
            BiConsumer<Integer, Map<String, Shareable>> callback ) throws IOException
         {
            return _repositories.call( name, opName, in, callback );
         }
      };
   }
}
