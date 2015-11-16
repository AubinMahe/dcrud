package org.hpms.mw.distcrud;

import java.util.Map;

public interface IOperationInOut {

   boolean execute( Map<String, Shareable> in, Map<String, Shareable> out );
}
