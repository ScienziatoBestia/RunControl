#include "RunControl.h"

#define nMon 6
DaqSharedMemory* rcDaqSharedMemory;

GtkWidget *sbNEvents, *sbTime,  *sbAlfaScaler, *sbRate;  // spin buttton
GtkWidget *sbComptonV1742Ch, *sbComptonV1742Gr, *sbComptonDT5780Ch, *sbGCALV1742Ch, *sbGCALV1742Gr, *sbDT5743Ch, *sbDT5743Gr, *sbComptonDT5780Ch, *sbCSPECplotScaler, *sbGCALplotScaler, *sbNRSSplotScaler; 
GtkWidget *eFileName, *eRunNumber; // entry text
GtkWidget *eMonitorProd[nMon], *eMonitorCons[nMon], *eCSPECrate, *eGCALrate, *eNRSSrate;
GtkWidget *tMonitor; // table
GtkWidget *bConfigure, *bAdvancedSetup, *bStartDaq, *bStopDaq, *bStartSlowControl, *bQuit; //buttons
GtkWidget *rTime, *rNEvents, *rGCALV1742Ch, *rGCALV1742Gr , *rBaFV1742Ch, *rBaFV1742Gr, *rDT5743Ch, *rDT5743Gr; // radio buttons
GtkWidget *cConsumer, *cCSPECplotter, *cGCALplotter, *cNRSSplotter, *cMonitor, *cSoftTrg, *cTestHPGe, *cBaF, *cHPGe, *cSiStrip, *cNRSS, *cGCAL; //check buttons
GtkWidget *cbRecipeList; //combo box
GtkWidget *pbar;
GtkTextIter iter;
GtkTextBuffer *buffer;

char currentDir[100], defaultDir[100], defaultFile[100];
int connectionParams[4];
uint32_t dcOffset[MaxDT5743NChannels];
int triggerLevel[MaxDT5743NChannels];
int V1742Ready=0, DT5780Ready=0, V1495Ready=0, V1742GCALReady=0, DT5743Ready=0;
int running=0;
int ret;
char logFilename[100];
char timeNow[22];
char text[100];
char nameRecipe[100];

int main(int argc, char *argv[]) {

  ret = CheckRunningProcesses(RunControl);
  //printf("ret %i \n",ret);
  if(ret) exit(1);

  GtkWidget* main_window;

  gtk_init(&argc, &argv);
  main_window = create_main_window();
  gtk_widget_show_all(main_window);
  g_signal_connect(main_window, "destroy", G_CALLBACK(Quit), NULL);

  sprintf(text,"\nRunControl: Configuring the DAQ shared memory\n");
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text); 
  rcDaqSharedMemory=configDaqSharedMemory("RunControl");

  updateRunNumber(0);

  gtk_main();
  
  return 0;

}


/*************************************************************************************************************/
GtkWidget* create_main_window() {
/*************************************************************************************************************/
  getcwd(currentDir, sizeof(currentDir));

  GdkColor color; // 16-bit are used. Multiply RGB values (range 1-255) by 65535/255=257
  color.red = 0xD8D8;
  color.green = 0xBFBF;
  color.blue = 0xD8D8;

  GtkWidget *lFileName, *lRunNumber, *lAlfaScaler, *lRate; // labels

  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "RUN CONTROL");
  gtk_window_set_default_size(GTK_WINDOW(window), 1520, 800);
  gtk_container_set_border_width(GTK_CONTAINER(window), 20);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);

  GtkWidget* fixed = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(window), fixed);

  int xframe,yframe,xbsize,ybsize;

  /////////////////////////////////////////////////////////////////////
  // Device configuration frame
  /////////////////////////////////////////////////////////////////////
  xframe=10;  yframe=10;  xbsize=150;  ybsize=40;
  GtkWidget* frameConfig = gtk_frame_new ("Device Config");
  gtk_fixed_put(GTK_FIXED(fixed), frameConfig, xframe,yframe);
  gtk_widget_set_size_request (frameConfig, 600, 130);

  cbRecipeList=gtk_combo_box_text_new();
  gtk_widget_set_size_request (cbRecipeList, 300, ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), cbRecipeList,xframe+120 , yframe+30 );
  gtk_combo_box_set_focus_on_click(GTK_COMBO_BOX(cbRecipeList),1);
  gtk_widget_show (cbRecipeList);
  FillRecipeList(cbRecipeList, "Default",1);
  GtkWidget* lRecipes=gtk_label_new("RECIPE NAME");
  gtk_fixed_put(GTK_FIXED(fixed), lRecipes, xframe+10, yframe+40);
  g_signal_connect(G_OBJECT(cbRecipeList), "set-focus-child", G_CALLBACK(loadRecipeList), NULL);
  g_signal_connect(G_OBJECT(cbRecipeList), "changed", G_CALLBACK(changeRecipe), NULL);

  bConfigure=gtk_button_new_with_label("Configure Devices");
  gtk_widget_set_size_request(bConfigure, xbsize, ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bConfigure, xframe+120, yframe+80);
  g_signal_connect(G_OBJECT(bConfigure), "clicked", G_CALLBACK(ConfigDigi), NULL);

  bAdvancedSetup=gtk_button_new_with_label("Advanced Setup");
  gtk_widget_set_size_request(bAdvancedSetup, xbsize, ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bAdvancedSetup, xframe+430, yframe+80);
  g_signal_connect(G_OBJECT(bAdvancedSetup), "clicked", G_CALLBACK(openAdvancedSetup), NULL);


  /////////////////////////////////////////////////////////////////////
  // Device selection frame
  /////////////////////////////////////////////////////////////////////
  xframe=10;   yframe=160;
  GtkWidget *frameDevices = gtk_frame_new ("Device Selection");
  gtk_fixed_put(GTK_FIXED(fixed), frameDevices, xframe,yframe);
  gtk_widget_set_size_request (frameDevices, 600, 140);

  cBaF = gtk_check_button_new_with_label ("BaF");
  gtk_fixed_put(GTK_FIXED(fixed), cBaF, xframe+30, yframe+20);
  gtk_widget_set_sensitive(cBaF, FALSE);
  g_signal_connect(G_OBJECT(cBaF), "toggled", G_CALLBACK(checkIncludedDevice), NULL);

  cHPGe = gtk_check_button_new_with_label("HPGe");
  gtk_fixed_put(GTK_FIXED(fixed), cHPGe, xframe+30, yframe+50);
  gtk_widget_set_sensitive(cHPGe, FALSE);
  g_signal_connect(G_OBJECT(cHPGe), "toggled", G_CALLBACK(checkIncludedDevice), NULL);

  cSiStrip = gtk_check_button_new_with_label ("SiStrip");
  gtk_fixed_put(GTK_FIXED(fixed), cSiStrip, xframe+30, yframe+80);
  gtk_widget_set_sensitive(cSiStrip, FALSE);
  g_signal_connect(G_OBJECT(cSiStrip), "toggled", G_CALLBACK(checkIncludedDevice), NULL);

  sbAlfaScaler = gtk_spin_button_new_with_range(1,5000,10);
  gtk_fixed_put(GTK_FIXED(fixed), sbAlfaScaler, xframe+210, yframe+20);
  lAlfaScaler = gtk_label_new("Alpha Scaler");
  gtk_fixed_put(GTK_FIXED(fixed), lAlfaScaler, 140, 185);

  cGCAL = gtk_check_button_new_with_label("GCAL"); 
  gtk_fixed_put(GTK_FIXED(fixed), cGCAL, xframe+340, yframe+20);
  gtk_widget_set_sensitive(cGCAL, FALSE);
  g_signal_connect(G_OBJECT(cGCAL), "toggled", G_CALLBACK(checkIncludedDevice), NULL);

  cNRSS = gtk_check_button_new_with_label("NRSS"); 
  gtk_fixed_put(GTK_FIXED(fixed), cNRSS, xframe+440, yframe+20);
  gtk_widget_set_sensitive(cNRSS, FALSE);
  g_signal_connect(G_OBJECT(cNRSS), "toggled", G_CALLBACK(checkIncludedDevice), NULL);

  /////////////////////////////////////////////////////////////////////
  // Run configuration frame
  /////////////////////////////////////////////////////////////////////
  xframe=10;   yframe=320;
  GtkWidget *frameRunConfig = gtk_frame_new ("Run Config");
  gtk_fixed_put(GTK_FIXED(fixed), frameRunConfig, xframe,yframe);
  gtk_widget_set_size_request (frameRunConfig, 600, 170);

  rTime = gtk_radio_button_new_with_label(NULL,"Time (s)");
  gtk_fixed_put(GTK_FIXED(fixed), rTime, xframe+10, yframe+20);
  rNEvents = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (rTime),"N Events");
  gtk_fixed_put(GTK_FIXED(fixed), rNEvents, xframe+10, yframe+60);
  g_signal_connect (GTK_TOGGLE_BUTTON (rTime), "toggled",G_CALLBACK (selectRunType), NULL);

  sbTime = gtk_spin_button_new_with_range(0, 1000000,1000);
  gtk_spin_button_set_value((gpointer) sbTime, 3600);
  gtk_fixed_put(GTK_FIXED(fixed), sbTime, xframe+110, yframe+20);

  sbNEvents = gtk_spin_button_new_with_range(0, 10000000,1000);
  gtk_spin_button_set_value((gpointer) sbNEvents, 100000);
  gtk_fixed_put(GTK_FIXED(fixed), sbNEvents, xframe+110, yframe+60);
  gtk_widget_set_sensitive(sbNEvents, FALSE);

  eFileName = gtk_entry_new();
  gtk_fixed_put(GTK_FIXED(fixed), eFileName, xframe+90, yframe+120);
  gtk_widget_set_size_request(eFileName, 500, 30);
  lFileName =  gtk_label_new("File Name");
  gtk_fixed_put(GTK_FIXED(fixed), lFileName, xframe+10, yframe+125);
  //strcat(defaultFile,currentDir);
  //strcat(defaultFile,"Data/");
  gtk_entry_set_text((gpointer) eFileName,defaultFile);
  gtk_entry_set_editable(GTK_ENTRY(eFileName), FALSE);
  gtk_widget_set_can_focus(GTK_WIDGET(eFileName), FALSE);


  cSoftTrg = gtk_check_button_new_with_label("Software Trigger"); 
  gtk_fixed_put(GTK_FIXED(fixed), cSoftTrg, xframe+290, yframe+60);
 
  cConsumer = gtk_check_button_new_with_label("Consumer");
  gtk_fixed_put(GTK_FIXED(fixed), cConsumer, xframe+290, yframe+20);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (cConsumer),1);

  cTestHPGe = gtk_check_button_new_with_label("Pulser HPGe"); 
  gtk_fixed_put(GTK_FIXED(fixed), cTestHPGe, xframe+450, yframe+20);
  gtk_widget_set_sensitive(cTestHPGe,FALSE);
  g_signal_connect(G_OBJECT(cTestHPGe), "toggled", G_CALLBACK(CheckTestHPGe), NULL);

  sbRate = gtk_spin_button_new_with_range(0, 500,10);
  gtk_spin_button_set_value((gpointer) sbRate, 10);
  gtk_fixed_put(GTK_FIXED(fixed), sbRate, xframe+520, yframe+60);
  lRate =  gtk_label_new("Rate (Hz)");
  gtk_fixed_put(GTK_FIXED(fixed), lRate, xframe+450, yframe+65);
  gtk_signal_connect (GTK_OBJECT (sbRate), "value_changed", G_CALLBACK(updateRate),NULL);



  /////////////////////////////////////////////////////////////////////
  // Scrolled window for the logbook
  /////////////////////////////////////////////////////////////////////
  GtkWidget *scrolled_window = gtk_scrolled_window_new( NULL, NULL );
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_widget_set_size_request(scrolled_window,600,300);

  GtkWidget *textview = gtk_text_view_new();
  buffer= gtk_text_view_get_buffer(GTK_TEXT_VIEW (textview));
  gtk_text_buffer_get_start_iter(buffer, &iter);
  gtk_text_buffer_create_tag(buffer,"lmarg","left_margin",5,NULL);
  gtk_text_buffer_create_tag(buffer,"blue_fg","foreground","blue",NULL);
  gtk_text_buffer_create_tag(buffer,"green_fg","foreground","green",NULL);
  gtk_text_buffer_create_tag(buffer,"red_bg","background","red",NULL); 
  gtk_text_buffer_create_tag(buffer,"blue_bg","background","blue",NULL);
  gtk_text_buffer_create_tag(buffer,"yellow_bg","background","yellow",NULL);

  gtk_container_add(GTK_CONTAINER (scrolled_window), textview);
  gtk_fixed_put(GTK_FIXED(fixed), scrolled_window, 10, 530);


  /////////////////////////////////////////////////////////////////////
  // CSPEC plotter frame
  /////////////////////////////////////////////////////////////////////
  xframe=650;
  yframe=160;
  GtkWidget *frameCSPECPlotter = gtk_frame_new ("CSPEC Plotter");
  gtk_fixed_put(GTK_FIXED(fixed), frameCSPECPlotter, xframe,yframe);
  gtk_widget_set_size_request (frameCSPECPlotter, 315, 140);

  cCSPECplotter = gtk_check_button_new_with_label("Plotter Enable");
  gtk_fixed_put(GTK_FIXED(fixed), cCSPECplotter, xframe+20, yframe+30);
  gtk_widget_set_sensitive(cCSPECplotter, FALSE);
  g_signal_connect(G_OBJECT(cCSPECplotter), "toggled", G_CALLBACK(checkCSPECplotter), NULL);

  sbComptonV1742Ch = gtk_spin_button_new_with_range (0,36,1);
  gtk_fixed_put(GTK_FIXED(fixed), sbComptonV1742Ch, xframe+100, yframe+80);
  gtk_signal_connect (GTK_OBJECT (sbComptonV1742Ch), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);
 
  sbComptonV1742Gr = gtk_spin_button_new_with_range (0,3,1);
  gtk_fixed_put(GTK_FIXED(fixed), sbComptonV1742Gr, xframe+100, yframe+110);
  gtk_signal_connect (GTK_OBJECT (sbComptonV1742Gr), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);

  GtkWidget *lComptonV1742Ch = gtk_label_new("V1742");
  gtk_fixed_put(GTK_FIXED(fixed), lComptonV1742Ch, xframe+20, yframe+60);

  rBaFV1742Ch = gtk_radio_button_new_with_label(NULL,"Channel");
  gtk_fixed_put(GTK_FIXED(fixed), rBaFV1742Ch, xframe+20, yframe+80);
  rBaFV1742Gr = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (rBaFV1742Ch),"Group");
  gtk_fixed_put(GTK_FIXED(fixed), rBaFV1742Gr, xframe+20, yframe+110);
  g_signal_connect (GTK_TOGGLE_BUTTON (rBaFV1742Ch), "toggled", G_CALLBACK (selectPlotType), NULL);


  sbComptonDT5780Ch = gtk_spin_button_new_with_range (0,1,1);
  gtk_fixed_put(GTK_FIXED(fixed), sbComptonDT5780Ch, xframe+240, yframe+80);
  gtk_signal_connect (GTK_OBJECT (sbComptonDT5780Ch), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);

  GtkWidget *lComptonDT5780 = gtk_label_new("DT5780");
  gtk_fixed_put(GTK_FIXED(fixed), lComptonDT5780, xframe+180, yframe+60);
  GtkWidget *lComptonDT5780Ch = gtk_label_new("Channel");
  gtk_fixed_put(GTK_FIXED(fixed), lComptonDT5780Ch, xframe+180, yframe+80);


  sbCSPECplotScaler = gtk_spin_button_new_with_range(1,1000,10);
  gtk_fixed_put(GTK_FIXED(fixed), sbCSPECplotScaler, xframe+240, yframe+25);
  gtk_spin_button_set_value((gpointer) sbCSPECplotScaler, 100);
  gtk_signal_connect (GTK_OBJECT (sbCSPECplotScaler), "value_changed",G_CALLBACK (updatePlotScaler),NULL);

  GtkWidget *lCSPECPlotScaler = gtk_label_new("Plot Scaler");
  gtk_fixed_put(GTK_FIXED(fixed), lCSPECPlotScaler, xframe+160, yframe+30);


  /////////////////////////////////////////////////////////////////////
  // GCAL plotter frame
  /////////////////////////////////////////////////////////////////////
  xframe=975;
  yframe=160;
  GtkWidget *frameGCALPlotter = gtk_frame_new ("GCAL Plotter");
  gtk_fixed_put(GTK_FIXED(fixed), frameGCALPlotter, xframe,yframe);
  gtk_widget_set_size_request (frameGCALPlotter, 315, 140);

  cGCALplotter = gtk_check_button_new_with_label("Plotter Enable");
  gtk_widget_set_sensitive(cGCALplotter, FALSE);
  gtk_fixed_put(GTK_FIXED(fixed), cGCALplotter, xframe+20, yframe+30);
  g_signal_connect(G_OBJECT(cGCALplotter), "toggled", G_CALLBACK(checkGCALplotter), NULL);

  sbGCALV1742Ch = gtk_spin_button_new_with_range (0,36,1);
  gtk_fixed_put(GTK_FIXED(fixed), sbGCALV1742Ch, xframe+100, yframe+80);
  gtk_signal_connect (GTK_OBJECT (sbGCALV1742Ch), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);

  sbGCALV1742Gr = gtk_spin_button_new_with_range (0,3,1);
  gtk_fixed_put(GTK_FIXED(fixed), sbGCALV1742Gr, xframe+100, yframe+110);
  gtk_signal_connect (GTK_OBJECT (sbGCALV1742Gr), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);

  GtkWidget *lGCALV1742Ch = gtk_label_new("V1742");
  gtk_fixed_put(GTK_FIXED(fixed), lGCALV1742Ch, xframe+20, yframe+60);

  rGCALV1742Ch = gtk_radio_button_new_with_label(NULL,"Channel");
  gtk_fixed_put(GTK_FIXED(fixed), rGCALV1742Ch, xframe+20, yframe+80);
  rGCALV1742Gr = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (rGCALV1742Ch),"Group");
  gtk_fixed_put(GTK_FIXED(fixed), rGCALV1742Gr, xframe+20, yframe+110);
  g_signal_connect (GTK_TOGGLE_BUTTON (rGCALV1742Ch), "toggled",G_CALLBACK (selectPlotType), NULL);

  sbGCALplotScaler = gtk_spin_button_new_with_range(1,1000,10);
  gtk_fixed_put(GTK_FIXED(fixed), sbGCALplotScaler, xframe+240, yframe+25);
  gtk_spin_button_set_value((gpointer) sbGCALplotScaler, 100);
  gtk_signal_connect (GTK_OBJECT (sbGCALplotScaler), "value_changed", G_CALLBACK (updatePlotScaler),NULL);

  GtkWidget *lGCALplotScaler = gtk_label_new("Plot Scaler");
  gtk_fixed_put(GTK_FIXED(fixed), lGCALplotScaler, xframe+160, yframe+30);


  /////////////////////////////////////////////////////////////////////
  // NRSS plotter frame
  /////////////////////////////////////////////////////////////////////
  xframe=1300;
  yframe=160;
  GtkWidget *frameNRSSplotter = gtk_frame_new ("NRSS Plotter");
  gtk_fixed_put(GTK_FIXED(fixed), frameNRSSplotter, xframe,yframe);
  gtk_widget_set_size_request (frameNRSSplotter, 315, 140);

  cNRSSplotter = gtk_check_button_new_with_label("Plotter Enable");
  gtk_widget_set_sensitive(cNRSSplotter, FALSE);
  gtk_fixed_put(GTK_FIXED(fixed), cNRSSplotter, xframe+20, yframe+30);
  g_signal_connect(G_OBJECT(cNRSSplotter), "toggled", G_CALLBACK(checkNRSSplotter), NULL);

  sbDT5743Ch = gtk_spin_button_new_with_range (0,7,0);
  gtk_fixed_put(GTK_FIXED(fixed), sbDT5743Ch, xframe+100, yframe+80);
  gtk_signal_connect (GTK_OBJECT (sbDT5743Ch), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);
  
  sbDT5743Gr = gtk_spin_button_new_with_range (0,3,1);
  gtk_fixed_put(GTK_FIXED(fixed), sbDT5743Gr, xframe+100, yframe+110);
  gtk_signal_connect (GTK_OBJECT (sbDT5743Gr), "value_changed", G_CALLBACK(updateChannelToPlot),NULL);
  
  GtkWidget *lDT5743Ch = gtk_label_new("DT5743");
  gtk_fixed_put(GTK_FIXED(fixed), lDT5743Ch, xframe+20, yframe+60);

  rDT5743Ch = gtk_radio_button_new_with_label(NULL,"Channel");
  gtk_fixed_put(GTK_FIXED(fixed), rDT5743Ch, xframe+20, yframe+80);

  rDT5743Gr = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (rDT5743Ch),"Group");
  gtk_fixed_put(GTK_FIXED(fixed), rDT5743Gr, xframe+20, yframe+110);
  g_signal_connect (GTK_TOGGLE_BUTTON (rDT5743Ch), "toggled",G_CALLBACK (selectPlotType), NULL);

  sbNRSSplotScaler = gtk_spin_button_new_with_range(1,1000,10);
  gtk_fixed_put(GTK_FIXED(fixed), sbNRSSplotScaler, xframe+240, yframe+25);
  gtk_spin_button_set_value((gpointer) sbNRSSplotScaler, 100);

  GtkWidget *lNRSSplotScaler = gtk_label_new("Plot Scaler");
  gtk_fixed_put(GTK_FIXED(fixed), lNRSSplotScaler, xframe+160, yframe+30);
  gtk_signal_connect (GTK_OBJECT (sbNRSSplotScaler), "value_changed", G_CALLBACK (updatePlotScaler),NULL);


  /////////////////////////////////////////////////////////////////////
  // Monitor frame
  /////////////////////////////////////////////////////////////////////
  xframe=650;
  yframe=320;
  GtkWidget *frameMonitor = gtk_frame_new ("Monitoring");
  gtk_fixed_put(GTK_FIXED(fixed), frameMonitor, xframe,yframe);
  gtk_widget_set_size_request (frameMonitor, 740, 230);
  
  tMonitor=gtk_table_new (nMon,2,TRUE);
  gtk_fixed_put(GTK_FIXED(fixed), tMonitor, xframe+100,yframe+100);
  int et=0;
  for(et=0;et<nMon;et++) {
    eMonitorProd[et] = gtk_entry_new();
    gtk_widget_set_size_request(eMonitorProd[et], 100, 25);
    gtk_table_attach_defaults (GTK_TABLE (tMonitor), eMonitorProd[et], et, et+1, 0, 1);
    eMonitorCons[et] = gtk_entry_new();
    gtk_widget_set_size_request(eMonitorCons[et], 100, 25);
    gtk_table_attach_defaults (GTK_TABLE (tMonitor), eMonitorCons[et], et, et+1, 1, 2);
  }
  GtkWidget* lProd = gtk_label_new("Producer");
  gtk_fixed_put(GTK_FIXED(fixed), lProd, xframe+10, yframe+105);
  GtkWidget* lCons = gtk_label_new("Consumer");
  gtk_fixed_put(GTK_FIXED(fixed), lCons, xframe+10, yframe+125);
  GtkWidget* lBaFGamma = gtk_label_new("BaF Gamma");
  gtk_fixed_put(GTK_FIXED(fixed), lBaFGamma, xframe+100, yframe+80);
  GtkWidget* lBaFAlfa = gtk_label_new("BaF Alfa");
  gtk_fixed_put(GTK_FIXED(fixed), lBaFAlfa, xframe+200, yframe+80);
  GtkWidget* lHPGe = gtk_label_new("HPGe");
  gtk_fixed_put(GTK_FIXED(fixed), lHPGe, xframe+300, yframe+80);
  GtkWidget* lSiStrip = gtk_label_new("SiStrip");
  gtk_fixed_put(GTK_FIXED(fixed), lSiStrip, xframe+400, yframe+80);
  GtkWidget* lGCAL = gtk_label_new("GCAL");
  gtk_fixed_put(GTK_FIXED(fixed), lGCAL, xframe+500, yframe+80);
  GtkWidget* lNRSS = gtk_label_new("NRSS");
  gtk_fixed_put(GTK_FIXED(fixed), lNRSS, xframe+600, yframe+80);

  cMonitor = gtk_check_button_new_with_label("Monitor Enable");
  gtk_fixed_put(GTK_FIXED(fixed), cMonitor, xframe+20, yframe+40);

  GtkWidget* lCSPECrate = gtk_label_new("CSPEC Rate");
  gtk_fixed_put(GTK_FIXED(fixed), lCSPECrate, 700, 490);
  eCSPECrate = gtk_entry_new();
  gtk_widget_set_size_request(eCSPECrate, 60, 25);
  gtk_fixed_put(GTK_FIXED(fixed),eCSPECrate, 785, 485);

  GtkWidget* lGCALrate = gtk_label_new("GCAL Rate");
  gtk_fixed_put(GTK_FIXED(fixed), lGCALrate, 870, 490);
  eGCALrate = gtk_entry_new();
  gtk_widget_set_size_request(eGCALrate, 60, 25);
  gtk_fixed_put(GTK_FIXED(fixed),eGCALrate, 945, 485);

  GtkWidget* lNRSSrate = gtk_label_new("NRSS Rate");
  gtk_fixed_put(GTK_FIXED(fixed), lNRSSrate, 1030, 490);
  eNRSSrate = gtk_entry_new();
  gtk_widget_set_size_request(eNRSSrate, 60, 25);
  gtk_fixed_put(GTK_FIXED(fixed),eNRSSrate, 1105, 485);

  pbar = gtk_progress_bar_new();
  gtk_fixed_put(GTK_FIXED(fixed), pbar, 1200, 485);
  gtk_widget_set_size_request(pbar, 150, 20);

  GtkWidget* l0 = gtk_label_new("0%");
  gtk_fixed_put(GTK_FIXED(fixed), l0, 1200, 510);

  GtkWidget* l100 = gtk_label_new("100%");
  gtk_fixed_put(GTK_FIXED(fixed), l100, 1330, 510);


  /////////////////////////////////////////////////////////////////////
  // Start/Stop buttons, Run number display
  /////////////////////////////////////////////////////////////////////
  eRunNumber = gtk_entry_new();
  lRunNumber =  gtk_label_new("Run Number");
  gtk_fixed_put(GTK_FIXED(fixed), lRunNumber, 650, 590);
  gtk_widget_set_size_request(eRunNumber, 100, 25);
  gtk_fixed_put(GTK_FIXED(fixed), eRunNumber, 650, 610);
  gtk_entry_set_text((gpointer) eRunNumber,"1");

  bStartSlowControl = gtk_button_new_with_label("Start Slow Control");
  gtk_widget_set_size_request(bStartSlowControl, xbsize, ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStartSlowControl, 700, 670);
  gtk_widget_set_sensitive(bStartSlowControl, FALSE);
  g_signal_connect(G_OBJECT(bStartSlowControl), "clicked", G_CALLBACK(StartSlowControl), NULL);

  bStartDaq = gtk_button_new_with_label("Start DAQ");
  gtk_widget_set_size_request(bStartDaq, xbsize, ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStartDaq, 900, 670);
  gtk_widget_set_sensitive(bStartDaq, FALSE);
  g_signal_connect(G_OBJECT(bStartDaq), "clicked", G_CALLBACK(StartDaq), NULL);

  bStopDaq = gtk_button_new_with_label("Stop DAQ");
  gtk_widget_set_size_request(bStopDaq, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStopDaq, 1100, 670);
  gtk_widget_set_sensitive(bStopDaq, FALSE);
  g_signal_connect(G_OBJECT(bStopDaq), "clicked", G_CALLBACK(StopDaq), NULL);



  /////////////////////////////////////////////////////////////////////
  // Quit button
  /////////////////////////////////////////////////////////////////////
  bQuit = gtk_button_new_with_label("Quit");
  gtk_widget_set_size_request(bQuit, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bQuit, 1400, 750);
  g_signal_connect(G_OBJECT(bQuit), "clicked", G_CALLBACK(Quit), NULL);


  /////////////////////////////////////////////////////////////////////
  // INFN logo
  /////////////////////////////////////////////////////////////////////
  GtkWidget* image = gtk_image_new_from_file("Setup/images/logoINFN200x117.png");
  gtk_widget_set_size_request(image,145,80);
  gtk_fixed_put(GTK_FIXED(fixed), image, 1400, 20);



  return window; 
}



/*************************************************************************************************************/
void WriteLogHeader() {
/*************************************************************************************************************/
  sprintf(logFilename,"Log/LogFile_Run%i",rcDaqSharedMemory->runNumber);
  freopen (logFilename,"a",stdout);
  freopen (logFilename,"a",stderr);
  sprintf(text,"\nRunControl: WriteLogHeader \nOpening %s  \n",logFilename); 
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text); 

  
  struct timespec currTime;
  clock_gettime(CLOCK_REALTIME, &currTime);
  time_t mysec=currTime.tv_sec;
  sprintf(text,"Date and Time: %s \n",ctime(&mysec));
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text); 
  char* currentRecipe=getCurrentRecipe(cbRecipeList);
  if(strstr(currentRecipe, "Default")!=NULL)  {
    char str[100];
    FILE* deffile = popen("readlink Setup/Default","r");
    fscanf(deffile, "%s", str);
    pclose(deffile);
    currentRecipe=str;
  }

  sprintf(text,"Selected Recipe: %s \n",currentRecipe);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text); 
  sprintf(text,"Device Selection for this run: \n");
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text); 
  if(rcDaqSharedMemory->IncludeBaF) { 
    sprintf(text,"  BaF\n");
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
    printf(text); 
  }
  if(rcDaqSharedMemory->IncludeHPGe) { 
    sprintf(text,"  HPGe\n");
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
    printf(text); 
  }
  if(rcDaqSharedMemory->IncludeSiStrip) {
    sprintf(text,"  SiStrip\n");
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
    printf(text); 
  }
  if(rcDaqSharedMemory->IncludeGCAL) {
    sprintf(text,"  GCAL\n");
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
    printf(text); 
  }
  if(rcDaqSharedMemory->IncludeNRSS) {
    sprintf(text,"  NRSS\n");
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
    printf(text); 
  }

  sprintf(text,"\nRun Configuration: \n");
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text); 
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rTime))) sprintf(text,"  Run Lenght %i s \n",gtk_spin_button_get_value_as_int((gpointer) sbTime));
  else sprintf(text,"  Run Lenght %i events \n",gtk_spin_button_get_value_as_int((gpointer) sbNEvents));
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text);   
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cSoftTrg))) {
    sprintf(text,"  Software Trigger is ON (Rate %i Hz) \n",gtk_spin_button_get_value_as_int((gpointer) sbRate));
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
    printf(text); 
  }
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cTestHPGe))) {
    sprintf(text,"  Pulser HPGe is ON (Rate %i Hz) \n",gtk_spin_button_get_value_as_int((gpointer) sbRate));
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
    printf(text); 
  }  

  fflush(stdout);
}


/*************************************************************************************************************/
void loadRecipeList() {
/*************************************************************************************************************/
  FillRecipeList(cbRecipeList,getCurrentRecipe(cbRecipeList),0);
}


/*************************************************************************************************************/
void changeRecipe() {
/*************************************************************************************************************/
  V1742Ready=0;
  V1742GCALReady=0;
  DT5780Ready=0;
  DT5743Ready=0;

  gtk_widget_set_sensitive(cBaF, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cBaF), FALSE);
  gtk_widget_set_sensitive(cHPGe, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cHPGe), FALSE);
  gtk_widget_set_sensitive(cSiStrip, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cSiStrip), FALSE);
  gtk_widget_set_sensitive(cGCAL, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cGCAL), FALSE);
  gtk_widget_set_sensitive(cNRSS, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cNRSS), FALSE);
 
  gtk_widget_set_sensitive(bStartDaq, FALSE);
  gtk_widget_set_sensitive(bStopDaq, FALSE);

}

/*************************************************************************************************************/
void updateChannelToPlot() {
/*************************************************************************************************************/
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rBaFV1742Ch))) 
       rcDaqSharedMemory->PlotChannelBaF = gtk_spin_button_get_value_as_int ((gpointer) sbComptonV1742Ch);
  else rcDaqSharedMemory->PlotChannelBaF = gtk_spin_button_get_value_as_int ((gpointer) sbComptonV1742Gr);

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rGCALV1742Ch))) 
       rcDaqSharedMemory->PlotChannelGCAL = gtk_spin_button_get_value_as_int ((gpointer) sbGCALV1742Ch);
  else rcDaqSharedMemory->PlotChannelGCAL = gtk_spin_button_get_value_as_int ((gpointer) sbGCALV1742Gr);

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rDT5743Ch))) 
       rcDaqSharedMemory->PlotChannelNRSS = gtk_spin_button_get_value_as_int ((gpointer) sbDT5743Ch);
  else rcDaqSharedMemory->PlotChannelNRSS = gtk_spin_button_get_value_as_int ((gpointer) sbDT5743Gr);

  rcDaqSharedMemory->PlotChannelHPGe = gtk_spin_button_get_value_as_int ((gpointer) sbComptonDT5780Ch);
}


/*************************************************************************************************************/
void updatePlotScaler() {
/*************************************************************************************************************/
  rcDaqSharedMemory->CSPECplotScaler = gtk_spin_button_get_value_as_int ((gpointer) sbCSPECplotScaler);
  rcDaqSharedMemory->GCALplotScaler  = gtk_spin_button_get_value_as_int ((gpointer) sbGCALplotScaler);
  rcDaqSharedMemory->NRSSplotScaler  = gtk_spin_button_get_value_as_int ((gpointer) sbNRSSplotScaler);
}




/*************************************************************************************************************/
void updateRate() {
/*************************************************************************************************************/
  rcDaqSharedMemory->SoftwareTrgRate = gtk_spin_button_get_value_as_int ((gpointer) sbRate);
}


/*************************************************************************************************************/
void updateRunNumber(int increment) {
/*************************************************************************************************************/
  char strRun[100];
  FILE* file = fopen(runNumberFile,"r");
  int nrun=0;
  fscanf(file, "%i", &nrun);
  if(increment) nrun++;
  rcDaqSharedMemory->runNumber=nrun;
  fclose(file);
  file = fopen(runNumberFile,"w");
  fprintf(file,"%i",nrun);
  sprintf(strRun,"%i",nrun);
  gtk_entry_set_text((gpointer) eRunNumber,(gpointer) strRun);
  fclose(file);
}


/*************************************************************************************************************/
void updateMonitor() {
/*************************************************************************************************************/
  char strEvt[100];

  // Producer monitor
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsProd[BaFGamma]);
  gtk_entry_set_text((gpointer) eMonitorProd[BaFGamma],(gpointer) strEvt);
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsProd[BaFAlfa]);
  gtk_entry_set_text((gpointer) eMonitorProd[BaFAlfa],(gpointer) strEvt);
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsProd[HPGe]);
  gtk_entry_set_text((gpointer) eMonitorProd[HPGe],(gpointer) strEvt);
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsProd[SiStrip]);
  gtk_entry_set_text((gpointer) eMonitorProd[SiStrip],(gpointer) strEvt);
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsProd[GCAL]);
  gtk_entry_set_text((gpointer) eMonitorProd[GCAL],(gpointer) strEvt);
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsProd[NRSS]);
  gtk_entry_set_text((gpointer) eMonitorProd[NRSS],(gpointer) strEvt);

  // Consumer monitor
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsCons[BaFGamma]);
  gtk_entry_set_text((gpointer) eMonitorCons[BaFGamma],(gpointer) strEvt);
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsCons[BaFAlfa]);
  gtk_entry_set_text((gpointer) eMonitorCons[BaFAlfa],(gpointer) strEvt);
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsCons[HPGe]);
  gtk_entry_set_text((gpointer) eMonitorCons[HPGe],(gpointer) strEvt);
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsCons[SiStrip]);
  gtk_entry_set_text((gpointer) eMonitorCons[SiStrip],(gpointer) strEvt);
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsCons[GCAL]);
  gtk_entry_set_text((gpointer) eMonitorCons[GCAL],(gpointer) strEvt);
  sprintf(strEvt,"%i",rcDaqSharedMemory->EventsCons[NRSS]);
  gtk_entry_set_text((gpointer) eMonitorCons[NRSS],(gpointer) strEvt);

  // Event rate monitor
  double myTime=getTime();
  double evtRate;
  if(rcDaqSharedMemory->EventsProd[BaFGamma] !=0) evtRate=rcDaqSharedMemory->EventsProd[BaFGamma]/(myTime-rcDaqSharedMemory->startTime);  
  else evtRate=rcDaqSharedMemory->EventsProd[SiStrip]/(myTime-rcDaqSharedMemory->startTime);  
  sprintf(strEvt,"%6.2f",evtRate);
  gtk_entry_set_text((gpointer) eCSPECrate,(gpointer) strEvt);

  evtRate=rcDaqSharedMemory->EventsProd[GCAL]/(myTime-rcDaqSharedMemory->startTime);  
  sprintf(strEvt,"%6.2f",evtRate);
  gtk_entry_set_text((gpointer) eGCALrate,(gpointer) strEvt);

  evtRate=rcDaqSharedMemory->EventsProd[NRSS]/(myTime-rcDaqSharedMemory->startTime);  
  sprintf(strEvt,"%6.2f",evtRate);
  gtk_entry_set_text((gpointer) eNRSSrate,(gpointer) strEvt);

}

/*************************************************************************************************************/
void checkIncludedDevice() {
/*************************************************************************************************************/
  int countDevice=0;
  if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cBaF)) || 
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cHPGe)) || 
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cSiStrip)))  countDevice++;

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cGCAL))) countDevice++;

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cNRSS))) countDevice++;

  if(countDevice>1) gtk_widget_set_sensitive(rNEvents, FALSE);
  else gtk_widget_set_sensitive(rNEvents, TRUE);

  if( !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cBaF)) && 
       gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cHPGe)) && 
      !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cSiStrip)) && 
      !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cGCAL)) && V1495Ready) gtk_widget_set_sensitive(cTestHPGe, TRUE);
  else 
    gtk_widget_set_sensitive(cTestHPGe, FALSE);
}

/*************************************************************************************************************/
void checkRunNumber() {
/*************************************************************************************************************/
  int RunLenght,TimeBasedRunLength;
  double currentTime,startTime;
  int count=0;
  int delay=0;

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rTime))) {
    RunLenght=gtk_spin_button_get_value_as_int((gpointer) sbTime);
    TimeBasedRunLength=1;
  } 
  else if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rNEvents))) {
    RunLenght=gtk_spin_button_get_value_as_int((gpointer) sbNEvents);
    TimeBasedRunLength=0;
  }

  if(TimeBasedRunLength) startTime=getTime();

  while(running) {
    while (gtk_events_pending()) gtk_main_iteration();
    usleep(1000); delay++;

//    // Check the slow control system
//    if(delay%(2*SlowControlSleepTime*1000)==0) { 
//      if(rcDaqSharedMemory->IncludeBaF) DecodeSlowControlStatus(BaF); 
//      //if(rcDaqSharedMemory->IncludeSiStrip) DecodeSlowControlStatus(SiStrip); 
//      if(rcDaqSharedMemory->IncludeHPGe) DecodeSlowControlStatus(HPGe); 
//      if(rcDaqSharedMemory->IncludeGCAL) DecodeSlowControlStatus(GCAL); 
//      if(rcDaqSharedMemory->IncludeNRSS) DecodeSlowControlStatus(NRSS); 
//      delay=0;
//    }
//

    // *** 23/05/18 m.v. fix count !! 
    if(TimeBasedRunLength) {
      currentTime=getTime();
      count=currentTime-startTime;
    } 
    else if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cGCAL))) {
      count=rcDaqSharedMemory->EventsCons[GCAL];
    } 
    else if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cNRSS))) {
      count=rcDaqSharedMemory->EventsCons[NRSS];
    } 
    else {
      count=rcDaqSharedMemory->EventsCons[BaFGamma]+rcDaqSharedMemory->EventsCons[BaFAlfa]+rcDaqSharedMemory->EventsCons[HPGe]+rcDaqSharedMemory->EventsCons[SiStrip];
    }

    // Progress bar
    gtk_progress_bar_update(GTK_PROGRESS_BAR(pbar), (double)count/(double)RunLenght);

    if(count >= RunLenght) {
      updateRunNumber(1);
      count=0;
      if(TimeBasedRunLength) startTime=getTime();
      for(int id=0;id<nMon;id++) rcDaqSharedMemory->EventsProd[id]=0;
      WriteLogHeader();
      // Update the display of the data filename
      char filename[400];
      strcpy(filename,"Data/");
      char strnrun[100];
      sprintf(strnrun,"Run%i",rcDaqSharedMemory->runNumber);
      strcat(filename,strnrun);
      strcat(filename,"_");
      if(rcDaqSharedMemory->IncludeBaF || rcDaqSharedMemory->IncludeHPGe || rcDaqSharedMemory->IncludeSiStrip) strcat(filename,"CSPEC");
      if(rcDaqSharedMemory->IncludeGCAL) strcat(filename,"GCAL");
      if(rcDaqSharedMemory->IncludeNRSS) strcat(filename,"NRSS");
      strcat(filename,"_");
      strcat(filename,nameRecipe);
      gtk_entry_set_text((gpointer) eFileName,filename);
    }
      
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cMonitor))) updateMonitor(); 
  }
}


/*************************************************************************************************************/
void DecodeSlowControlStatus(Device_t detector) {
/*************************************************************************************************************/
  uint32_t u32;
  char name[10];

  if(detector==BaF) {    
    sprintf(name,"%s","BaF");
    u32=rcDaqSharedMemory->bafStatus;
  }
  else if(detector==SiStrip) {    
    sprintf(name,"%s","SiStrip");
    u32=rcDaqSharedMemory->sistripStatus;
  }
  else if(detector==HPGe) {    
    sprintf(name,"%s","HPGe");
    u32=rcDaqSharedMemory->hpgeStatus;
  }
  else if(detector==GCAL) {
    sprintf(name,"%s","GCAL");
    u32=rcDaqSharedMemory->gcalStatus;
  }
  else if(detector==NRSS) {
    sprintf(name,"%s","NRSS");
    u32=rcDaqSharedMemory->nrssStatus;
  }

  GetTime(timeNow);
 
  if(! (u32 & (0x1<<0)) ) {
    sprintf(text,"%s ***WARNING: The %s slow control is not running\n",timeNow,name);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "red_bg", NULL);
    printf(text);
  }

  if(u32 & (0x1 << 1)) {
      sprintf(text,"%s ***WARNING: %s HV channel OFF\n",timeNow,name);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "red_bg", NULL);
      printf(text);
  }

  if(u32 & (0x1 << 2) || u32 & (0x1 << 3) || u32 & (0x1 << 4)) {
      sprintf(text,"%s ***WARNING: %s HV status alarm\n",timeNow,name);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "red_bg", NULL);
      printf(text);
  }  

  if(u32 & (0x1 << 17)) {
      sprintf(text,"%s ***WARNING: %s LV channel OFF\n",timeNow,name);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "red_bg", NULL);
      printf(text);
  }

  if(u32 & (0x1 << 18) || u32 & (0x1 << 19) || u32 & (0x1 << 20)) {
      sprintf(text,"%s ***WARNING: %s LV status alarm\n",timeNow,name);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "red_bg", NULL);
      printf(text);
  }  


}



/*************************************************************************************************************/
void CheckTestHPGe() {
/*************************************************************************************************************/
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cTestHPGe)))
    rcDaqSharedMemory->testHPGe=0;
  else
    rcDaqSharedMemory->testHPGe=1;
}



/*************************************************************************************************************/
void selectRunType() {
/*************************************************************************************************************/
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rTime))) {
    gtk_widget_set_sensitive(sbTime, TRUE);
    gtk_widget_set_sensitive(sbNEvents, FALSE);
  } 
  else {
    gtk_widget_set_sensitive(sbTime, FALSE);
    gtk_widget_set_sensitive(sbNEvents, TRUE);
  }
}


/*************************************************************************************************************/
void selectPlotType() {
/*************************************************************************************************************/
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rGCALV1742Ch))) 
    rcDaqSharedMemory->isGCALplotGroup=0;
  else 
    rcDaqSharedMemory->isGCALplotGroup=1;

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rBaFV1742Ch))) 
    rcDaqSharedMemory->isBaFplotGroup=0;
  else 
    rcDaqSharedMemory->isBaFplotGroup=1;

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rDT5743Ch))) 
    rcDaqSharedMemory->isNRSSplotGroup=0;
  else 
    rcDaqSharedMemory->isNRSSplotGroup=1;

  updateChannelToPlot();
}

/*************************************************************************************************************/
void startPlotter(char* device) {
/*************************************************************************************************************/
  sprintf(text,"RunControl: starting %s Plotter for Run %i\n",device,rcDaqSharedMemory->runNumber);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text);

  char command[200]="bin/Plotter -d ";
  strcat(command,device);
  strcat(command,"&");
  ret = system(command);

}

/*************************************************************************************************************/
void checkGCALplotter() {
/*************************************************************************************************************/
  if(running) {
    // check GCAL Plotter
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cGCALplotter))) {
      rcDaqSharedMemory->GCALplotScaler=gtk_spin_button_get_value_as_int((gpointer) sbGCALplotScaler);
      rcDaqSharedMemory->stopGCALplotter=0;
      startPlotter("GCAL");
    }
    else {
      rcDaqSharedMemory->stopGCALplotter=1;
    }
    selectPlotType();
  }
}

/*************************************************************************************************************/
void checkCSPECplotter() {
/*************************************************************************************************************/
  if(running) {
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cCSPECplotter))) {
      rcDaqSharedMemory->CSPECplotScaler=gtk_spin_button_get_value_as_int((gpointer) sbCSPECplotScaler);
      rcDaqSharedMemory->stopCSPECplotter=0;
      startPlotter("CSPEC");
    }
    else {
      rcDaqSharedMemory->stopCSPECplotter=1;
    }
    selectPlotType();
  }
}

/*************************************************************************************************************/
void checkNRSSplotter() {
/*************************************************************************************************************/
  if(running) {
    // check NRSS Plotter
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (cNRSSplotter))) {
      rcDaqSharedMemory->NRSSplotScaler=gtk_spin_button_get_value_as_int((gpointer) sbNRSSplotScaler);
      rcDaqSharedMemory->stopNRSSplotter=0;
      startPlotter("NRSS");
    }
    else {
      rcDaqSharedMemory->stopNRSSplotter=1;
    }
    selectPlotType();
  }
}


/*************************************************************************************************************/
void startConsumer(char* device) {
/*************************************************************************************************************/
  char filename[400];
  strcpy(filename,"Data/");

  char strnrun[100];
  sprintf(strnrun,"Run%i",rcDaqSharedMemory->runNumber);
  strcat(filename,strnrun);

  strcat(filename,"_");
  strcat(filename,device);
  strcat(filename,"_");

  strcpy(nameRecipe,"");

  char* recipe;
  char defrecipe[100];
  recipe=(char*) gtk_combo_box_text_get_active_text ((GtkComboBoxText *) cbRecipeList );
  if(strstr(recipe, "Default")!=NULL) {
    FILE* deffile = popen("readlink Setup/Default","r");
    fscanf(deffile, "%s", defrecipe);
    pclose(deffile);
    strcat(filename,defrecipe);
    strcat(nameRecipe,defrecipe);
  } 
  else {
    strcat(filename,recipe);
    strcat(nameRecipe,recipe);
  }

  gtk_entry_set_text((gpointer) eFileName,filename);

  sprintf(text,"RunControl: Starting %s Consumer for Run %i\n",device,rcDaqSharedMemory->runNumber);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text);
  fflush(stdout);
  char command[200]="bin/Consumer -f ";
  strcat(command,nameRecipe);
  strcat(command," -d ");
  strcat(command,device);
  strcat(command," &");
  ret = system(command);  
}



/*************************************************************************************************************/
void startProducer(char* device) {
/*************************************************************************************************************/
  sprintf(text,"RunControl: starting %s Producer for Run %i\n",device,rcDaqSharedMemory->runNumber);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text);
  fflush(stdout);

  char command[200]="bin/Producer -d ";
  strcat(command,device);
  strcat(command," &");
  ret = system(command);
}



/*************************************************************************************************************/
void ConfigDigi() { 
/*************************************************************************************************************/
  rcDaqSharedMemory->IncludeBaF=FALSE;
  rcDaqSharedMemory->IncludeSiStrip=FALSE;
  rcDaqSharedMemory->IncludeHPGe=FALSE;
  rcDaqSharedMemory->IncludeGCAL=FALSE;
  rcDaqSharedMemory->IncludeNRSS=FALSE;
  gtk_widget_set_sensitive(cGCALplotter, FALSE);
  gtk_widget_set_sensitive(cCSPECplotter, FALSE);
  gtk_widget_set_sensitive(cNRSSplotter, FALSE);

//  int isSlowControlRunning = CheckRunningProcesses(SlowControl);
//  printf("isSlowControlRunning %i \n",isSlowControlRunning);
//  int isHPGeSlowControlRunning = CheckRunningProcesses(HPGeSlowControl);
//  printf("isHPGeSlowControlRunning %i \n",isHPGeSlowControlRunning);

  char* V812configfile=getConfigFile(cbRecipeList,Discr);
  if( access( V812configfile, F_OK ) != -1 ) { 
    ret = ProgramV812(V812configfile);   
    if(ret==0)  g_print("Discriminator V812 successfully configured \n");
    else g_print("***RunControl: ERROR configuring V812: %i \n",ret);
  }

  char* V1495configfile=getConfigFile(cbRecipeList,SiStrip);
  if( access( V1495configfile, F_OK ) != -1 ) { 
    V1495Params_t Params;
    ParseConfigFileV1495(V1495configfile, &Params);
    rcDaqSharedMemory->connectionParamsV1495[0]=Params.LinkNum;
    rcDaqSharedMemory->connectionParamsV1495[1]=Params.BaseAddress;
    ret = TestConnection(&Params);
    if(ret==cvSuccess) {
      g_print("RunControl: IO board V1495 successfully configured \n");
      V1495Ready=1;
      gtk_widget_set_sensitive(cSiStrip, TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cSiStrip), TRUE);
      rcDaqSharedMemory->IncludeSiStrip=TRUE;
    } 
    else g_print("***RunControl: ERROR configuring V1495: %i \n",CAENVME_DecodeError(ret));
  }

  char* DT5780configfile=getConfigFile(cbRecipeList,HPGe);
  if( access( DT5780configfile, F_OK ) != -1 ) {
    ret=ProgramDigitizerDT5780(connectionParams,DT5780configfile);
    if(ret==0) {
      g_print("RunControl: Digitizer DT5780 successfully configured \n");
      for(int c=0;c<4;c++) rcDaqSharedMemory->connectionParamsDT5780[c]=connectionParams[c];
      DT5780Ready=1;
      gtk_widget_set_sensitive(cHPGe, TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cHPGe), TRUE);
      rcDaqSharedMemory->IncludeHPGe=TRUE;
    } 
    else g_print("***RunControl: ERROR configuring DT5780: %i \n",ret);
  }

  char* DT5743configfile=getConfigFile(cbRecipeList,NRSS);
  if( access( DT5743configfile, F_OK ) != -1 ) {

    ret=ProgramDigitizerDT5743(connectionParams, DT5743configfile);
    if(ret==0) {
      g_print("RunControl: Digitizer DT5743 successfully configured \n");
      for(int c=0;c<4;c++) rcDaqSharedMemory->connectionParamsDT5743[c]=connectionParams[c];
      DT5743Ready=1;
      gtk_widget_set_sensitive(cNRSS, TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cNRSS), TRUE);
      rcDaqSharedMemory->IncludeNRSS=TRUE;
      gtk_widget_set_sensitive(cNRSSplotter, TRUE);

      // Parse the configuration file to get the values of DCOffset and TriggerLevel
      DT5743Params_t Params;
      CAEN_DGTZ_DPP_X743_Params_t DPP_Params;
      ParseConfigFileDT5743(DT5743configfile, &Params, &DPP_Params);
      for(int i=0; i<MaxDT5743NChannels; i++) {
        rcDaqSharedMemory->DCOffset[i]=Params.DCOffset[i];
        rcDaqSharedMemory->TriggerLevel[i]=Params.TriggerLevel[i];
      }
    } 
    else g_print("***RunControl: ERROR configuring DT5743: %i \n",ret);
  }


  char* V1742BaFconfigfile=getConfigFile(cbRecipeList,BaF);
  if( access( V1742BaFconfigfile, F_OK ) != -1 ) {
    ret=ProgramDigitizerV1742(connectionParams,V1742BaFconfigfile,0);
    if(ret==0) {
      g_print("RunControl: Digitizer V1742 (BaF) successfully configured \n");
      for(int c=0;c<4;c++) rcDaqSharedMemory->connectionParamsV1742BaF[c]=connectionParams[c];
      V1742Ready=1;
      gtk_widget_set_sensitive(cBaF, TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cBaF), TRUE);
      rcDaqSharedMemory->IncludeBaF=TRUE;
    } 
    else g_print("***RunControl: ERROR configuring V1742 (BaF): %i \n",ret);
  }

  char* V1742GCALconfigfile=getConfigFile(cbRecipeList,GCAL);
  if( access( V1742GCALconfigfile, F_OK ) != -1 ) {
    ret=ProgramDigitizerV1742(connectionParams,V1742GCALconfigfile,1);
    if(ret==0) {
      g_print("RunControl: Digitizer V1742 (GCAL) successfully configured \n");
      for(int c=0;c<4;c++) rcDaqSharedMemory->connectionParamsV1742GCAL[c]=connectionParams[c];
      V1742GCALReady=1;
      gtk_widget_set_sensitive(cGCAL, TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (cGCAL), TRUE);
      rcDaqSharedMemory->IncludeGCAL=TRUE;
      gtk_widget_set_sensitive(cGCALplotter, TRUE);
    } 
    else g_print("***RunControl: ERROR configuring V1742 (GCAL): %i \n",ret);
  }

  if(rcDaqSharedMemory->IncludeBaF || rcDaqSharedMemory->IncludeSiStrip || rcDaqSharedMemory->IncludeHPGe) gtk_widget_set_sensitive(cCSPECplotter, TRUE);

  //gtk_widget_set_sensitive(bStartSlowControl, TRUE);
  gtk_widget_set_sensitive(bStartDaq, TRUE);

}


/*************************************************************************************************************/
void StartSlowControl() {
/*************************************************************************************************************/
  sprintf(text,"Starting the Slow Control for Run %i\n",rcDaqSharedMemory->runNumber+1);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text);

  char command[100];
  sprintf(command,"bin/SlowControl &");
  ret = system(command);

  gtk_widget_set_sensitive(bStartDaq, TRUE);
  gtk_widget_set_sensitive(bStartSlowControl, FALSE);

}


/*************************************************************************************************************/
void StartDaq() {
/*************************************************************************************************************/
  int et=0;
  for(int et=0;et<nMon;et++) {
    rcDaqSharedMemory->EventsProd[et]=0;
    rcDaqSharedMemory->EventsCons[et]=0;
  }

  rcDaqSharedMemory->stopDAQ=0; 

  gtk_widget_set_sensitive(bStopDaq, TRUE);
  //////gtk_widget_set_sensitive(bStartDaq, FALSE);
  gtk_widget_set_sensitive(bStartDaq, TRUE);
  gtk_widget_set_sensitive(cBaF, FALSE);
  gtk_widget_set_sensitive(cHPGe, FALSE);
  gtk_widget_set_sensitive(cSiStrip, FALSE);
  gtk_widget_set_sensitive(cSoftTrg,FALSE);
  gtk_widget_set_sensitive(cTestHPGe,FALSE);
  gtk_widget_set_sensitive(sbRate,FALSE);
  gtk_widget_set_sensitive(cConsumer, FALSE);
  gtk_widget_set_sensitive(bConfigure,FALSE);

  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cSoftTrg))) rcDaqSharedMemory->softwareTrigger=1; 
  else rcDaqSharedMemory->softwareTrigger=0;
  if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cTestHPGe))) rcDaqSharedMemory->testHPGe=1; 
  else rcDaqSharedMemory->testHPGe=0;
  rcDaqSharedMemory->AlfaScaler = gtk_spin_button_get_value_as_int ((gpointer) sbAlfaScaler);

  updateRunNumber(1);
  updateRate();
  WriteLogHeader();

  rcDaqSharedMemory->stopCSPECproducer=-1;
  rcDaqSharedMemory->stopGCALproducer=-1;
  rcDaqSharedMemory->stopNRSSproducer=-1;
  if(rcDaqSharedMemory->IncludeBaF || rcDaqSharedMemory->IncludeHPGe || rcDaqSharedMemory->IncludeSiStrip) {    
    startProducer("CSPEC");
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cConsumer))) startConsumer("CSPEC");
  }

  if(rcDaqSharedMemory->IncludeGCAL) {
    startProducer("GCAL");
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cConsumer))) startConsumer("GCAL");
  } 

  if(rcDaqSharedMemory->IncludeNRSS) {
    startProducer("NRSS");
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (cConsumer))) startConsumer("NRSS");
  } 

  running=1;
  checkGCALplotter();
  checkCSPECplotter();
  checkNRSSplotter();
  checkRunNumber();
}


/*************************************************************************************************************/
void StopDaq() {
/*************************************************************************************************************/ 
  GetTime(timeNow);
  sprintf(text,"%s RunControl: Stopping the acquisition for Run%i \n",timeNow,rcDaqSharedMemory->runNumber);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
  printf(text);

  rcDaqSharedMemory->stopDAQ=1;
  rcDaqSharedMemory->stopCSPECplotter=1;
  rcDaqSharedMemory->stopGCALplotter=1;
  rcDaqSharedMemory->stopNRSSplotter=1;

  running=0;
  gtk_widget_set_sensitive(bStopDaq, FALSE);
  gtk_widget_set_sensitive(bStartDaq, TRUE);
  if(V1742Ready) gtk_widget_set_sensitive(cBaF, TRUE);
  if(V1742GCALReady) gtk_widget_set_sensitive(cGCAL, TRUE);
  if(DT5780Ready) gtk_widget_set_sensitive(cHPGe, TRUE);
  if(V1495Ready) gtk_widget_set_sensitive(cSiStrip, TRUE);
  if(DT5743Ready) gtk_widget_set_sensitive(cNRSS, TRUE);
  gtk_widget_set_sensitive(cSoftTrg,TRUE);
  if(DT5780Ready) gtk_widget_set_sensitive(cTestHPGe,TRUE);
  gtk_widget_set_sensitive(sbRate,TRUE);
  gtk_widget_set_sensitive(bConfigure,TRUE);
  gtk_widget_set_sensitive(cConsumer, TRUE);

}


/*************************************************************************************************************/
void Quit() {
/*************************************************************************************************************/
  rcDaqSharedMemory->stopDAQ=1;
  rcDaqSharedMemory->stopCSPECplotter=1;
  rcDaqSharedMemory->stopGCALplotter=1;
  rcDaqSharedMemory->stopNRSSplotter=1;

  int wait=0, stopProducer, stopConsumer;
  if(rcDaqSharedMemory->IncludeBaF || rcDaqSharedMemory->IncludeHPGe || rcDaqSharedMemory->IncludeSiStrip) { 
    stopProducer=rcDaqSharedMemory->stopCSPECproducer;
    stopConsumer=rcDaqSharedMemory->stopCSPECconsumer;
  }
  else if(rcDaqSharedMemory->IncludeGCAL){ 
    stopProducer=rcDaqSharedMemory->stopGCALproducer;
    stopConsumer=rcDaqSharedMemory->stopGCALconsumer;
  }
  else if(rcDaqSharedMemory->IncludeNRSS){ 
    stopProducer=rcDaqSharedMemory->stopNRSSproducer;
    stopConsumer=rcDaqSharedMemory->stopNRSSconsumer;
  }
  while(stopProducer !=1 || stopConsumer !=1) {
    if(wait>4) break;
    sleep(1);
    sprintf(text,"RunControl: Waiting the termination of children processes %is - stopProducer %i  stopConsumer %i\n",
            wait+1,stopProducer,stopConsumer);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);
    printf(text);
    wait++;
  }
  deleteDaqSharedMemory(rcDaqSharedMemory,1);

  fclose (stdout);

  gtk_main_quit();
}


/*************************************************************************************************************/
void GetTime(char *timeNow) {
/*************************************************************************************************************/
  time_t now;
  struct tm *tm;
  char bufTime[21];
  char mesgTime[22];
       
  now = time(NULL);
  tm = localtime(&now);
  strftime(bufTime, sizeof(bufTime), "%d-%m-%Y  %H:%M:%S", tm);
  sprintf(mesgTime,"%s", bufTime);
  mesgTime[22]='\0';
  
  strcpy(timeNow, mesgTime);
  
}


/*************************************************************************************************************/
/*************************************************************************************************************/

//int CheckRunningProcesses(process_t caller) {
//#define LINE_BUFSIZE 128
//
//    char line[LINE_BUFSIZE];
//    FILE *pipe;
//
//    if(caller==RunControl) pipe = popen("./bin/CheckRC", "r");
//    else if(caller==SlowControl) pipe = popen("./bin/CheckSC", "r");
//    else if(caller==HPGeSlowControl) pipe = popen("./bin/CheckHSC", "r");
//    else if(caller==Producer) pipe = popen("./bin/CheckP", "r");
//    else if(caller==Consumer) pipe = popen("./bin/CheckC", "r");
//
//    // Check for errors 
//    if(pipe == NULL) perror("***CheckRunningProcesses: No pipe ");
//
//    // Read the script output from the pipe line by line
//    int linenr=0;
//    while (fgets(line, LINE_BUFSIZE, pipe) != NULL) {
//      printf("%s", line);
//      ++linenr;
//    }
//
//    pclose(pipe); /* Close the pipe */
//
//   return linenr;
//}
