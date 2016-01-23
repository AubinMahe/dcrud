package tests.registry;

import java.net.InetSocketAddress;
import java.util.Comparator;

final class InetSocketAddressComparator implements Comparator<InetSocketAddress> {

   @Override
   public int compare( InetSocketAddress left, InetSocketAddress right ) {
      return left.toString().compareTo( right.toString());
   }
}
