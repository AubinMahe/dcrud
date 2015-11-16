package org.hpms.mw.distcrud;

public interface IDispatcher {

   IProvided provide( String name );

   IRequired require( String name );
}
