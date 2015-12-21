package tests;

import java.net.URL;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.layout.BorderPane;
import javafx.stage.Stage;

public final class App extends Application {

   private static URL _fxml;

   @Override
   public void start( Stage stage ) throws Exception {
      final FXMLLoader loader = new FXMLLoader( _fxml );
      final Parent     view   = loader.load();
      final Controller ctrl   = loader.getController();
      ctrl.initialize( stage, getParameters());
      stage.setScene( new Scene( new BorderPane( view )));
      stage.show();
   }

   public static void launch( String[] args, Class<? extends Controller> mainController ) {
      _fxml = mainController.getResource( mainController.getSimpleName() + ".fxml" );
      Application.launch( App.class, args );
   }
}
