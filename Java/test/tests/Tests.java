package tests;

import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

public final class Tests {

   public static boolean checkNetworkInterface( NetworkInterface via ) throws SocketException {
      if( via == null ) {
         for( final Enumeration<NetworkInterface> e = NetworkInterface.getNetworkInterfaces();
            e.hasMoreElements(); )
         {
            final NetworkInterface ni = e.nextElement();
            if( ni.isUp()) {
               System.err.println( ni + " ==> virtual: " + ni.isVirtual());
            }
         }
         return false;
      }
      if( ! via.isUp()) {
         System.err.println( via + " must be up!" );
         return false;
      }
      return true;
   }
}
