package org.hpms.mw.distcrud;

import java.util.Map;

public interface IOperationInOut {

   boolean execute( Map<String, Object> in, Map<String, Object> out );
}
