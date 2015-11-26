package org.hpms.mw.distcrud;

import java.util.Map;

public interface IOperation {

   void execute( Map<String, Object> in, Map<String, Object> out );
}
