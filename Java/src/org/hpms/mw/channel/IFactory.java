package org.hpms.mw.channel;

public interface IFactory<T> {

   int getClassID();

   T    create();
   void encode( T t, CoDec encoder );
   void decode( T t, CoDec decoder );
}
