import java.awt.BasicStroke;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.Polygon;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Optional;
import java.util.TimeZone;

import javax.imageio.ImageIO;
import javax.swing.Timer;

import org.w3c.dom.Element;
import org.w3c.dom.Node;

import javafx.application.Application;
import javafx.beans.binding.Bindings;
import javafx.beans.property.DoubleProperty;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.embed.swing.SwingFXUtils;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.geometry.Rectangle2D;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.Button;
import javafx.scene.control.ButtonType;
import javafx.scene.control.ChoiceBox;
import javafx.scene.control.ChoiceDialog;
import javafx.scene.control.Label;
import javafx.scene.control.RadioButton;
import javafx.scene.control.TextField;
import javafx.scene.control.TextInputDialog;
import javafx.scene.control.ToggleGroup;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.image.WritableImage;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyCombination;
import javafx.scene.input.KeyEvent;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.StackPane;
import javafx.scene.media.Media;
import javafx.scene.media.MediaPlayer;
import javafx.scene.media.MediaView;
import javafx.scene.paint.Color;
import javafx.stage.Screen;
import javafx.stage.Stage;
import javafx.stage.StageStyle;
import javafx.stage.WindowEvent;
import javafx.stage.FileChooser;
import javafx.stage.FileChooser.ExtensionFilter;




// MicroBit Epidemic Slideshow
// Hacked out of the Barcode Epidemic Presenter, 3.01
// It expects lang.txt and script.txt
@SuppressWarnings("restriction")

public class MicroEpiSlideshow extends Application {
  /* GUI for screen control */
  
  //final String dataPath = "C:/Files/Dev/public-events-microbit-epidemic/data/";
  final String dataPath = "../data/";
  public int no_events = 0;
  boolean unsaved_changes = false;
 
  final RadioButton rb_on = new RadioButton("ON");
  final RadioButton rb_off = new RadioButton("OFF");
  public final RadioButton nl_on = new RadioButton("ON");
  public final RadioButton nl_off = new RadioButton("OFF");
  final ObservableList<String> config_choices = FXCollections.observableArrayList();
  final ChoiceBox<String> cb_configs = new ChoiceBox<String>(config_choices);
  final Label l_display = new Label("Display");
  final Label l_location = new Label("Location (x,y)");
  final Label l_size = new Label("Size (w,h)");
  final Label l_epi = new Label("None set yet");
  final Button b_epi = new Button("Set Datafile");

  final GridPane grid = new GridPane();
  final TextField tf_x = new TextField();
  final TextField tf_y = new TextField();
  final TextField tf_w = new TextField();
  final TextField tf_h = new TextField();
  final HBox hb_display = new HBox();
  final HBox hb_location = new HBox();
  final HBox hb_size = new HBox();
  final HBox hb_data = new HBox();

  final ToggleGroup tg_display = new ToggleGroup();
  final Button b_detect = new Button("Detect Fullscreen");
  final Label l_config = new Label("Saved Settings:");
  final HBox hb_configs = new HBox();
  final Button b_saveConfig = new Button("Save");
  final Button b_saveAsConfig = new Button("Save As");
  final Button b_delConfig = new Button("Delete");

  Element configs_xml;
  
  Stage displayStage;
  final StackPane displayStageSP = new StackPane();
  Scene displayScene = null;

  
  //String in_file = "C:/Files/Dev/Eclipse/microepi-manager/498461975_289.csv";
  String in_file = null;
  
  
  LangSupport L = new LangSupport();

  // Below - set the right timezone!
  
  GregorianCalendar gc = null;
  int left_margin = 100;
  int right_margin = 100;
  int top_margin = 100;
  int bottom_margin = 100;

  BufferedImage layer;
  Timer jtimer;
  
  Image pauseImg = new Image("media/pause.png");
  ImageView pauseButton = new ImageView();
  ImageView epiImage = new ImageView();
  MediaView epiMovie = new MediaView();
  MediaPlayer epiPlayer;  
  WritableImage fx_img;
  Dimension screen;
  String webFolder="";
  String RScript="";
  String RNetGraph="";
  String div_col1="#000000", div_col2="#000000", div_col3 = "#707070";
  ArrayList<String> script = new ArrayList<String>();
  int[] script_indexes;
  int current_script_line;
  
  ArrayList<String[]> epi_csv = new ArrayList<String[]>(); // The main data file.
  
  private static byte COL_EVENT = 0;
  private static byte COL_TIMEH = 1;
  private static byte COL_TIMEM = 2;
  private static byte COL_INFBY = 3;
  //private static byte COL_SEED = 4;
  //private static byte COL_REC = 5;
  //private static byte COL_CAT = 6;
  private static byte COL_VICTIM = 7;
  private static byte COL_NCONS = 8;
  


  public void GUItoXML(Node tag) {
    Tools.setAttribute(tag, "x", tf_x.getText());
    Tools.setAttribute(tag, "y", tf_y.getText());
    Tools.setAttribute(tag, "w", tf_w.getText());
    Tools.setAttribute(tag, "h", tf_h.getText());
  }

  public void XMLtoGUI(Node tag) {
    tf_x.setText(Tools.getAttribute(tag, "x"));
    tf_y.setText(Tools.getAttribute(tag, "y"));
    tf_w.setText(Tools.getAttribute(tag, "w"));
    tf_h.setText(Tools.getAttribute(tag, "h"));
    displayStage.setX(Integer.parseInt(tf_x.getText()));
    displayStage.setY(Integer.parseInt(tf_y.getText()));
    displayStage.setWidth(Integer.parseInt(tf_w.getText()));
    displayStage.setHeight(Integer.parseInt(tf_h.getText()));
  }

  void changeTF() {
    unsaved_changes = true;
    b_saveConfig.setDisable(false);
    displayStage.setX(Integer.parseInt(tf_x.getText()));
    displayStage.setY(Integer.parseInt(tf_y.getText()));
    displayStage.setWidth(Integer.parseInt(tf_w.getText()));
    displayStage.setHeight(Integer.parseInt(tf_h.getText()));
  }

  public void createDummyXML() {
    try {
      PrintWriter PW = new PrintWriter(new File("configs.xml"));
      PW.println("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>");
      PW.println("<configs>");
      PW.println("  <recent name=\"Default Config\"/>");
      PW.println("    <config name=\"Default Config\" x=\"1920\" y=\"0\" w=\"1920\" h=\"1080\" b=\"\" />");
      PW.println("</configs>");
      PW.close();
    } catch (Exception e) {
    }
  }

  public void loadXML() {
    if (!new File("configs.xml").exists()) createDummyXML();
    configs_xml = Tools.loadDocument("configs.xml");
    int countConfigs = Tools.countChildren(configs_xml, "config");
    if (countConfigs == 0) {
      createDummyXML();
      configs_xml = Tools.loadDocument("configs.xml");
    }
    for (int i = 0; i < Tools.countChildren(configs_xml, "config"); i++) {
      config_choices.add(Tools.getAttribute(Tools.getChildNo(configs_xml, "config", i), "name"));
    }
    FXCollections.sort(config_choices);
    String recent = Tools.getAttribute(Tools.getTag(configs_xml, "recent"),"name");
    if (!config_choices.contains(recent)) {
      recent = Tools.getAttribute(Tools.getChildNo(configs_xml,"config",0),"name");
      Tools.setAttribute(Tools.getTag(configs_xml, "recent"),"name",recent);
      Tools.writeXML(configs_xml, "configs.xml");
    }
    cb_configs.getSelectionModel().select(recent);
  }


  
  public void showDisplayScreen(boolean smoothly) {
    displayStage.setWidth(Integer.parseInt(tf_w.getText()));
    displayStage.setHeight(Integer.parseInt(tf_h.getText()));
    displayStage.setX(Integer.parseInt(tf_x.getText()));
    displayStage.setY(Integer.parseInt(tf_y.getText()));
    displayStageSP.setStyle("-fx-background-color: BLACK");
    displayScene.setFill(Color.BLACK);
    displayStage.show();
  }

  public void hideDisplayScreen(boolean smoothly) {
    displayStage.hide();
  }
  
  private void loadScript() {
    try {
      BufferedReader br = new BufferedReader(new FileReader("script.txt"));
      String s = br.readLine();
      String[] bits;
      while (s!=null) {
        s=s.trim();
        if ((s.length()>0) && (!s.startsWith("#"))) {
          bits=s.split(":");
          bits[0]=bits[0].toUpperCase();
          if (bits[0].toUpperCase().equals("WEBFOLDER")) webFolder=s.substring(10);
          else if (bits[0].toUpperCase().equals("RNETGRAPH")) RNetGraph=s.substring(10);          
          else if (bits[0].toUpperCase().equals("RSCRIPT")) RScript=s.substring(8);
          else if (bits[0].toUpperCase().equals("TIMEZONE")) gc = new GregorianCalendar(TimeZone.getTimeZone(bits[1]));
          else if (bits[0].toUpperCase().equals("LANGUAGE")) L.setLanguage(bits[1]);
          else if (bits[0].toUpperCase().equals("SCRIPT")) {
            s=br.readLine();
            boolean done=false;
            while ((s!=null) && (!done)) {
              s=s.trim();
              if (s.toUpperCase().equals("LOOP")) done=true;
              else {
                if (!s.toUpperCase().equals("LOOP")) {
                  if (!s.startsWith("#")) {
                    if (s.length()>0) {
                      script.add(s);
                    }
                  }
                }
              }
              s=br.readLine();
            }
          }
        }
        if (s!=null) s=br.readLine();
      }
      br.close();
      if (!webFolder.endsWith(File.separator)) webFolder+=File.separator;
            
      script_indexes = new int[script.size()];
      current_script_line=0;
      if (gc==null) gc = new GregorianCalendar(TimeZone.getTimeZone("GMT"));
    } catch (Exception e) {
      System.out.println("Error Loading Config Script");
      e.printStackTrace();
      System.exit(1);
    }
  }
  
  
  
  public void start(Stage primaryStage) throws Exception {
    
    /* Control GUI */
    int gridy = 0;
    
    grid.setAlignment(Pos.CENTER);
    grid.setHgap(10);
    grid.setVgap(10);
    grid.setPadding(new Insets(20, 20, 20, 20));

    grid.add(l_display, 0, gridy);
    rb_on.setToggleGroup(tg_display);
    rb_off.setToggleGroup(tg_display);
    hb_display.getChildren().add(rb_off);
    rb_on.setPadding(new Insets(0, 10, 0, 10));
    rb_off.setPadding(new Insets(0, 10, 0, 10));
    rb_off.setSelected(true);
    hb_display.getChildren().add(rb_on);
    grid.add(hb_display, 1, gridy++);

    rb_on.setOnAction(new EventHandler<ActionEvent>() {
      public void handle(ActionEvent e) {
        showDisplayScreen(false);
      }
    });

    rb_off.setOnAction(new EventHandler<ActionEvent>() {
      public void handle(ActionEvent e) {
        hideDisplayScreen(false);

      }
    });

    grid.add(l_config, 0, gridy);
    grid.add(cb_configs, 1, gridy++);
    hb_configs.getChildren().add(b_saveConfig);
    hb_configs.getChildren().add(b_saveAsConfig);
    hb_configs.getChildren().add(b_delConfig);

    grid.add(hb_configs, 1, gridy++);

    // Location line

    grid.add(l_location, 0, gridy);
    tf_x.setMaxWidth(50);
    tf_y.setMaxWidth(50);
    hb_location.getChildren().add(tf_x);
    hb_location.getChildren().add(tf_y);
    grid.add(hb_location, 1, gridy++);

    // Size line

    grid.add(l_size, 0, gridy);
    tf_w.setMaxWidth(50);
    tf_h.setMaxWidth(50);
    hb_size.getChildren().add(tf_w);
    hb_size.getChildren().add(tf_h);
    grid.add(hb_size, 1, gridy++);
    
    
    // Detect

    grid.add(b_detect, 1, gridy++);
    b_detect.setOnAction(new EventHandler<ActionEvent>() {
      public void handle(ActionEvent e) {
        List<String> choices = new ArrayList<>();
        int scr_no = 1;
        ObservableList<Screen> screens = Screen.getScreens();
        for (Screen scr : screens) {
          Rectangle2D bounds = scr.getBounds();
          choices.add(scr_no + ": " + (int) bounds.getWidth() + "x"
              + (int) bounds.getHeight() + " at (" + (int) bounds.getMinX()
              + "," + (int) bounds.getMinY() + ")");
          scr_no++;
        }
        ChoiceDialog<String> dialog = new ChoiceDialog<>(choices.get(choices.size() - 1), choices);
        dialog.setTitle("Screen detection");
        dialog.setHeaderText("Screens Detected:");
        dialog.setContentText("Choose screen: ");
        Optional<String> result = dialog.showAndWait();
        if (result.isPresent()) {
          String r = result.get();
          int pick = Integer.parseInt(r.substring(0, r.indexOf(":"))) - 1;
          Screen scr = screens.get(pick);
          Rectangle2D bounds = scr.getBounds();
          tf_w.setText(String.valueOf((int) bounds.getWidth()));
          tf_h.setText(String.valueOf((int) bounds.getHeight()));
          tf_x.setText(String.valueOf((int) bounds.getMinX()));
          tf_y.setText(String.valueOf((int) bounds.getMinY()));
          unsaved_changes = true;
          b_saveConfig.setDisable(false);
        }
      }
    });
    
    // Set Datafile
    
    
    hb_data.getChildren().add(b_epi);
    grid.add(hb_data,  0,  gridy);
    grid.add(l_epi, 1, gridy++);
    
    final Stage _stage = primaryStage;
    b_epi.setOnAction(new EventHandler<ActionEvent>() {
      @Override
      public void handle(ActionEvent e) {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("Choose CSV File");
        fileChooser.getExtensionFilters().addAll(
            new ExtensionFilter("CSV Files", "*.csv"));
        fileChooser.setInitialDirectory(new File(dataPath));
        File selectedFile = fileChooser.showOpenDialog(_stage);
        if (selectedFile != null) {
          in_file = selectedFile.getPath();
          l_epi.setText(selectedFile.getName());
          refreshData();
        }
        unsaved_changes = true;

      }
    });
    
    primaryStage.setOnCloseRequest(new EventHandler<WindowEvent>() {
      public void handle(WindowEvent we) {
        hideDisplayScreen(false);
        displayStage.close();
        System.exit(0);
      }
    });
    
    b_saveConfig.setOnAction(new EventHandler<ActionEvent>() {
      @Override
      public void handle(ActionEvent e) {
        Node configTag = Tools.getTagWhereAttr(configs_xml, "config", "name", cb_configs.getValue().toString());
        GUItoXML(configTag);
        Tools.writeXML(configs_xml, "configs.xml");
        unsaved_changes = false;
        b_saveConfig.setDisable(true);
      }
    });

    b_saveAsConfig.setOnAction(new EventHandler<ActionEvent>() {
      @Override
      public void handle(ActionEvent e) {
        TextInputDialog tid = new TextInputDialog();
        tid.setTitle("Please enter name for new config");
        tid.setHeaderText(null);
        
        Optional<String> result = tid.showAndWait();
        if (result.isPresent()) {
          String res = result.get().trim();
          if (Tools.getTagWhereAttr(configs_xml, "config", "name", res) != null) {
            Alert dupError = new Alert(AlertType.ERROR, "A configuration with that name already exists.", ButtonType.OK);
            dupError.showAndWait();
          } else {
            Element n = Tools.addTag(configs_xml, "config");
            Tools.setAttribute(n, "name", res);
            GUItoXML(n);
            Tools.setAttribute(Tools.getTag(configs_xml, "recent"), "name", res);
            Tools.writeXML(configs_xml, "configs.xml");
            unsaved_changes = false;
            b_saveConfig.setDisable(true);
            config_choices.add(res);
            FXCollections.sort(config_choices);
            cb_configs.getSelectionModel().select(res);
            
          }
        }
      }
    });

    b_delConfig.setOnAction(new EventHandler<ActionEvent>() {
      @Override
      public void handle(ActionEvent e) {
        if (Tools.countChildren(configs_xml, "config") <= 1) {
          Alert zeroError = new Alert(AlertType.ERROR,"Can't delete the only configuration...", ButtonType.OK);
          zeroError.showAndWait();
        } else {
          Alert confirmDel = new Alert(AlertType.CONFIRMATION, "Confirm deleting this configuration...", ButtonType.OK, ButtonType.CANCEL);
          Optional<ButtonType> result = confirmDel.showAndWait();
          if ((result.isPresent()) && (result.get() == ButtonType.OK)) {
            Node x = Tools.getTagWhereAttr(configs_xml, "config", "name", cb_configs.getValue().toString());
            x.getParentNode().removeChild(x);
            int index = config_choices.indexOf(cb_configs.getValue().toString());
            config_choices.remove(index);
            while (index >= config_choices.size()) index--;
            cb_configs.getSelectionModel().select(index);
            Tools.setAttribute(Tools.getTag(configs_xml, "recent"), "name", cb_configs.getValue().toString());
            Tools.writeXML(configs_xml, "configs.xml");
            unsaved_changes = false;
            b_saveConfig.setDisable(true);
          }
        }
      }
    });

    cb_configs.getSelectionModel().selectedItemProperty().addListener(new ChangeListener<String>() {
      @Override
      public void changed(ObservableValue<? extends String> observableValue, String number, String number2) {
        boolean proceed = (!unsaved_changes);
        if (unsaved_changes) {
          Alert confirmDel = new Alert(AlertType.CONFIRMATION,
              "There are unsaved changes to this configuration. Proceed anyway?",
              ButtonType.OK, ButtonType.CANCEL);
          Optional<ButtonType> result = confirmDel.showAndWait();
          if ((result.isPresent()) && (result.get() == ButtonType.OK)) proceed = true;
        }
        
        if (proceed) {
          cb_configs.getSelectionModel().select(number2);
          Node config = Tools.getTagWhereAttr(configs_xml, "config", "name",cb_configs.getValue().toString());
          XMLtoGUI(config);
          Tools.setAttribute(Tools.getTag(configs_xml, "recent"), "name", cb_configs.getValue().toString());
          Tools.writeXML(configs_xml, "configs.xml");
          unsaved_changes = false;
          b_saveConfig.setDisable(true);
        } else {
          cb_configs.getSelectionModel().select(number);
        }
      }
    });

    EventHandler<ActionEvent> unsaveAndResizeEvent = new EventHandler<ActionEvent>() {
      @Override
      public void handle(ActionEvent e) {
        changeTF();
      }
    };

    tf_x.setOnAction(unsaveAndResizeEvent);
    tf_y.setOnAction(unsaveAndResizeEvent);
    tf_w.setOnAction(unsaveAndResizeEvent);
    tf_h.setOnAction(unsaveAndResizeEvent);

    displayStage = new Stage(StageStyle.UNDECORATED);
    displayStage.getIcons().add(new Image("file:resources/sbscreen_icon.png"));
    displayStage.setAlwaysOnTop(true);
    
    displayScene = new Scene(displayStageSP, displayStage.getWidth(), displayStage.getHeight(), Color.BLACK);
    displayStage.setFullScreenExitKeyCombination(KeyCombination.NO_MATCH);
    displayStage.setScene(displayScene);
    displayStage.setResizable(false);
    
    displayStage.setOnCloseRequest(new EventHandler<WindowEvent>() {
      public void handle(WindowEvent we) {
        hideDisplayScreen(false);
        rb_off.setSelected(true);
      }
    });
    
    displayScene.setOnKeyReleased(new EventHandler<KeyEvent>() {
      @Override
      public void handle(KeyEvent e) {
        if (e.getCode()==KeyCode.ESCAPE) {
          hideDisplayScreen(false);
          rb_off.setSelected(true);
        } else if (e.getCode()==KeyCode.Q) {
          jump();
        } else {
          pause();
        }
      }
    });
    
    loadXML();
    b_saveConfig.setDisable(true);
    unsaved_changes = false;
    Scene scene = new Scene(grid);
    scene.setOnKeyReleased(new EventHandler<KeyEvent>() {
      @Override
      public void handle(KeyEvent e) {
        if (rb_on.isSelected()) {
          if (e.getCode()==KeyCode.ESCAPE) {
            hideDisplayScreen(false);
            rb_off.setSelected(true);
          } else if (e.getCode()==KeyCode.Q) {
            jump();
          } else {
            pause();
          }
        }
      }
    });


    primaryStage.setScene(scene);
    primaryStage.sizeToScene();
    primaryStage.setTitle("Micro:bit Epidemic Live Slideshow");
    primaryStage.setResizable(false);
    primaryStage.show();
    
    loadScript();
    
    screen = Toolkit.getDefaultToolkit().getScreenSize();
    layer = new BufferedImage(screen.width, screen.height, BufferedImage.TYPE_4BYTE_ABGR);
    fx_img = new WritableImage(screen.width, screen.height);
    epiImage.setImage(fx_img);
    pauseButton.setImage(pauseImg);
    pauseButton.setVisible(false);
    displayStageSP.getChildren().add(epiImage);
    displayStageSP.getChildren().add(pauseButton);
    
    displayScene.setFill(javafx.scene.paint.Color.BLACK);
   
    displayStage.setFullScreenExitKeyCombination(KeyCombination.NO_MATCH);
    displayStage.setScene(displayScene);
    
    displayStage.show();
    displayStageSP.getChildren().add(epiMovie);
    pauseButton.setTranslateX(-50+(screen.width/2));
    pauseButton.setTranslateY(50-(screen.height/2));

    epiImage.setVisible(true);
    epiMovie.setVisible(false);
    DoubleProperty width = epiMovie.fitWidthProperty();
    DoubleProperty height = epiMovie.fitHeightProperty();
    width.bind(Bindings.selectDouble(epiMovie.sceneProperty(), "width"));
    height.bind(Bindings.selectDouble(epiMovie.sceneProperty(), "height"));
    width.bind(Bindings.selectDouble(epiImage.sceneProperty(), "width"));
    height.bind(Bindings.selectDouble(epiImage.sceneProperty(), "height"));
    epiMovie.setPreserveRatio(true);
    jtimer = new Timer(10000,new java.awt.event.ActionListener() {
      public void actionPerformed(java.awt.event.ActionEvent e) {
        nextChapter();
                
      }
    });
    jtimer.setInitialDelay(100);
    jtimer.setRepeats(false);
    jtimer.start();
  }

  public void generateCasesGraph(BufferedImage bi, boolean include_unconfirmed, boolean cumulative) {
    // Work out time axis - seconds since midnight.
    
    long first_time=0;
    long last_time=0;
    int min_time=Integer.MAX_VALUE, max_time = Integer.MIN_VALUE;
    for (int i=0; i<epi_csv.size(); i++) {
      if (epi_csv.get(i)[COL_EVENT].equals("I")) {
        int time_s = Integer.parseInt(epi_csv.get(i)[COL_TIMEH])*3600;
        time_s += Math.round(Float.parseFloat(epi_csv.get(i)[COL_TIMEM])*60);
        min_time = Math.min(min_time, time_s);
        max_time = Math.max(max_time, time_s);
      }
    }
    gc.setTimeInMillis(System.currentTimeMillis());
    gc.set(GregorianCalendar.HOUR_OF_DAY, 0);
    gc.set(GregorianCalendar.MINUTE, 0);
    gc.set(GregorianCalendar.SECOND, 0);
    long start_of_day = gc.getTimeInMillis();
    first_time = start_of_day + (1000*min_time);
    last_time = start_of_day + (1000*max_time);
        
    Font titleFont = new Font("Calibri", Font.BOLD, 32);
    Font subTitleFont = new Font("Calibri", Font.PLAIN, 20);
    Font axisFont = new Font("Calibri", Font.PLAIN, 14);
    Graphics2D g = getNiceGraphics(bi);
    FontMetrics fm = g.getFontMetrics();
    g.setColor(java.awt.Color.WHITE);
    g.fillRect(0, 0, bi.getWidth(), bi.getHeight());
    g.setFont(titleFont);
    gc.setTimeInMillis(first_time);
    gc.set(GregorianCalendar.MINUTE, 0);
    long time1 = gc.getTimeInMillis();
    gc.setTimeInMillis(last_time);
    long time2 = gc.getTimeInMillis();
    // 5,10,15,20,30,hour
    long range = time2 - time1;
    range /= 1000;
    range /= 60; // Range is now in minutes

    int step = 60; // Step size in minutes

    if ((range / 60) > 6) step = 60; // Hourly stepsize
    else if ((range / 30) > 6) step = 30; // Half-hourly
    else if ((range / 20) > 6) step = 20; // 20-mins
    else if ((range / 15) > 6) step = 15; // 15-mins
    else if ((range / 10) > 6) step = 10;
    else if ((range / 5) > 6) step = 5;
    else if ((range / 3) > 6) step = 3;
    else if ((range / 2) > 6) step = 2;
    else step = 1;

    int no_steps = 1+(int) ((time2 - time1) / (1000 * 60 * step));
    if (no_steps==0) no_steps=1;
    g.setColor(java.awt.Color.BLACK);
    int step_pix = (int) Math.floor(((bi.getWidth() - left_margin) - right_margin) / no_steps);
    g.drawLine(left_margin, (bi.getHeight() - bottom_margin) + 5, bi.getWidth() - right_margin, (bi.getHeight() - bottom_margin) + 5);
    g.drawLine(left_margin - 5, bi.getHeight() - bottom_margin, left_margin - 5, top_margin);
    int step_no = 0;
    g.setFont(titleFont);
    fm = g.getFontMetrics();
    int left_of_title = 0;
    int right_of_title;
    int bottom_of_title;
    if (!cumulative) {
      left_of_title = (int) ((bi.getWidth() - fm.stringWidth(L.getText("CasesGraph"))) / 2);
      right_of_title = left_of_title + fm.stringWidth(L.getText("CasesGraph"));
      bottom_of_title = (top_margin / 2) - 10;
      g.drawString(L.getText("CasesGraph"), left_of_title, bottom_of_title);
    } else {
      left_of_title = (int) ((bi.getWidth() - fm.stringWidth(L.getText("CumulCasesGraph"))) / 2);
      right_of_title = left_of_title + fm.stringWidth(L.getText("CumulCasesGraph"));
      bottom_of_title = (top_margin / 2) - 10;
      g.drawString(L.getText("CumulCasesGraph"), left_of_title, bottom_of_title);
    }
    g.setFont(subTitleFont);
    Graphics2D grot = (Graphics2D) g.create();
    grot.rotate(-Math.PI / 2);
    fm = grot.getFontMetrics();
    grot.drawString(L.getText("TotalInfs"), -(bi.getHeight() / 2) - (fm.stringWidth(L.getText("TotalInfs")) / 2), left_margin / 3);
    grot.dispose();
    g.drawString(L.getText("TimeAxis"), (bi.getWidth() - fm.stringWidth(L.getText("TimeAxis"))) / 2, bi.getHeight() - (bottom_margin / 6));

    fm = g.getFontMetrics();
    if (include_unconfirmed) {
      bottom_of_title += 15;
      g.setColor(java.awt.Color.RED);
      g.fillRect(left_of_title, bottom_of_title, 20, 20);
      g.setColor(java.awt.Color.BLACK);
      g.drawRect(left_of_title, bottom_of_title, 20, 20);
      g.drawString(L.getText("ConfirmedCases"), left_of_title + 30, bottom_of_title + 18);
      g.setColor(java.awt.Color.LIGHT_GRAY);
      g.fillRect(right_of_title - 20, bottom_of_title, 20, 20);
      g.setColor(java.awt.Color.BLACK);
      g.drawRect(right_of_title - 20, bottom_of_title, 20, 20);
      g.drawString(L.getText("PotentialCases"), (right_of_title - 30) - fm.stringWidth(L.getText("PotentialCases")), bottom_of_title + 18);
    }
    g.setFont(axisFont);
    fm = g.getFontMetrics();
    int[] confirmed_cases = new int[no_steps];
    int[] all_cases = new int[no_steps];

    /* Find the confirmed cases. */
    for (int i = 0; i < epi_csv.size(); i++) {
      if (epi_csv.get(i)[COL_EVENT].equals("I")) {
        int time_s = Integer.parseInt(epi_csv.get(i)[COL_TIMEH])*3600;
        time_s += Float.parseFloat(epi_csv.get(i)[COL_TIMEM])*60;
        long event_time = (start_of_day+(time_s*1000))/1000;
        
        int index = (int) (((event_time - (time1/1000)) / 60) / step);
        all_cases[index] += Integer.parseInt(epi_csv.get(i)[COL_NCONS]);
        for (int j = 0; j < epi_csv.size(); j++) {
          if (epi_csv.get(j)[COL_EVENT].equals("I")) {
            if (epi_csv.get(j)[COL_INFBY].equals(epi_csv.get(i)[COL_VICTIM])) {
              time_s = Integer.parseInt(epi_csv.get(i)[COL_TIMEH])*3600;
              time_s += Float.parseFloat(epi_csv.get(i)[COL_TIMEM])*60;
              event_time = (start_of_day+(1000*time_s))/1000;
              index = (int) (((event_time - (time1 / 1000)) / 60) / step);
              confirmed_cases[index]++;
            }
          }
        }
      }
    }

    if (cumulative) {
      for (int i = 1; i < no_steps; i++) {
        confirmed_cases[i] += confirmed_cases[i - 1];
        all_cases[i] += all_cases[i - 1];
      }
    }
    int max_y = 0;
    int min_y = 0;
    if (!include_unconfirmed) {
      for (int i = 0; i < all_cases.length; i++) {
        all_cases[i] = confirmed_cases[i];
      }
    }
    if (all_cases.length > 0) {
      max_y = all_cases[0];
      min_y = all_cases[0];
      for (int i = 1; i < all_cases.length; i++) {
        max_y = Math.max(all_cases[i], max_y);
        min_y = Math.min(all_cases[i], min_y);
      }
    }
    min_y = 0; // Actually... use 0 as the minimum...
    if (max_y == 0) max_y = 1;
    double scale_y = (bi.getHeight() - (bottom_margin + top_margin)) / (max_y - min_y);
    if (epi_csv.size() > 0) {
      while (time1 < last_time) {
        gc.setTimeInMillis(time1);
        String hour = String.valueOf(gc.get(GregorianCalendar.HOUR_OF_DAY));
        if (hour.length() == 1) hour = "0" + hour;
        String min = String.valueOf(gc.get(GregorianCalendar.MINUTE));
        if (min.length() == 1) min = "0" + min;
        String s1 = hour + ":" + min + " -";
        gc.add(GregorianCalendar.MINUTE, step);
        time2 = gc.getTimeInMillis();
        hour = String.valueOf(gc.get(GregorianCalendar.HOUR_OF_DAY));
        if (hour.length() == 1) hour = "0" + hour;
        min = String.valueOf(gc.get(GregorianCalendar.MINUTE));
        if (min.length() == 1) min = "0" + min;
        String s2 = hour + ":" + min;
        int mid = left_margin + (step_no * step_pix) + (step_pix / 2);
        g.drawString(s1, mid - (fm.stringWidth(s1) / 2), 25 + (bi.getHeight() - bottom_margin));
        g.drawString(s2, mid - (fm.stringWidth(s2) / 2), 45 + (bi.getHeight() - bottom_margin));
        g.drawLine(left_margin + (step_no * step_pix), (bi.getHeight() - bottom_margin) + 5, left_margin + (step_no * step_pix), (bi.getHeight() - bottom_margin) + 8);
        if (include_unconfirmed) {
          g.setColor(java.awt.Color.LIGHT_GRAY);
          g.fillRect((int) (left_margin + (step_no * step_pix)), (int) (bi.getHeight() - (bottom_margin + (scale_y * (all_cases[step_no] - min_y)))), step_pix, (int) ((scale_y * (all_cases[step_no] - min_y))));
          g.setColor(java.awt.Color.BLACK);
          g.drawRect((int) (left_margin + (step_no * step_pix)), (int) (bi.getHeight() - (bottom_margin + (scale_y * (all_cases[step_no] - min_y)))), step_pix, (int) ((scale_y * (all_cases[step_no] - min_y))));
        }
        g.setColor(java.awt.Color.RED);
        g.fillRect((int) (left_margin + (step_no * step_pix)), (int) (bi.getHeight() - (bottom_margin + (scale_y * (confirmed_cases[step_no] - min_y)))), step_pix, (int) ((scale_y * (confirmed_cases[step_no] - min_y))));
        g.setColor(java.awt.Color.BLACK);
        g.drawRect((int) (left_margin + (step_no * step_pix)), (int) (bi.getHeight() - (bottom_margin + (scale_y * (confirmed_cases[step_no] - min_y)))), step_pix, (int) ((scale_y * (confirmed_cases[step_no] - min_y))));
        time1 = time2;
        step_no++;
      }

      int y_scale = (int) Math.round((max_y - min_y) / 6);
      if (y_scale < 1)
        y_scale = 1;

      for (int j = min_y; j <= max_y; j += y_scale) {
        int ypix = (int) (bi.getHeight() - (bottom_margin + (scale_y * (j - min_y))));
        g.drawLine(left_margin - 5, ypix, left_margin - 8, ypix);
        g.drawString(String.valueOf(j), left_margin - 12 - fm.stringWidth(String.valueOf(j)), ypix + 5);

      }
    }
    g.dispose();
    SwingFXUtils.toFXImage(layer, fx_img);
  }

  public void generateGenTimeGraph(BufferedImage bi) {
    long largest_gt=0;
    ArrayList<Integer> gts = new ArrayList<Integer>();
    for (int i=0; i<epi_csv.size(); i++) {
      if (epi_csv.get(i)[COL_EVENT].equals("I")) {
        int time0 = Integer.parseInt(epi_csv.get(i)[COL_TIMEH])*3600;
        time0 += (int) (Float.parseFloat(epi_csv.get(i)[COL_TIMEM])*60.0);
        for (int j=0; j<epi_csv.size(); j++) {
          if (epi_csv.get(j)[COL_EVENT].equals("I")) {
            if (epi_csv.get(i)[COL_VICTIM].equals(epi_csv.get(j)[COL_INFBY])) {
              int time1 = Integer.parseInt(epi_csv.get(j)[COL_TIMEH])*3600;
              time1 += (int) (Float.parseFloat(epi_csv.get(j)[COL_TIMEM])*60.0);
              gts.add(time1-time0);
              largest_gt=Math.max(largest_gt,  time1- time0);
            }
          }
        }
      }
    }
    Collections.sort(gts);
    Font titleFont = new Font("Calibri", Font.BOLD, 32);
    Font subTitleFont = new Font("Calibri", Font.PLAIN, 20);
    Font axisFont = new Font("Calibri", Font.PLAIN, 14);
    Graphics2D g = getNiceGraphics(bi);
    FontMetrics fm = g.getFontMetrics();
    g.setColor(java.awt.Color.WHITE);
    g.fillRect(0, 0, bi.getWidth(), bi.getHeight());
    g.setFont(titleFont);
    fm = g.getFontMetrics();
    int left_of_title = (int) ((bi.getWidth() - fm.stringWidth(L.getText("GenTimeGraph")))/2);
    int bottom_of_title = (top_margin / 2) - 10;
    g.setColor(java.awt.Color.BLACK);
    g.drawString(L.getText("GenTimeGraph"), left_of_title, bottom_of_title);
    int ybase = bi.getHeight() - bottom_margin;
    g.drawLine(left_margin, ybase, bi.getWidth() - right_margin, ybase);
    int step_size = Math.max(1, (int) (largest_gt / 720));
    step_size *= 60;
    int m = 0;
    g.setFont(subTitleFont);
    fm = g.getFontMetrics();
    g.drawString(L.getText("Minutes"), (bi.getWidth() - fm.stringWidth(L.getText("Minutes"))) / 2, bi.getHeight() - (bottom_margin / 6));

    Graphics2D grot = (Graphics2D) g.create();
    grot.rotate(-Math.PI / 2);
    fm = grot.getFontMetrics();
    grot.drawString(L.getText("PCOfInfections"), -(bi.getHeight() / 2) - (fm.stringWidth(L.getText("PCOfInfections")) / 2), left_margin / 3);
    grot.dispose();
    g.setColor(new java.awt.Color(220, 220, 50));
    Polygon p = new Polygon();
    for (int i = 0; i < gts.size(); i++) {
      int ypix = (bi.getHeight() - bottom_margin) - (int) ((bi.getHeight() - (top_margin + bottom_margin)) * ((i + 1) / (double) gts.size()));
      int xpix = left_margin + (int) ((gts.get(i) / (double) largest_gt) * (bi.getWidth() - (right_margin + left_margin)));
      // int cc=(int) (255*i/(double)a_gentime.length);
      p.addPoint(xpix, ypix);
    }
    p.addPoint(left_margin + (int) ((bi.getWidth() - (right_margin + left_margin))), bi.getHeight() - bottom_margin);
    p.addPoint(left_margin, bi.getHeight() - bottom_margin);
    g.fillPolygon(p);
    g.setColor(java.awt.Color.BLUE);
    for (int i = 0; i < p.npoints - 2; i++) {
      g.fillOval(p.xpoints[i], p.ypoints[i], 3, 3);
    }
    g.setFont(axisFont);
    fm = g.getFontMetrics();
    m = 0;
    Graphics2D g2 = (Graphics2D) g.create();
    g2.setStroke(new BasicStroke(1.0f, BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER, 10.0f, new float[] { 5.0f }, 0.0f));
    g2.setColor(java.awt.Color.DARK_GRAY);
    while (m < largest_gt) {
      int xpix = left_margin + (int) ((m / (double) largest_gt) * (bi.getWidth() - (right_margin + left_margin)));
      g.setColor(java.awt.Color.BLACK);
      g.drawLine(xpix, ybase, xpix, ybase + 5);
      g.drawString(String.valueOf(m / 60), xpix - (fm.stringWidth(String.valueOf(m / 60)) / 2), ybase + 20);
      g2.drawLine(xpix, ybase, xpix, top_margin);
      m += step_size;
    }

    g.setColor(java.awt.Color.BLACK);
    g.drawLine(left_margin, bi.getHeight() - bottom_margin, left_margin, top_margin);

    for (int i = 0; i <= 10; i++) {
      int ypix = (int) ((bi.getHeight() - bottom_margin) - ((i / 10.0) * (bi.getHeight() - (bottom_margin + top_margin))));
      g.drawLine(left_margin, ypix, left_margin - 3, ypix);
      g2.drawLine(left_margin + 1, ypix, bi.getWidth() - right_margin, ypix);
      g.drawString(String.valueOf(i * 10), left_margin - 12 - fm.stringWidth(String.valueOf(i * 10)), ypix + 5);

    }

    g2.dispose();
    g.dispose();
    SwingFXUtils.toFXImage(layer, fx_img);
  }

  public void generateR0Graph(BufferedImage bi, boolean include_unconfirmed) {
    
    // Calculate potential_r0 - assume every contact results in an infection...
    
    ArrayList<Integer> potential_r0 = new ArrayList<Integer>();
    for (int i=0; i<epi_csv.size(); i++) {
      if (epi_csv.get(i)[COL_EVENT].equals("I")) {
        int cons = Integer.parseInt(epi_csv.get(i)[COL_NCONS]);
        while (potential_r0.size()<(1+cons)) potential_r0.add(0);
        potential_r0.set(cons, potential_r0.get(cons)+1);
      }
    }
    
    // Calculate effective_r0 - for each infection, count actual infections made...
    
    ArrayList<Integer> effective_r0 = new ArrayList<Integer>();
    
    for (int i=0; i<epi_csv.size(); i++) {
      if (epi_csv.get(i)[COL_EVENT].equals("I")) {
        int count_infs = 0;
        int infector_id = Integer.parseInt(epi_csv.get(i)[COL_VICTIM]);
        for (int j=0; j<epi_csv.size(); j++) {
          if (epi_csv.get(j)[COL_EVENT].equals("I")) {
            if (!epi_csv.get(j)[COL_INFBY].equals("NA")) {
              if (Integer.parseInt(epi_csv.get(j)[COL_INFBY])==infector_id) {
                count_infs++;
              }
            }
          }
        }
        while (effective_r0.size()<(1+count_infs)) effective_r0.add(0);
        effective_r0.set(count_infs, effective_r0.get(count_infs)+1);
      }
      
    }
    
    Font titleFont = new Font("Calibri", Font.BOLD, 32);
    Font subTitleFont = new Font("Calibri", Font.PLAIN, 20);
    Font axisFont = new Font("Calibri", Font.PLAIN, 14);
    Graphics2D g = getNiceGraphics(bi);
    FontMetrics fm = g.getFontMetrics();
    g.setColor(java.awt.Color.WHITE);
    g.fillRect(0, 0, bi.getWidth(), bi.getHeight());
    g.setFont(titleFont);
    g.setColor(java.awt.Color.BLACK);
    g.drawLine(left_margin, (bi.getHeight() - bottom_margin) + 5, bi.getWidth() - right_margin, (bi.getHeight() - bottom_margin) + 5);
    g.drawLine(left_margin - 5, bi.getHeight() - bottom_margin, left_margin - 5, top_margin);
    int lgth = effective_r0.size();
    if (lgth==0) lgth=1;
    int step_pix = (int) Math.floor(((bi.getWidth() - left_margin) - right_margin) / lgth);
    fm = g.getFontMetrics();
    int left_of_title = (int) ((bi.getWidth() - fm.stringWidth(L.getText("R0Graph"))) / 2);
    int right_of_title = left_of_title + fm.stringWidth(L.getText("R0Graph"));
    int bottom_of_title = (top_margin / 2) - 10;
    g.drawString(L.getText("R0Graph"), left_of_title, bottom_of_title);
    g.setFont(subTitleFont);
    Graphics2D grot = (Graphics2D) g.create();
    grot.rotate(-Math.PI / 2);
    fm = grot.getFontMetrics();
    grot.drawString(L.getText("HowMany"), -(bi.getHeight() / 2) - (fm.stringWidth(L.getText("HowMany"))/ 2), left_margin / 3);
    grot.dispose();
    fm = g.getFontMetrics();
    g.drawString(L.getText("Number_infections"), (bi.getWidth() - fm.stringWidth(L.getText("Number_infections"))) / 2, bi.getHeight() - (bottom_margin / 6));
    if (include_unconfirmed) {
      bottom_of_title += 15;
      g.setColor(java.awt.Color.RED);
      g.fillRect(left_of_title, bottom_of_title, 20, 20);
      g.setColor(java.awt.Color.BLACK);
      g.drawRect(left_of_title, bottom_of_title, 20, 20);
      g.drawString(L.getText("Effective_R0"), left_of_title + 30, bottom_of_title + 18);
      g.setColor(java.awt.Color.LIGHT_GRAY);
      g.fillRect(right_of_title - 20, bottom_of_title, 20, 20);
      g.setColor(java.awt.Color.BLACK);
      g.drawRect(right_of_title - 20, bottom_of_title, 20, 20);
      g.drawString(L.getText("Theoretical_R0"), (right_of_title - 30) - fm.stringWidth(L.getText("Theoretical_R0")), bottom_of_title + 18);
    }
    g.setFont(axisFont);
    fm = g.getFontMetrics();

    int max_y = 0;
    int min_y = 0;
    int max_r0 = Math.max(potential_r0.size(), effective_r0.size());
    if (max_r0 > 0) {
      if (include_unconfirmed) {
        max_y = Math.max(potential_r0.get(0), effective_r0.get(0));
        min_y = Math.min(potential_r0.get(0), effective_r0.get(0));
        for (int i = 1; i < potential_r0.size(); i++) {
          max_y = Math.max(Math.max(potential_r0.get(i), effective_r0.get(i)), max_y);
          min_y = Math.min(Math.min(potential_r0.get(i), effective_r0.get(i)), min_y);
        }
      } else {
        max_y = effective_r0.get(0);
        min_y = effective_r0.get(0);
        for (int i = 1; i < effective_r0.size(); i++) {
          max_y = Math.max(effective_r0.get(i), max_y);
          min_y = Math.min(effective_r0.get(i), min_y);
        }
      }

    }
    min_y = 0; // Actually... use 0 as the minimum...
    if (max_y == 0) max_y = 1;
    double scale_y = (bi.getHeight() - (bottom_margin + top_margin)) / (max_y - min_y);

    if (epi_csv.size() > 0) {
      for (int i = 0; i < effective_r0.size(); i++) {
        int mid = left_margin + (i * step_pix) + (step_pix / 2);
        g.drawString(String.valueOf(i), mid - (fm.stringWidth(String.valueOf(i)) / 2), 25 + (bi.getHeight() - bottom_margin));
        g.drawLine(left_margin + (i * step_pix), (bi.getHeight() - bottom_margin) + 5, left_margin + (i * step_pix), (bi.getHeight() - bottom_margin) + 8);
        if (include_unconfirmed) {
          g.setColor(java.awt.Color.LIGHT_GRAY);
          g.fillRect((int) (left_margin + ((i + 0.5) * step_pix)), (int) (bi.getHeight() - (bottom_margin + (scale_y * (potential_r0.get(i) - min_y)))), step_pix / 4, (int) ((scale_y * (potential_r0.get(i) - min_y))));
          g.setColor(java.awt.Color.BLACK);
          g.drawRect((int) (left_margin + ((i + 0.5) * step_pix)), (int) (bi.getHeight() - (bottom_margin + (scale_y * (potential_r0.get(i) - min_y)))), step_pix / 4, (int) ((scale_y * (potential_r0.get(i) - min_y))));
          g.setColor(java.awt.Color.RED);
          g.fillRect((int) (left_margin + ((i + 0.25) * step_pix)), (int) (bi.getHeight() - (bottom_margin + (scale_y * (effective_r0.get(i) - min_y)))), step_pix / 4,  (int) ((scale_y * (effective_r0.get(i) - min_y))));
          g.setColor(java.awt.Color.BLACK);
          g.drawRect((int) (left_margin + ((i + 0.25) * step_pix)), (int) (bi.getHeight() - (bottom_margin + (scale_y * (effective_r0.get(i) - min_y)))), step_pix / 4,  (int) ((scale_y * (effective_r0.get(i) - min_y))));
        } else {
          g.setColor(java.awt.Color.RED);
          g.fillRect((int) (left_margin + (i * step_pix)), (int) (bi.getHeight() - (bottom_margin + (scale_y * (effective_r0.get(i) - min_y)))), step_pix, (int) ((scale_y * (effective_r0.get(i) - min_y))));
          g.setColor(java.awt.Color.BLACK);
          g.drawRect((int) (left_margin + (i * step_pix)), (int) (bi.getHeight() - (bottom_margin + (scale_y * (effective_r0.get(i) - min_y)))), step_pix, (int) ((scale_y * (effective_r0.get(i) - min_y))));
        }
      }

      int y_scale = (int) Math.round((max_y - min_y) / 6);
      if (y_scale < 1) y_scale = 1;

      for (int j = min_y; j <= max_y; j += y_scale) {
        int ypix = (int) (bi.getHeight() - (bottom_margin + (scale_y * (j - min_y))));
        g.drawLine(left_margin - 5, ypix, left_margin - 8, ypix);
        g.drawString(String.valueOf(j), left_margin - 12 - fm.stringWidth(String.valueOf(j)), ypix + 5);

      }
    }
    g.dispose();
    SwingFXUtils.toFXImage(layer, fx_img);
  }

  public void save(BufferedImage bi, String f) {
    try {
      ImageIO.write(bi, "PNG", new File(f));
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  public Graphics2D getNiceGraphics(BufferedImage bi) {
    Graphics2D g = (Graphics2D) bi.createGraphics();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
    g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
    g.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
    g.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS, RenderingHints.VALUE_FRACTIONALMETRICS_ON);
    g.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BICUBIC);
    g.setRenderingHint(RenderingHints.KEY_COLOR_RENDERING, RenderingHints.VALUE_COLOR_RENDER_QUALITY);
    g.setRenderingHint(RenderingHints.KEY_TEXT_LCD_CONTRAST, 140);
    g.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL, RenderingHints.VALUE_STROKE_NORMALIZE);

    return g;
  }

  public void playMovie(String ff) {
    File f = new File(ff);
    Media epiMedia = new Media(f.toURI().toString());
    epiPlayer = new MediaPlayer(epiMedia);
    
    epiMovie.setMediaPlayer(epiPlayer);
    epiImage.setVisible(false);
    epiMovie.setVisible(true);
    epiPlayer.play();
    epiPlayer.setOnEndOfMedia(new Runnable() {
      public void run() {
        jtimer.setInitialDelay(500);
        jtimer.start();
        epiPlayer.dispose();
      }
    });
  }
  
  public void addNetworkLabels(BufferedImage bi) {
    Graphics2D g2d = getNiceGraphics(bi);
    int w = bi.getWidth();
    w=(w-800)/2;
    int h = bi.getHeight();
    g2d.setFont(new Font("Arial",Font.PLAIN,28));
    g2d.setColor(java.awt.Color.BLACK);
    g2d.drawString(L.getText("Seed"),w+100, h-100);
    g2d.drawString(L.getText("Infector"), w+100,h-60);
    g2d.drawString(L.getText("Terminal"), w+100,h-20);
    g2d.drawOval(w+45,h-120,20,20);
    g2d.drawRect(w+45,h-80,20,20);
    g2d.drawLine(w+45, h-20,w+65,h-20);
    g2d.drawLine(w+45,h-20,w+55,h-40);
    g2d.drawLine(w+55,h-40,w+65,h-20);
    g2d.drawString(L.getText("Recent"), w+400, h-60);
    g2d.drawString(L.getText("Older"),w+400,h-20);
    g2d.drawOval(w+335,h-90,40,40);
    g2d.drawOval(w+345, h-40, 20, 20);
    g2d.dispose();
  }

  public void showImage(String ff, boolean network_labels) {
    try {
      BufferedImage bi = ImageIO.read(new File(ff));
      if (network_labels) addNetworkLabels(bi);
      int wid = bi.getWidth();
      int hei = bi.getHeight();

      double wid_ratio = screen.width / (double) wid;
      double hei_ratio = screen.height / (double) hei;
      double ratio = Math.min(wid_ratio, hei_ratio);
      double new_wid = wid * ratio;
      double new_hei = hei * ratio;
      int x = (int) ((screen.width - new_wid) / 2);
      int y = (int) ((screen.height - new_hei) / 2);
      Graphics2D g = getNiceGraphics(layer);
      g.setColor(java.awt.Color.WHITE);
      g.fillRect(0, 0, layer.getWidth(), layer.getHeight());
      g.drawImage(bi, x, y, x + (int) new_wid, y + (int) new_hei, 0, 0, wid, hei, null);
      bi = null;
      SwingFXUtils.toFXImage(layer, fx_img);

      epiImage.setVisible(true);
      epiMovie.setVisible(false);
      jtimer.setInitialDelay(500);
      jtimer.start();      
      g.dispose();
     } catch (Exception e) {
      e.printStackTrace();
    }
  }
  

  public void pause() {
    if (!epiMovie.isVisible()) {
      if (jtimer.isRunning()) {
        jtimer.stop();
        pauseButton.setVisible(true);
      } else {
        jtimer.start();
        pauseButton.setVisible(false);
      }
    } else {
      if (epiPlayer.getStatus()==MediaPlayer.Status.PAUSED) {
        epiPlayer.play();
      } else {
        epiPlayer.pause();
      }
    }
  }

  
  public void jump() {
    if (!epiMovie.isVisible()) {
      jtimer.stop();
      nextChapter();
    } else {
      epiPlayer.stop();
      nextChapter();
    }
  }
   
  
  public void refreshData() {
    epi_csv.clear();
    try {
      BufferedReader br = new BufferedReader(new FileReader(in_file));
      String s = br.readLine(); // Omit header
      while (s!=null) {
        epi_csv.add(s.toUpperCase().split(","));
        s=br.readLine();
      }
      br.close();
      
    } catch (Exception ex) {

    }
  }
  
  public void nextChapter() {
    jtimer.stop();
  
    String[] bits;
    String line = script.get(current_script_line).trim();
  
    if (line.toUpperCase().startsWith("WAIT")) {
      bits = line.split("\\s+");
      if (!pauseButton.isVisible()) {
        jtimer.setInitialDelay((int)(1000*Float.parseFloat(bits[1])));
        jtimer.start();
      }
    } else if (line.toUpperCase().startsWith("CASESGRAPH")) {
      refreshData();
      boolean unconfirmed = (line.toUpperCase().indexOf("UNCONFIRMED")>1);
      boolean cumulative = (line.toUpperCase().indexOf("CUMULATIVE")>1);
      generateCasesGraph(layer, unconfirmed, cumulative);
      jtimer.setInitialDelay(500);
      jtimer.start();
      
    } else if (line.toUpperCase().startsWith("R0GRAPH")) {
      refreshData();
      boolean unconfirmed = (line.toUpperCase().indexOf("UNCONFIRMED")>1);
      generateR0Graph(layer, unconfirmed);      
      jtimer.setInitialDelay(500);
      jtimer.start();
      
    } else if (line.toUpperCase().equals("GENTIMEGRAPH")) {
      refreshData();
      generateGenTimeGraph(layer);
      jtimer.setInitialDelay(500);
      jtimer.start();
      
    } else if ((line.toUpperCase().startsWith("MOVIE[")) || (line.toUpperCase().startsWith("IMAGE["))) {
      boolean movie = line.toUpperCase().startsWith("MOVIE[");
      line=line.substring(6);
      line=line.substring(0, line.length()-1);
      bits=line.split(",");
      String themovie = bits[script_indexes[current_script_line]];
      script_indexes[current_script_line]++;
      if (script_indexes[current_script_line]>=bits.length) script_indexes[current_script_line]=0;
      if (themovie.startsWith("\"")) themovie=themovie.substring(1);
      if (themovie.endsWith("\"")) themovie=themovie.substring(0, themovie.length()-1);
      if (movie) playMovie(themovie); 
      else showImage(themovie,false);
      
      
    } else if (line.toUpperCase().startsWith("NETWORKGRAPH")) {
      refreshData();
      File f = new File("staticnetworkplot.png");
      System.out.println(f.getPath());
      if (f.length()>0) showImage("staticnetworkplot.png",true);
      Runtime rt = Runtime.getRuntime();
      bits = new String[] {RScript,RNetGraph,in_file,String.valueOf(screen.width),String.valueOf(screen.height),div_col1, div_col2, div_col3, "staticnetworkplot.png"};
      try {
        rt.exec(bits);
      } catch (Exception e) { e.printStackTrace(); }
      jtimer.setInitialDelay(500);
      jtimer.start();
      
    }
    current_script_line++;
    if (current_script_line>=script.size()) current_script_line=0;
    
  }
 
  public static void main(String[] args) {
    launch(args);
  }
}
