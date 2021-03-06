package tests;

import java.nio.ByteBuffer;
import java.time.LocalDate;

import javafx.scene.paint.Color;

public class SerializerHelper {

   public static void putLocalDate( LocalDate date, ByteBuffer target ) {
      target.putShort((short)date.getYear());
      target.put     ((byte )date.getMonthValue());
      target.put     ((byte )date.getDayOfMonth());
   }

   public static LocalDate getLocalDate( ByteBuffer source ) {
      final short year       = source.getShort();
      final byte  month      = source.get();
      final byte  dayOfMonth = source.get();
      return LocalDate.of( year, month, dayOfMonth );
   }

   public static void putFxColor( Color color, ByteBuffer target, Color def ) {
      if( color == null ) {
         color = def;
      }
      target.putDouble( color.getRed());
      target.putDouble( color.getGreen());
      target.putDouble( color.getBlue());
      target.putDouble( color.getOpacity());
   }

   public static Color getFxColor( ByteBuffer source ) {
      final double red     = source.getDouble();
      final double green   = source.getDouble();
      final double blue    = source.getDouble();
      final double opacity = source.getDouble();
      return Color.color( red, green, blue, opacity );
   }
}
