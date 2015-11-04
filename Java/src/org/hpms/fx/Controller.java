package org.hpms.fx;

import javafx.application.Application.Parameters;
import javafx.stage.Stage;

public interface Controller {

   public void setArgs( Object context, Parameters parameters ) throws Exception;

   public String getAppTitle();

   public double getAppX();

   public double getAppY();

   public void onShown( Stage stage );

   public void onCloseMainStage( Stage stage );
}
