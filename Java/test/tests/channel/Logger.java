package tests.channel;

import org.hpms.mw.channel.ILogger;

import javafx.application.Platform;
import javafx.scene.control.TextArea;

public class Logger implements ILogger {

   private final TextArea _view;

   Logger( TextArea view ) {
      _view = view;
   }

   @Override
   public void printf( String format, Object ... args ) {
      Platform.runLater(() -> _view.appendText( String.format( format, args )));
   }
}
