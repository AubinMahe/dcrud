package org.hpms.mw.distcrud;

public interface IProvided {

   boolean addOperation( String operationName, IOperation executor );

   boolean addOperation( String operationName, IOperationIn executor );

   boolean addOperation( String operationName, IOperationInOut executor );
}
