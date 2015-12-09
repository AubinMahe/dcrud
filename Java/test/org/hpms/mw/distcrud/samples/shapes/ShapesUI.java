package org.hpms.mw.distcrud.samples.shapes;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.function.BiFunction;

import javax.xml.parsers.ParserConfigurationException;

import org.hpms.dbg.Performance;
import org.hpms.mw.distcrud.ClassID;
import org.hpms.mw.distcrud.ICache;
import org.hpms.mw.distcrud.IDispatcher;
import org.hpms.mw.distcrud.IParticipant;
import org.hpms.mw.distcrud.IRequired;
import org.hpms.mw.distcrud.Network;
import org.hpms.mw.distcrud.Shareable;
import org.hpms.mw.distcrud.samples.App;
import org.hpms.mw.distcrud.samples.Controller;
import org.hpms.mw.distcrud.samples.QuadFunction;
import org.xml.sax.SAXException;

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

public class ShapesUI implements Controller {

   private final Random _random = new Random();

   private ICache    _cache;
   private int       _rank;
   private Node      _dragged;
   private double    _nodeX;
   private double    _nodeY;
   private double    _fromX;
   private double    _fromY;
   private boolean   _periodic;
   private boolean   _moveThem;
   private boolean   _readonly;
   private IRequired _remoteShapesFactory;
   private String    _window;

   @FXML private Menu               _publisherMnu;
   @FXML private CheckMenuItem      _periodicChkMnu;
   @FXML private CheckMenuItem      _moveChkMnu;
   @FXML private TableView<FxShape> _shapesTbl;
   @FXML private Pane               _shapesArea;
   @FXML private ColorPicker        _strokeColor;
   @FXML private ColorPicker        _fillColor;

   @SuppressWarnings("static-method")
   @FXML
   private void onQuit() {
      try {
         Performance.saveToDisk();
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

   private void dcrudInitialize( Map<String, String> n ) throws IOException, SAXException, ParserConfigurationException {
      _window   =                                     n.getOrDefault( "window"   , "left-top" );
      _readonly =               Boolean.parseBoolean( n.getOrDefault( "readonly" , "false" ));
      final boolean ownership = Boolean.parseBoolean( n.getOrDefault( "ownership", "false" ));
      final boolean periodic  = Boolean.parseBoolean( n.getOrDefault( "periodic" , "false" ));
      final boolean move      = Boolean.parseBoolean( n.getOrDefault( "move"     , "false" ));
      final boolean perf      = Boolean.parseBoolean( n.getOrDefault( "perf"     , "false" ));
      final boolean remote    = Boolean.parseBoolean( n.getOrDefault( "remote"   , "false" ));
      final String  name      =                       n.get         ( "publisher-name" );
      final String  intrfc    =                       n.get         ( "interface" );
      if( name == null ) {
         throw new IllegalStateException( "--publisher-name=<string> missing" );
      }
      if( intrfc == null ) {
         throw new IllegalStateException( "--interface=<string> missing" );
      }
      Performance.enable( perf );
      final IParticipant participant = Network.join( new File( "network.xml"), intrfc, name );
      participant.registerClass( ShareableEllipse.CLASS_ID, ShareableEllipse::new );
      participant.registerClass( ShareableRect   .CLASS_ID, ShareableRect   ::new );
      _cache = participant.createCache();
      _cache.setOwnership( ownership );
      _cache.subscribe( ShareableEllipse.CLASS_ID );
      _cache.subscribe( ShareableRect   .CLASS_ID );
      if( remote ) {
         final IDispatcher dispatcher = participant.getDispatcher();
         _remoteShapesFactory = dispatcher.require( "IShapesFactory" );
      }
      new Timer( "AnimationActivity", true ).schedule(
         new TimerTask() { @Override public void run() { animationActivity(); }}, 0, 40L );
      new Timer( "NetworkActivity", true ).schedule(
         new TimerTask() { @Override public void run() { networkActivity(); }}, 0, 40L );
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
   }

   @Override
   public void initialize( Stage stage, Parameters args ) throws Exception {
      stage.setTitle( "Editeur de formes" );
      stage.setOnShown       ( e -> onShown( stage ));
      stage.setOnCloseRequest( e -> onQuit());
      stage.getIcons().add( new Image( getClass().getResource( "local.png" ).toExternalForm()));

      final Map<String, String> n = args.getNamed();
      final String fill = n.get( "fill" );
      if( fill != null ) {
         _fillColor.setValue( Color.web( fill ));
      }
      else {
         _fillColor.setValue( Color.LIGHTSALMON );
      }
      final String stroke = n.get( "stroke" );
      if( stroke != null ) {
         _strokeColor.setValue( Color.web( stroke ));
      }
      else {
         _strokeColor.setValue( Color.BLACK );
      }
      dcrudInitialize( n );
   }

   private double nextDouble( double min, double max ) {
      return min + ( _random.nextDouble()*(max-min) );
   }

   private <S extends Shape, SS extends ShareableShape>
   void createShape(
      String                                      name,
      QuadFunction<Double,Double,Double,Double,S> sf,
      BiFunction<String, S, SS>                   ssf )
   {
      final double x = nextDouble(  0, 540 );
      final double y = nextDouble(  0, 400 );
      final double w = nextDouble( 40, 100 );
      final double h = nextDouble( 20,  80 );
      final S  shape  = sf.apply( x, y, w, h );
      final SS sshape = ssf.apply( String.format( name + " %03d", ++_rank ), shape );
      shape.setStroke( _strokeColor.getValue());
      shape.setFill( _fillColor.getValue());
      _cache.create( sshape );
      _shapesArea.getChildren().add( shape );
   }

   private void createShapeRemotely( ClassID classId ) {
      try {
         final Map<String, Object> arguments = new HashMap<>();
         arguments.put( "class", classId );
         arguments.put( "x"    , nextDouble(  0, 540 ));
         arguments.put( "y"    , nextDouble(  0, 400 ));
         arguments.put( "w"    , nextDouble( 40, 100 ));
         arguments.put( "h"    , nextDouble( 20,  80 ));
         _remoteShapesFactory.call( "create", arguments );
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
   }

   @FXML
   private void createRectangle() {
      if( _remoteShapesFactory != null ) {
         createShapeRemotely( ShareableRect.CLASS_ID );
      }
      else {
         createShape(
            "Rectangle",
            ( x, y, w, h ) -> new Rectangle( x, y, w, h ),
            ( name, shape ) -> new ShareableRect( name, shape ));
      }
   }

   @FXML
   private void createEllipse() {
      if( _remoteShapesFactory != null ) {
         createShapeRemotely( ShareableEllipse.CLASS_ID );
      }
      else {
         createShape(
            "Ellipse",
            ( x, y, w, h ) -> new Ellipse( x, y, w, h ),
            ( name, shape ) -> new ShareableEllipse( name, shape ));
      }
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
      for( final Shareable shareable : _cache.values()) {
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
            if( ! _readonly ) {
               _cache.publish();
            }
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
         if( _cache.owns( shape._id )) {
            shape.moveIt( _shapesArea.getLayoutBounds());
            _cache.update( shape );
         }
      }
   }

   void animationActivity() {
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
