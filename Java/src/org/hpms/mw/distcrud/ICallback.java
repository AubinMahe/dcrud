package org.hpms.mw.distcrud;

import java.util.Map;

public interface ICallback {

   void callback( String intrfc, String operation, Map<String, Object> results );
}
