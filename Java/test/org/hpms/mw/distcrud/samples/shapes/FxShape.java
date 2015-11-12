package org.hpms.mw.distcrud.samples.shapes;

import javafx.fxml.FXML;

public class FxShape {

   final ShareableShape _shape;

   public FxShape( ShareableShape sshape ) {
      _shape = sshape;
   }

   @FXML
   public String getId() {
      return _shape.getId().toString();
   }

   @FXML
   public String getName() {
      return _shape._name;
   }

   @FXML
   public String getKind() {
      return _shape.getClass().getSimpleName();
   }
}
