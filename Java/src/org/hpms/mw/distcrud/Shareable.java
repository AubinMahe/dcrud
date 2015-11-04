package org.hpms.mw.distcrud;

import java.nio.ByteBuffer;

public abstract class Shareable {

   private final int  _classId;
   private /* */ GUID _id;

   protected Shareable( int classId ) {
      _classId = classId;
   }

   /**
    * Package used only.
    * @param id the GUID
    */
   void setId( GUID id ) {
      _id = id;
   }

   public final GUID getId() {
      return _id;
   }

   public int getClassId() {
      return _classId;
   }

   public abstract void set( Shareable source );

   public abstract void serialize( ByteBuffer target );

   public abstract void unserialize( ByteBuffer source );
}
