package org.hpms.mw.distcrud.samples.shapes;

import java.io.IOException;
import java.util.Map;
import java.util.Random;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.function.BiFunction;

import org.hpms.mw.distcrud.IRepository;
import org.hpms.mw.distcrud.IRepositoryFactory;
import org.hpms.mw.distcrud.Performance;
import org.hpms.mw.distcrud.RepositoryFactoryBuilder;
import org.hpms.mw.distcrud.Shareable;
import org.hpms.mw.distcrud.samples.App;
import org.hpms.mw.distcrud.samples.Controller;
import org.hpms.mw.distcrud.samples.QuadFunction;
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
import javafx.scene.image.Image;
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

   private IRepository _cache;
   private int         _rank;
   private Node        _dragged;
   private double      _nodeX;
   private double      _nodeY;
   private double      _fromX;
   private double      _fromY;
   private boolean     _periodic;
   private boolean     _moveThem;
   private String      _window;

   @FXML private Menu               _publisherMnu;
   @FXML private CheckMenuItem      _periodicChkMnu;
   @FXML private CheckMenuItem      _moveChkMnu;
   @FXML private TableView<FxShape> _shapesTbl;
   @FXML private Pane               _shapesArea;
   @FXML private ColorPicker        _strokeColor;
   @FXML private ColorPicker        _fillColor;

   private static byte setArg( Map<String, String> args, String key, byte min, byte max ) {
      final String value = args.get( key );
      if( value != null ) {
         return Byte.parseByte( value );
      }
      throw new IllegalArgumentException(
         "--" + key + "=<value> is mandatory, value in [" + min + ".." + max + "]" );
   }

   @SuppressWarnings("static-method")
   @FXML
   private void onQuit() {
      try {
         Performance.display();
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
      System.exit( 0 );
   }

   private void onShown( Stage stage ) {
      final Rectangle2D screen = Screen.getPrimary().getBounds();
      final double left = ( screen.getWidth () - stage.getWidth ()) + 4;
      final double top  = ( screen.getHeight() - stage.getHeight()) + 4;
      Platform.runLater(() -> {
         switch( _window ) {
         default:
         case "left-top":
            stage.setX( -4 );
            stage.setY( -4 );
            break;
         case "right-top":
            stage.setX( left );
            stage.setY( -4 );
            break;
         case "right-bottom":
            stage.setX( left );
            stage.setY( top );
            break;
         case "left-bottom":
            stage.setX( -4 );
            stage.setY( top );
            break;
         }
      });
   }

   @Override
   public void initialize( Stage stage, Parameters args ) throws Exception {
      stage.setTitle( "Editeur de formes" );
      stage.setOnShown       ( e -> onShown( stage ));
      stage.setOnCloseRequest( e -> onQuit());
      stage.getIcons().add( new Image( getClass().getResource( "local.png" ).toExternalForm()));

      final Map<String, String> n = args.getNamed();
      final String  address    =                   n.getOrDefault( "address"    , MC_GROUP );
      final short   port       = Short.parseShort( n.getOrDefault( "port"       , ""+MC_PORT ));
      final String  intrfc     =                   n.getOrDefault( "intrfc"     , MC_INTRFC );
      final byte    platformId = setArg( n, "platform-id", (byte)0, (byte)255 );
      final byte    execId     = setArg( n, "exec-id"    , (byte)0, (byte)255 );
      final boolean ownership  = Boolean.parseBoolean( n.getOrDefault( "ownership", "false" ));
      final boolean periodic   = Boolean.parseBoolean( n.getOrDefault( "periodic" , "false" ));
      final boolean move       = Boolean.parseBoolean( n.getOrDefault( "move"     , "false" ));
      final boolean perf       = Boolean.parseBoolean( n.getOrDefault( "perf"     , "false" ));
      _window                  = n.getOrDefault( "window", "left-top" );
      final String  fill       = n.get( "fill" );
      if( fill != null ) {
         _fillColor  .setValue( Color.web( fill ));
      }
      else {
         _fillColor  .setValue( Color.LIGHTSALMON );
      }
      final String  stroke     = n.get( "stroke" );
      if( stroke != null ) {
         _strokeColor.setValue( Color.web( stroke ));
      }
      else {
         _strokeColor.setValue( Color.BLACK );
      }
      Performance.enable( perf );

      final IRepositoryFactory repositories =
         RepositoryFactoryBuilder.join( address, intrfc, port, platformId, execId );
      repositories.registerClass( ShareableEllipse.CLASS_ID, ShareableEllipse::new );
      repositories.registerClass( ShareableRect   .CLASS_ID, ShareableRect   ::new );

      _cache = repositories.createRepository();
      _cache.ownership( ownership );
      _cache.subscribe( ShareableEllipse.CLASS_ID );
      _cache.subscribe( ShareableRect   .CLASS_ID );

      new Timer( "UI-Periodic-Activity", true ).schedule(
         new TimerTask() { @Override public void run() { uiActivity(); }}, 0, 40L );

      new Timer( "Network-Periodic-Activity", true ).schedule(
         new TimerTask() { @Override public void run() { networkActivity(); }}, 0, 200L );

      if( periodic ) {
         _periodicChkMnu.setSelected( periodic );
         _periodic = periodic;
      }
      if( move ) {
         _moveChkMnu.setSelected( move );
         _moveThem = move;
      }
      final String create = n.getOrDefault( "create", null );
      if( create != null ) {
         final String[] shapes = create.split( "," );
         for( final String shape : shapes ) {
            switch( shape ) {
            default: throw new IllegalArgumentException( create );
            case "Ellipse"  : createEllipse();   break;
            case "Rectangle": createRectangle(); break;
            }
         }
      }
      final Thread repositoriesThread = new Thread( repositories::run );
      repositoriesThread.setName( "network" );
      repositoriesThread.setDaemon( true );
      repositoriesThread.start();
   }

   private double nextDouble( double max, double min ) {
      return min + ( _random.nextDouble()*(max-min) );
   }

   private <S extends Shape, SS extends ShareableShape>
   void createShape(
      String                                      name,
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
      _cache.create( sshape );
      _shapesArea.getChildren().add( shape );
   }

   @FXML
   private void createRectangle() {
      createShape(
         "Rectangle",
         ( x, y, w, h ) -> new Rectangle( x, y, w, h ),
         ( name, shape ) -> new ShareableRect( name, shape ));
   }

   @FXML
   private void createEllipse() {
      createShape(
         "Ellipse",
         ( x, y, w, h ) -> new Ellipse( x, y, w, h ),
         ( name, shape ) -> new ShareableEllipse( name, shape ));
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

   @FXML
   private void publish() throws IOException {
      _cache.publish();
   }

   private void refreshUI() {
      _shapesArea.getChildren().clear();
      final Set<Shareable> shareables = _cache.select( s -> true );
      for( final Shareable shareable : shareables ) {
         final ShareableShape shape = (ShareableShape)shareable;
         assert shape != null;
         assert shape.getShape() != null;
         _shapesArea.getChildren().add( shape.getShape());
         search: {
            for( final FxShape fxs : _shapesTbl.getItems()) {
               if( fxs.idProperty().get().equals( shape._id )) {
                  fxs.setShape( shape );
                  break search;
               }
            }
            _shapesTbl.getItems().add( new FxShape( shape ));
         }
      }
   }

   @FXML
   private void refresh() {
      _cache.refresh();
      Platform.runLater( this::refreshUI );
   }

   void networkActivity() {
      try {
         if( _periodic ) {
            _cache.publish();
            refresh();
         }
      }
      catch( final IOException x ) {
         x.printStackTrace();
      }
   }

   private void moveUI() {
      final Set<Shareable> shareables = _cache.select( s -> true );
      for( final Shareable shareable : shareables ) {
         final ShareableShape shape = (ShareableShape)shareable;
         if( _cache.matches( shape._id )) {
            shape.moveIt( _shapesArea.getLayoutBounds());
            _cache.update( shape );
         }
      }
   }

   void uiActivity() {
      if( _moveThem ) {
         Platform.runLater( this::moveUI );
      }
   }

   @FXML
   private void drag( MouseEvent event ) {
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

   @FXML
   private void drop( MouseEvent event ) {
      try {
         final double dx = event.getX() - _fromX;
         final double dy = event.getY() - _fromY;
         _dragged.setLayoutX( _nodeX + dx );
         _dragged.setLayoutY( _nodeY + dy );
         final ShareableShape shape = (ShareableShape)_dragged.getUserData();
         _cache.update( shape );
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
      _dragged = null;
      event.consume();
   }

   public static void main( String[] args ) {
      App.launch( args, ShapesUI.class );
   }
}
