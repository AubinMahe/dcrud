package tests.registry;

import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeSet;

import javafx.application.Application;
import javafx.application.Platform;
import javafx.geometry.HPos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.control.TreeItem;
import javafx.scene.control.TreeView;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.Priority;
import javafx.stage.Screen;
import javafx.stage.Stage;

public class GUI extends Application implements IRegistryListener {

   private Registry         _registry;
   private final TextField        _topic      = new TextField();
   private final TextField        _intrfc     = new TextField( "setColor" );
   private final TreeView<String> _registryVw = new TreeView<>();

   private void refreshTree( Registry registry ) {
      final TreeItem<String> root = new TreeItem<>( "Registry" );
      _registryVw.setRoot( root );
      final TreeItem<String> endpoints = new TreeItem<>( "Servers" );
      for( final InetSocketAddress endpoint : registry.getEndPoints()) {
         endpoints.getChildren().add( new TreeItem<>( endpoint.toString()));
      }
      endpoints.setExpanded( true );
      final TreeItem<String> topics = new TreeItem<>( "Topics" );
      for( final Entry<InetSocketAddress, TreeSet<Topic>> e : registry.getTopics().entrySet()) {
         final TreeItem<String> address = new TreeItem<>( e.getKey().toString());
         topics.getChildren().add( address );
         for( final Topic topic : e.getValue()) {
            address.getChildren().add( new TreeItem<>( topic.toString()));
         }
         address.setExpanded( true );
      }
      topics.setExpanded( true );
      final TreeItem<String> interfaces = new TreeItem<>( "Interfaces" );
      for( final Entry<InetSocketAddress, TreeSet<String>> e : registry.getInterfaces().entrySet()) {
         final TreeItem<String> address = new TreeItem<>( e.getKey().toString());
         interfaces.getChildren().add( address );
         for( final String intrfc : e.getValue()) {
            address.getChildren().add( new TreeItem<>( intrfc ));
         }
         address.setExpanded( true );
      }
      interfaces.setExpanded( true );
      root.getChildren().add( endpoints );
      root.getChildren().add( topics );
      root.getChildren().add( interfaces );
      root.setExpanded( true );
   }

   @Override
   public void registryHasChanged( Registry registry ) {
      Platform.runLater(() -> refreshTree( registry ));
   }

   void addTopic() {
      _registry.addTopic( new Topic( Integer.parseInt( _topic.getText())));
   }

   void addIntrfc() {
      _registry.addInterface( _intrfc.getText());
   }

   private void close() {
      try {
         _registry.close();
      }
      catch( final Throwable t ) {
         t.printStackTrace();
      }
      System.exit( 0 );
   }

   @Override
   public void start( final Stage primaryStage ) throws Exception {
      final Map<String, String> args     = getParameters().getNamed();
      final NetworkInterface    intrfc   = NetworkInterface.getByName( args.get( "interface" ));
      final InetAddress         host     = InetAddress.getByName( args.get( "registry-host" ));
      final int                 port     = Integer.parseInt( args.get( "registry-port" ));
      final int                 bootPort = Integer.parseInt( args.get( "boot-port" ));
      final int                 rank     = Integer.parseInt( args.get( "rank" )) - 1;
      _registry = new Registry( intrfc, host, port, bootPort, b -> {/**/});
      _registry.addListener( this );
      final GridPane grid       = new GridPane();
      final Button   addTopic   = new Button( "Add" );
      final Button   addIntrfc  = new Button( "Add" );
      final Label    topicsLbl  = new Label( "Topic: " );
      final Label    intrfcsLbl = new Label( "Interface: " );
      final Label    regLbl     = new Label( "Registry" );
      grid.add( topicsLbl  , 0, 0 );
      grid.add( _topic     , 1, 0 );
      grid.add( addTopic   , 2, 0 );
      grid.add( intrfcsLbl , 0, 1 );
      grid.add( _intrfc    , 1, 1 );
      grid.add( addIntrfc  , 2, 1 );
      grid.add( regLbl     , 0, 2, 3, 1 );
      grid.add( _registryVw, 0, 3, 3, 1 );
      _registryVw.setMaxHeight( Double.MAX_VALUE );
      GridPane.setHalignment( topicsLbl , HPos.RIGHT  );
      GridPane.setHalignment( intrfcsLbl, HPos.RIGHT  );
      GridPane.setHalignment( regLbl    , HPos.CENTER );
      GridPane.setHgrow( _topic     , Priority.ALWAYS );
      GridPane.setHgrow( _intrfc    , Priority.ALWAYS );
      GridPane.setHgrow( _registryVw, Priority.ALWAYS );
      GridPane.setVgrow( _registryVw, Priority.ALWAYS );
      addTopic .setOnAction( e -> addTopic());
      addIntrfc.setOnAction( e -> addIntrfc());
      primaryStage.setTitle( _registry.getThisEndPoint().toString());
      primaryStage.setScene( new Scene( grid ));
      primaryStage.setOnCloseRequest( e -> close());
      primaryStage.centerOnScreen();
      primaryStage.show();
      _topic.setText( "" + port );
      registryHasChanged( _registry );
      Platform.runLater(() -> {
         final double w = primaryStage.getWidth();
         final double h = primaryStage.getHeight();
         final int    c = (int)( Screen.getPrimary().getVisualBounds().getMaxX() / w );
         final double x = (( rank % c ) * w );
         final double y = (( rank / c ) * h );
         primaryStage.setX( x );
         primaryStage.setY( y );
      });
   }

   public static void main( String[] args ) throws Throwable {
      launch( args );
   }
}
