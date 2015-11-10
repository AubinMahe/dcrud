package org.hpms.fx;

import java.net.URL;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.layout.BorderPane;
import javafx.stage.Stage;

public final class App extends Application {

   private static URL    _fxml;
   private static Object _context;

   @Override
   public void start( Stage stage ) throws Exception {
      final FXMLLoader loader = new FXMLLoader( _fxml );
      final Parent     view   = loader.load();
      final Controller ctrl   = loader.getController();
      ctrl.initialize( _context, getParameters());
      stage.setScene( new Scene( new BorderPane( view )));
      stage.setTitle( ctrl.getAppTitle());
      stage.setX( ctrl.getAppX());
      stage.setY( ctrl.getAppY());
      stage.setOnShown( e -> ctrl.onShown( stage ));
      stage.setOnCloseRequest( e -> ctrl.onCloseMainStage( stage ));
      stage.show();
   }

   public static void launch( String[] args, URL fxml, Object context ) {
      _fxml    = fxml;
      _context = context;
      Application.launch( App.class, args );
   }

   public static void launch( String[] args, URL fxml ) {
      _fxml    = fxml;
      _context = null;
      Application.launch( App.class, args );
   }

   public static void launch( String[] args, Class<? extends Controller> mainController ) {
      _fxml    = mainController.getResource( mainController.getSimpleName() + ".fxml" );
      _context = null;
      Application.launch( App.class, args );
   }
}
