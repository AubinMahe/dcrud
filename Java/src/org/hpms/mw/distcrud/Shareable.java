package org.hpms.mw.distcrud;

import java.nio.ByteBuffer;

public abstract class Shareable {

   private final GUID _id;

   protected Shareable( GUID id ) {
      _id = id;
   }

   public final GUID getId() {
      return _id;
   }

   public abstract void set( Shareable source );

   public abstract void serialize( ByteBuffer target );

   public abstract void unserialize( ByteBuffer source );
}
