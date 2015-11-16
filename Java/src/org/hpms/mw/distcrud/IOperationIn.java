package org.hpms.mw.distcrud;

import java.util.Map;

public interface IOperationIn {

   boolean execute( Map<String, Shareable> in );
}
