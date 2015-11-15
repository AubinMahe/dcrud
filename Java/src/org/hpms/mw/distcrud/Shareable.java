package org.hpms.mw.distcrud;

import java.nio.ByteBuffer;

public abstract class Shareable {

   public final GUID    _id = new GUID();
   public final ClassID _class;

   protected Shareable( ClassID classId ) {
      _class = classId;
   }

   public abstract void serialize( ByteBuffer target );

   public abstract void unserialize( ByteBuffer source );
}
