package org.hpms.function;

@FunctionalInterface
public interface QuadFunction<T1, T2, T3, T4, T> {

   public T apply( T1 x, T2 y, T3 w, T4 h );
}
