package tests.shapes;

import org.hpms.mw.distcrud.GUID;

import javafx.beans.property.DoubleProperty;
import javafx.beans.property.ObjectProperty;
import javafx.beans.property.SimpleDoubleProperty;
import javafx.beans.property.SimpleObjectProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;
import javafx.fxml.FXML;
import javafx.scene.shape.Ellipse;
import javafx.scene.shape.Rectangle;

public class FxShape {

   private final ObjectProperty<GUID> _id   = new SimpleObjectProperty<>();
   private final StringProperty       _name = new SimpleStringProperty();
   private final StringProperty       _kind = new SimpleStringProperty();
   private final DoubleProperty       _x    = new SimpleDoubleProperty();
   private final DoubleProperty       _y    = new SimpleDoubleProperty();

   FxShape( ShareableShape shape ) {
      setShape( shape );
   }

   void setShape( ShareableShape shape ) {
      _id  .set( shape._id );
      _name.set( shape._name );
      _kind.set( shape.getClass().getSimpleName());
      if( shape._shape instanceof Rectangle ) {
         final Rectangle r = (Rectangle)shape._shape;
         _x.set( r.getX());
         _y.set( r.getY());
      }
      else if( shape._shape instanceof Ellipse ) {
         final Ellipse r = (Ellipse)shape._shape;
         _x.set( r.getCenterX());
         _y.set( r.getCenterY());
      }
   }

   @FXML
   public ObjectProperty<GUID> idProperty() {
      return _id;
   }

   @FXML
   public StringProperty nameProperty() {
      return _name;
   }

   @FXML
   public StringProperty kindProperty() {
      return _kind;
   }

   @FXML
   public DoubleProperty xProperty() {
      return _x;
   }

   @FXML
   public DoubleProperty yProperty() {
      return _y;
   }
}
