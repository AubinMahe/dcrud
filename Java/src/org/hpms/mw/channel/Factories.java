package org.hpms.mw.channel;

import java.util.HashMap;
import java.util.Map;

public class Factories {

   private static final Map<Integer,
      IFactory<? extends ISerializable<?>>> _factories = new HashMap<>();

   public static <T extends ISerializable<T>>
   void add( IFactory<T> factory ) {
      final int classID = factory.getClassID();
      if( classID == 0 ) {
         throw new IllegalStateException( "Class ID must be non nul" );
      }
      _factories.put( classID, factory );
   }

   public static <T extends ISerializable<T>>
   IFactory<T> get( int classID ) {
      @SuppressWarnings("unchecked")
      final IFactory<T> factory = (IFactory<T>)_factories.get( classID );
      return factory;
   }
}
