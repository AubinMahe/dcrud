package tests.channel;

import java.io.IOException;
import java.time.LocalDate;
import java.util.Map;

import org.hpms.mw.channel.CoDec;
import org.hpms.mw.channel.Factories;
import org.hpms.mw.channel.UDPChannel;

import channel.Family;
import channel.Person;
import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.TextArea;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;
import javafx.scene.text.Font;
import javafx.stage.Stage;

public class Test_004 extends Application {

   private static final short HELLO_MSG_ID            = 10;
   private static final short FAMILY_REFERENCE_MSG_ID = 20;

   private CoDec       _encoder;
   private Logger      _logger;
   private UDPChannel _UDPEndPoint;

   private void init( String intrfc, int local, String targetHost, int remote ) throws IOException {
      _encoder     = new CoDec();
      _UDPEndPoint = new UDPChannel( intrfc, local, targetHost, remote );
      _UDPEndPoint.addLogger( _logger );
      _UDPEndPoint.addHandler( HELLO_MSG_ID           , this::parseHello );
      _UDPEndPoint.addHandler( FAMILY_REFERENCE_MSG_ID, this::parseFamily );
      Factories.add( new Person.Factory());
      Factories.add( new Family.Factory());
   }

   private void sendHello() {
      try {
         _encoder.init( HELLO_MSG_ID );
         _encoder.putString( "Hello, World!" );
         _encoder.putDouble( 4.2 );
         _UDPEndPoint.send( _encoder );
         _logger.printf( "'Hello' message sent\n" );
      }
      catch( final IOException e ) {
         e.printStackTrace();
      }
   }

   private void parseHello( short msgId, CoDec decoder ) {
      final String   text  = decoder.getString();
      final double   value = decoder.getDouble();
      _logger.printf( "%d received: '%s', %f\n", msgId, text, value );
   }

   private void sendFamily() {
      try {
         final Family family = new Family();
         family.getParent1().setForname( "Aubin" );
         family.getParent1().setName( "Mahé" );
         family.getParent1().setBirthdate( LocalDate.parse( "1966-01-24" ));
         family.setParent2( new Person( "Muriel", "Le Nain", LocalDate.parse( "1973-01-26" )));
         family.getChildren().add( new Person( "Eve", "Mahé", LocalDate.parse( "2008-02-28" )));

         _encoder.init( FAMILY_REFERENCE_MSG_ID );
         _encoder.putReference( family );
         _UDPEndPoint.send( _encoder );
         _logger.printf( "'%s' sent\n", Family.class.getName());
      }
      catch( final IOException e ) {
         e.printStackTrace();
      }
   }

   private void parseFamily( short msgId, CoDec decoder ) {
      final Family family = decoder.getReference();
      _logger.printf( "%d received: %s\n", msgId, family );
   }

   /**
    * Pour Thales :
    * --interface=192.168.56.1
    * --target-host=192.168.56.1
    * --local-port=2416
    * --remote-port=2417
    * java -cp bin thales.io.socket.generator.Main --interface=192.168.56.1 --target-host=192.168.56.1 --local-port=2416 --remote-port=2417
    */
   @Override
   public void start( Stage primaryStage ) throws Exception {
      final Map<String, String> args = getParameters().getNamed();
      if( args.containsKey( "show-interfaces" )) {
         UDPChannel.showAvailableInterfaces();
         System.exit( 1 );
      }
      final String   intrfc     = args.getOrDefault( "interface"  , "192.168.1.7" );
      final String   targetHost = args.getOrDefault( "target-host", "192.168.1.7" );
      final String   localStr   = args.getOrDefault( "local-port" , "2416"        );
      final String   remoteStr  = args.getOrDefault( "remote-port", "2417"        );
      final int      local      = Integer.parseInt( localStr );
      final int      remote     = Integer.parseInt( remoteStr );
      final TextArea logPane    = new TextArea();
//      for( final String family : Font.getFamilies()) {
//         System.err.println( family );
//      }
      logPane.setFont( Font.font( "Monospaced" ));
      _logger = new Logger( logPane );

      init( intrfc, local, targetHost, remote );

      final Button     sendHello = new Button( "Hello" );
      final Button     sendTree  = new Button( "Tree" );
      final BorderPane rootPane  =
         new BorderPane( new VBox( new HBox( sendHello, sendTree ), logPane ));
      VBox.setVgrow( logPane, Priority.ALWAYS );
      final Scene scene = new Scene ( rootPane );
      primaryStage.setScene( scene );
      primaryStage.centerOnScreen();
      primaryStage.setTitle( "Listening on " + intrfc + ':' + localStr );
      primaryStage.show();

      sendHello.setOnAction( e -> sendHello());
      sendTree .setOnAction( e -> sendFamily());
   }

   public static void main( String[] args ) {
      launch( args );
   }
}
