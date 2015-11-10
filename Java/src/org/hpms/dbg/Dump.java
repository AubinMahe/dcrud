package org.hpms.dbg;

import java.nio.ByteBuffer;

public class Dump {

   private static void dumpLine( String hexa, String ascii ) {
      hexa += "                                                                          ";
      hexa = hexa.substring( 0, 6+3*8+2+3*8 );
      System.err.println( hexa + " - " + ascii );
   }

   /**
    * Print a traditional dump: hexadecimal address, 8 hexadecimal bytes, a
    * separator, 8 hexadecimal bytes, a separator, 16 ASCII characters and a
    * newline.
    * <p>Sample:<pre>
    * 0000: 44 49 53 54 43 52 55 44 - 00 00 00 01 00 00 00 73  - DISTCRUD.......s
    * 0010: 00 00 00 2A 00 00 00 01 - 00 00 00 01 00 00 00 0D  - ...*............
    * 0020: 52 65 63 74 61 6E 67 6C - 65 20 30 30 31 40 76 C0  - Rectangle.001@v.
    * </pre></p>
    * @param bytes the array of bytes
    * @param from  the starting index
    * @param to    the ending index
    */
   public static void dump( byte[] bytes, int from, int to ) {
      int    addr   = 0;
      String addStr = "";
      String hexa   = "";
      String ascii  = "";
      for( int i = from; i < to; ++i ) {
         if( addr % 16 == 0 ) {
            if( addr > 0 ) {
               dumpLine( addStr + hexa, ascii );
               hexa  = "";
               ascii = "";
            }
            addStr = String.format( "%04X:", addr );
         }
         else if( addr % 8 == 0 ) {
            hexa = hexa + " -";
         }
         ++addr;
         final byte c = bytes[i];
         hexa = hexa + String.format( " %02X", c );
         ascii += ( c > 32 && c < 127 ) ? (char)c : '.';
      }
      if( ! hexa.isEmpty()) {
         dumpLine( addStr + hexa, ascii );
      }
   }

   /**
    * Dump an array of bytes, from 0 to length.
    * @param bytes the array of bytes.
    * @see {@link Dump#dump(byte[], int, int)}
    */
   public static void dump( byte[] bytes ) {
      dump( bytes, 0, bytes.length );
   }

   /**
    * Dump the remaining of a frame, from {@link java.nio.ByteBuffer#position()} to
    * {@link java.nio.ByteBufferByteBuffer#limit()}.
    * <p>Note: {@link java.nio.ByteBuffer#position()},
    * {@link java.nio.ByteBuffer#limit()} and
    * {@link java.nio.ByteBuffer#mark()} are not affected by the dump.</p>
    * @param frame the buffer to dump
    * @see {@link Dump#dump(byte[], int, int)}
    */
   public static void dump( ByteBuffer frame ) {
      dump( frame.array(), frame.position(), frame.limit());
   }
}
