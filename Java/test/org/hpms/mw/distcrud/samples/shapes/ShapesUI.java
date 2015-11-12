package org.hpms.mw.distcrud.samples.shapes;

import java.io.IOException;
import java.util.Map;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;
import java.util.function.BiFunction;

import org.hpms.function.QuadFunction;
import org.hpms.fx.App;
import org.hpms.fx.Controller;
import org.hpms.mw.distcrud.GUID;
import org.hpms.mw.distcrud.IRepository;
import org.hpms.mw.distcrud.IRepositoryFactory;
import org.hpms.mw.distcrud.RepositoryFactoryBuilder;
import org.hpms.mw.distcrud.Shareable;
import org.hpms.mw.distcrud.samples.Settings;

import javafx.application.Application.Parameters;
import javafx.application.Platform;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.geometry.Rectangle2D;
import javafx.scene.Node;
import javafx.scene.control.CheckMenuItem;
import javafx.scene.control.ColorPicker;
import javafx.scene.control.Menu;
import javafx.scene.control.TableView;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;
import javafx.scene.shape.Ellipse;
import javafx.scene.shape.Rectangle;
import javafx.scene.shape.Shape;
import javafx.stage.Screen;
import javafx.stage.Stage;

public class ShapesUI implements Controller, Settings {

   private final Random _random = new Random();

   private boolean                     _isPublisher;
   private String                      _title;
   private IRepository<ShareableShape> _shapes;
   private int                         _rank;
   private Node                        _dragged;
   private double                      _nodeX;
   private double                      _nodeY;
   private double                      _fromX;
   private double                      _fromY;
   private boolean                     _periodic;
   private boolean                     _moveThem;

   @FXML private Menu               _publisherMnu;
   @FXML private TableView<FxShape> _shapesTbl;
   @FXML private Pane               _shapesArea;
   @FXML private ColorPicker        _strokeColor;
   @FXML private ColorPicker        _fillColor;

   @Override
   public double getAppX() {
      return 10_000.00;
   }

   @Override
   public double getAppY() {
      return 10_000.00;
   }

   @Override
   public String getAppTitle() {
      return _title;
   }

   private Shareable shapeFactory( GUID id ) {
      assert id._topic.equals( ShareableShape.TOPIC ) : "unexpected topic '" + id._topic + "'";
      switch( id._class ) {
      case ShareableShape.RECTANGLE_CLASS: return new ShareableRect( id );
      case ShareableShape.ELLIPSE_CLASS  : return new ShareableEllipse( id );
      }
      throw new IllegalStateException( "Unexpected id: " + id );
   }

   private String getArg( Parameters args, String key, String deflt ) {
      final String value = args.getNamed().get( key );
      return ( value == null ) ? deflt : value;
   }

   @Override
   public void initialize( Object context, Parameters args ) throws Exception {
      _isPublisher =
         args.getNamed().containsKey( "publisher" ) &&
         Boolean.parseBoolean( args.getNamed().get( "publisher" ));
      if( _isPublisher ) {
         _title = "Créateur de formes";
      }
      else {
         _title = "Afficheur de formes";
         _publisherMnu.setDisable( true );
      }
      final String address = getArg( args, "address", MC_GROUP );
      final String port    = getArg( args, "port"   , "" + MC_PORT );
      final String intrfc  = getArg( args, "intrfc" , MC_INTRFC );
      final String srcId   = args.getNamed().get( "src-id" );
      if( srcId == null ) {
         throw new IllegalArgumentException( "src-id argument is mandatory" );
      }
      final IRepositoryFactory repositories =
         RepositoryFactoryBuilder.join(
            address,
            intrfc,
            Integer.parseInt( port  ),
            Integer.parseInt( srcId ) );
      _shapes = repositories.createRepository( "shapes", this::shapeFactory );
      _strokeColor.setValue( Color.BLACK );
      _fillColor  .setValue( Color.LIGHTSALMON );
      new Timer( "Ticker", true ).scheduleAtFixedRate(
         new TimerTask() { @Override public void run() { tick(); }}, 0, 40L );
   }

   @Override
   public void onShown( Stage stage ) {
      Platform.runLater(() -> {
         if( _isPublisher ) {
            stage.setX( -4 );
            stage.setY( -4 );
         }
         else {
            final Rectangle2D screen = Screen.getPrimary().getBounds();
            stage.setX( ( screen.getWidth () - stage.getWidth () ) + 4 );
            stage.setY( ( screen.getHeight() - stage.getHeight() ) + 4 );
         }
      });
   }

   private double nextDouble( double max, double min ) {
      return min + ( _random.nextDouble()*(max-min) );
   }

   @FXML
   private void periodic( ActionEvent event ) {
      final CheckMenuItem periodic = (CheckMenuItem)event.getSource();
      _periodic = periodic.isSelected();
   }

   @FXML
   private void moveThem( ActionEvent event ) {
      final CheckMenuItem moveThem = (CheckMenuItem)event.getSource();
      _moveThem = moveThem.isSelected();
   }

   private <S extends Shape, SS extends ShareableShape>
   void createShape(
      String                                      name,
      int                                         classId,
      QuadFunction<Double,Double,Double,Double,S> sf,
      BiFunction<String, S, SS>                   ssf )
   {
      final double x = nextDouble( 540,  0 );
      final double y = nextDouble( 400,  0 );
      final double w = nextDouble( 100, 40 );
      final double h = nextDouble(  80, 20 );
      final S  shape  = sf.apply( x, y, w, h );
      final SS sshape = ssf.apply( String.format( name + " %03d", ++_rank ), shape );
      shape.setStroke( _strokeColor.getValue());
      shape.setFill( _fillColor.getValue());
      _shapes.create( sshape, classId );
      _shapesArea.getChildren().add( shape );
   }

   @FXML
   private void createRectangle() {
      createShape(
         "Rectangle", ShareableShape.RECTANGLE_CLASS,
         ( x, y, w, h ) -> new Rectangle( x, y, w, h ),
         ( name, shape ) -> new ShareableRect( name, shape ));
   }

   @FXML
   private void createEllipse() {
      createShape(
         "Ellipse", ShareableShape.ELLIPSE_CLASS,
         ( x, y, w, h ) -> new Ellipse( x, y, w, h ),
         ( name, shape ) -> new ShareableEllipse( name, shape ));
   }

   @FXML
   private void publish() throws IOException {
      if( _isPublisher ) {
         _shapes.publish();
      }
   }

   @FXML
   private void refresh() {
      if( ! _isPublisher ) {
         _shapes.refresh();
         Platform.runLater(() -> {
            _shapesArea.getChildren().clear();
            _shapesTbl.getItems().clear();
            final Map<GUID, ShareableShape> shapes = _shapes.select( s -> true );
            for( final ShareableShape shape : shapes.values()) {
               _shapesArea.getChildren().add( shape.getShape());
               _shapesTbl.getItems().add( new FxShape( shape ));
            }
         });
      }
   }

   private void moveThem() {
      if( _isPublisher ) {
         Platform.runLater(() -> {
            final Map<GUID, ShareableShape> shapes = _shapes.select( s -> true );
            for( final ShareableShape shape : shapes.values()) {
               shape.moveIt( _shapesArea.getLayoutBounds());
               _shapes.update( shape );
            }
         });
      }
   }

   void tick() {
      try {
         if( _moveThem ) {
            moveThem();
         }
         if( _periodic ) {
            publish();
            refresh();
         }
      }
      catch( final IOException x ) {
         x.printStackTrace();
      }
   }

   @FXML
   private void drag( MouseEvent event ) {
      if( _isPublisher ) {
         if( _dragged == null ) {
            if( event.getTarget() instanceof javafx.scene.shape.Shape ) {
               _dragged = (Node)event.getTarget();
               _nodeX   = _dragged.getLayoutX();
               _nodeY   = _dragged.getLayoutY();
               _fromX   = event.getX();
               _fromY   = event.getY();
            }
         }
         else {
            final double dx = event.getX() - _fromX;
            final double dy = event.getY() - _fromY;
            _dragged.setLayoutX( _nodeX + dx );
            _dragged.setLayoutY( _nodeY + dy );
         }
         event.consume();
      }
   }

   @FXML
   private void drop( MouseEvent event ) {
      if( _isPublisher && ( _dragged != null ) ) {
         try {
            final double dx = event.getX() - _fromX;
            final double dy = event.getY() - _fromY;
            _dragged.setLayoutX( _nodeX + dx );
            _dragged.setLayoutY( _nodeY + dy );
            final ShareableShape shape = (ShareableShape)_dragged.getUserData();
            _shapes.update( shape );
         }
         catch( final Throwable t ) {
            t.printStackTrace();
         }
         _dragged = null;
         event.consume();
      }
   }

   @Override
   public void onCloseMainStage( Stage stage ) {
      System.exit( 0 );
   }

   @FXML
   private void onQuit() {
      onCloseMainStage((Stage)_shapesTbl.getScene().getWindow());
   }

   public static void main( String[] args ) {
      App.launch( args, ShapesUI.class );
   }
}
