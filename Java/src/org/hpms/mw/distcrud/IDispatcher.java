package org.hpms.mw.distcrud;

import java.io.IOException;

public interface IDispatcher {

   IProvided provide( String name );

   IRequired require( String name );

   ICRUD requireCRUD( ClassID classId );

   void handleRequests() throws IOException;
}
