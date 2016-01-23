package org.hpms.util;

import java.io.FileNotFoundException;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

public class Performance {

   private static final Map<String, List<Long>> _records = new HashMap<>();

   private Performance(){}

   public static synchronized void record( String attribute, long elapsed ) {
      List<Long> records = _records.get( attribute );
      if( records == null ) {
         _records.put( attribute, records = new ArrayList<>( 10_000 ));
      }
      records.add( elapsed );
   }

   public static synchronized void saveToDisk() throws FileNotFoundException {
      try( PrintStream _out = new PrintStream( "perfo.txt" )) {
         _out.printf( "+------------+-----------+-----------+-----------+\n" );
         _out.printf( "| Attribute  |    Min µs |    Max µs |    Avg µs |\n" );
         _out.printf( "+------------+-----------+-----------+-----------+\n" );
         for( final Entry<String, List<Long>> e : _records.entrySet()) {
            final String     attribute = e.getKey();
            final List<Long> measures  = e.getValue();
            final double min =
               measures.stream().parallel().mapToDouble( r -> r ).min().getAsDouble() / 1_000.0;
            final double max =
               measures.stream().parallel().mapToDouble( r -> r ).max().getAsDouble() / 1_000.0;
            final double avg =
               ( measures.stream().parallel().mapToDouble( r -> r ).sum() / measures.size())/1_000.0;
            _out.printf( "| %-10s | %9.2f | %9.2f | %9.2f |\n",
               attribute, min, max, avg );
         }
         _out.printf( "+------------+-----------+-----------+-----------+\n" );
      }
      for( final Entry<String, List<Long>> e : _records.entrySet()) {
         final String attribute = e.getKey();
         try( PrintStream _out = new PrintStream( attribute + ".dat" )) {
            for( final Long measure : e.getValue()) {
               _out.printf( "%9.2f\n", measure.longValue() / 1_000.0 );
            }
         }
      }
   }
}
