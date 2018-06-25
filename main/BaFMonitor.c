#include "GCALMonitor.h"

#define nHV 1
#define nLV 2

// Slot number for HV/LV boards in the SY5527 mainframe
#define slotA7030 0
#define slotA2518 2
#define slotA1536 3


DaqSharedMemory* gcalDaqSharedMemory;

GtkWidget *main_window;

char timeNow[22];
char text[100];
char status[20];

int handle;
unsigned short HVList[nHV]={0};
unsigned short LVList[nLV]={2,3};

int ParStatus[nHV], Power[nHV], ParStatusL[nLV], PowerL[nLV];

float ParVMon[nHV], ParIMon[nHV], ParV0Set[nHV], ParI0Set[nHV], ParPw[nHV], ParRup[nHV], ParRdwn[nHV], ParTrip[nHV], ParVal[nHV];
float ParVMonL[nLV], ParIMonL[nLV], ParV0SetL[nLV], ParI0SetL[nLV], ParPwL[nLV], ParRupL[nLV], ParRdwnL[nLV], ParVConL[nLV], ParValL[nLV];

GtkWidget *eVMon[nHV], *eIMon[nHV], *eV0Set[nHV], *eI0Set[nHV], *ePw[nHV], *eStatus[nHV], *eRup[nHV], *eRdwn[nHV], *eTrip[nHV];
GtkWidget *eVMonL[nLV], *eVConL[nLV], *eIMonL[nLV], *eV0SetL[nLV], *eI0SetL[nLV], *ePwL[nLV], *eStatusL[nLV], *eRupL[nLV], *eRdwnL[nLV];

GtkWidget *sbValueCommand, *sbValueLVCommand;

GtkWidget *eUpdateTime;

GtkWidget *lVMon, *lIMon, *lV0Set, *lI0Set, *lRup, *lRdwn, *lTrip, *lPw, *lStatus;
GtkWidget *lVMonL, *lIMonL, *lV0SetL, *lI0SetL, *lRupL, *lRdwnL, *lVConL, *lPwL, *lStatusL;

GtkWidget *lChannel[nHV], *lChannelL[nLV], *lUpdateTime;

GtkWidget *bQuit, *bStart, *bStop, *bSend, *bSendLV, *bAlarm;
GtkWidget *cCommand, *cHVSelector, *cLVCommand, *cLVSelector;
GtkTextIter iter;
GtkTextBuffer *buffer;

char strEntry[10];

//unsigned short running=0;
unsigned short updateAllHVParam, updateAllLVParam;

int main(int argc, char *argv[]) {
  GtkWidget* main_window;
  gtk_init(&argc, &argv);
  main_window = create_main_window();
  gtk_widget_show_all(main_window);
  g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gcalDaqSharedMemory=configDaqSharedMemory("BaFMonitor");
  sprintf(text,"Configuring the DAQ shared memory\n");
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, text, -1, "lmarg", "blue_fg", NULL);


  // Open logfile 
  char logFileName[100];
  sprintf(logFileName,"Log/BaFMonitorLog_Run%i",gcalDaqSharedMemory->runNumber);
  freopen(logFileName,"a",stdout);
  sprintf(text,"Opening logfile: %s \n",logFileName);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
  printf(text);


  // Open the connection with the SY5527 mainframe
  // 3->SY5527   
  // 0->TCP/IP
  int err_code =CAENHV_InitSystem(3,0,"172.16.3.164","admin","admin",&handle);
  GetTime(timeNow);
  if(!err_code) {
    sprintf(text,"%s Connection opened with the SY5527 mainframe \n",timeNow);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
    printf(text);
  }
  else {
    sprintf(text,"%s ERROR opening the SY5527 mainframe - code %u \n",timeNow,err_code);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    printf(text);
    exit(1);
  }

  CrateMap(handle);

  gcalDaqSharedMemory->gcalStatus=0;

//test ///////////////////////////////////////////////////////////////////////
/*
        char* ParNameList = (char *)NULL;
        char  (*par)[10];
        int parNumber=0;

        err_code = CAENHV_GetChParamInfo(handle, 3, 0, &ParNameList,&parNumber);
        par = (char (*)[10])ParNameList;

        for(int i=0; i<parNumber; i++ ) {
         sprintf(text,"%s\n", par[i]);
         gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "yellow_bg", NULL);
        }

*/

//////////////////////////////////////////////////////////////////////


  gtk_main();
  
  return 0;
}  




/*************************************************************************************************************/
//  GtkWidget* create_main_window(void)
//  Create the main window
/*************************************************************************************************************/
GtkWidget* create_main_window(void) {
    
  GdkColor color; // 16-bit are used. Multiply RGB values (range 1-255) by 65535/255=257
  color.red = 0x5555;
  color.green = 0x9999;
  color.blue = 0x6666;
  
  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "BaF SLOW CONTROL");
  gtk_window_set_default_size(GTK_WINDOW(window), 1550, 830);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);

  GtkWidget *fixed = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(window), fixed);  

    
  /////////////////////////////////////////////////////////////////////
  // HV setup frame
  /////////////////////////////////////////////////////////////////////
  GtkWidget *frameHV = gtk_frame_new ("  HV - A1536DN  ");
  gtk_fixed_put(GTK_FIXED(fixed), frameHV, 5, 10);
  gtk_widget_set_size_request (frameHV, 800, 560);
     
  int xsize=65, ysize=30;
  int xspace=10, yspace=10;
  int xoffset=0, yoffset=50;
  char strChan[2];
  char labelChan[10];
  for(int n=0; n<nHV; n++) {
    eVMon[n] = gtk_entry_new();
    gtk_widget_set_size_request(eVMon[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eVMon[n], xoffset+xsize+xspace, yoffset+(ysize+yspace)*n);

    eIMon[n] = gtk_entry_new();
    gtk_widget_set_size_request(eIMon[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eIMon[n], xoffset+(xsize+xspace)*2, yoffset+(ysize+yspace)*n);

    eV0Set[n] = gtk_entry_new();
    gtk_widget_set_size_request(eV0Set[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eV0Set[n], xoffset+(xsize+xspace)*3, yoffset+(ysize+yspace)*n);

    eI0Set[n] = gtk_entry_new();
    gtk_widget_set_size_request(eI0Set[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eI0Set[n], xoffset+(xsize+xspace)*4, yoffset+(ysize+yspace)*n);

    eRup[n] = gtk_entry_new();
    gtk_widget_set_size_request(eRup[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eRup[n], xoffset+(xsize+xspace)*5, yoffset+(ysize+yspace)*n);

    eRdwn[n] = gtk_entry_new();
    gtk_widget_set_size_request(eRdwn[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eRdwn[n], xoffset+(xsize+xspace)*6, yoffset+(ysize+yspace)*n);

    eTrip[n] = gtk_entry_new();
    gtk_widget_set_size_request(eTrip[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eTrip[n], xoffset+(xsize+xspace)*7, yoffset+(ysize+yspace)*n);

    ePw[n] = gtk_entry_new();
    gtk_widget_set_size_request(ePw[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), ePw[n], xoffset+(xsize+xspace)*8, yoffset+(ysize+yspace)*n);

    eStatus[n] = gtk_entry_new();
    gtk_widget_set_size_request(eStatus[n], xsize+30, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eStatus[n], xoffset+(xsize+xspace)*9, yoffset+(ysize+yspace)*n);

    if(n<10)sprintf(strChan,"0%i",n);
    else sprintf(strChan,"%i",n);
    strcpy(labelChan,"CH");
    strcat(labelChan,strChan);
    lChannel[n]=gtk_label_new(labelChan);
    gtk_fixed_put(GTK_FIXED(fixed), lChannel[n], 20, yoffset+5+(ysize+yspace)*n);
  }

  int lxoff=2, lyoff=20;
  xspace += xsize;
  lVMon =  gtk_label_new("VMon (V)");
  gtk_fixed_put(GTK_FIXED(fixed), lVMon, lxoff+xspace, yoffset-lyoff);

  lIMon =  gtk_label_new("IMon (uA)");
  gtk_fixed_put(GTK_FIXED(fixed), lIMon, lxoff+(xspace*2), yoffset-lyoff);

  lV0Set =  gtk_label_new("VSet (V)");
  gtk_fixed_put(GTK_FIXED(fixed), lV0Set, lxoff+(xspace*3), yoffset-lyoff);

  lI0Set =  gtk_label_new("ISet (uA)");
  gtk_fixed_put(GTK_FIXED(fixed), lI0Set, lxoff+(xspace*4), yoffset-lyoff);

  lRup =  gtk_label_new("Rup (s)");
  gtk_fixed_put(GTK_FIXED(fixed), lRup, lxoff+(xspace*5), yoffset-lyoff);

  lRdwn =  gtk_label_new("Rdwn (s)");
  gtk_fixed_put(GTK_FIXED(fixed), lRdwn, lxoff+(xspace*6), yoffset-lyoff);

  lTrip =  gtk_label_new("Trip");
  gtk_fixed_put(GTK_FIXED(fixed), lTrip, lxoff+(xspace*7), yoffset-lyoff);

  lPw =  gtk_label_new("   Pw");
  gtk_fixed_put(GTK_FIXED(fixed), lPw, lxoff+(xspace*8), yoffset-lyoff);

  lStatus =  gtk_label_new("  Status");
  gtk_fixed_put(GTK_FIXED(fixed), lStatus, lxoff+(xspace*9), yoffset-lyoff);


  /////////////////////////////////////////////////////////////////////
  // ComboBox for the HV Channel selection
  /////////////////////////////////////////////////////////////////////
  GList *csel = NULL;
  csel = g_list_append (csel, "CH00");
  cHVSelector=gtk_combo_new();
  gtk_widget_set_size_request(cHVSelector, 80, 30);
  gtk_combo_set_popdown_strings (GTK_COMBO(cHVSelector), csel);
  gtk_fixed_put(GTK_FIXED(fixed), cHVSelector, 100, 500);
  

  /////////////////////////////////////////////////////////////////////
  // ComboBox for the parameter selection
  /////////////////////////////////////////////////////////////////////
  GList *cbitems = NULL;
  cbitems = g_list_append (cbitems, "V0Set");
  cbitems = g_list_append (cbitems, "I0Set");
  cbitems = g_list_append (cbitems, "RUp");
  cbitems = g_list_append (cbitems, "RDWn");
  cbitems = g_list_append (cbitems, "Trip");
  cbitems = g_list_append (cbitems, "POWER ON");
  cbitems = g_list_append (cbitems, "POWER OFF");
  cCommand=gtk_combo_new(); 
  gtk_combo_set_popdown_strings (GTK_COMBO(cCommand), cbitems);
  gtk_widget_set_size_request(cCommand, 150, 30);
  gtk_fixed_put(GTK_FIXED(fixed), cCommand,220, 500);

  GtkAdjustment *adjustment = (GtkAdjustment *) gtk_adjustment_new (600.0, 0.0, 1000.0, 50.0, 1.0, 0.0);
  sbValueCommand = gtk_spin_button_new (adjustment, 1.0, 2);
  gtk_fixed_put(GTK_FIXED(fixed), sbValueCommand, 450, 500);

  bSend=gtk_button_new_with_label("Send");
  gtk_widget_set_size_request(bSend, 100, 30);
  gtk_fixed_put(GTK_FIXED(fixed), bSend, 600, 500);
  g_signal_connect(G_OBJECT(bSend), "clicked", G_CALLBACK(SendCommand), NULL);


  /////////////////////////////////////////////////////////////////////
  // LV setup frame
  ///////////////////////////////////////////////////////////////////// 
  GtkWidget *frameLV = gtk_frame_new ("  LV - A2518  ");
  gtk_fixed_put(GTK_FIXED(fixed), frameLV, 5, 590);
  gtk_widget_set_size_request (frameLV, 800, 210);


  xsize=65; ysize=30; xspace=10; yspace=10;
  yoffset=650;
  for(int n=0; n<nLV; n++) {
    eVMonL[n] = gtk_entry_new();
    gtk_widget_set_size_request(eVMonL[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eVMonL[n], xoffset+xsize+xspace, yoffset+(ysize+yspace)*n);

    eVConL[n] = gtk_entry_new();
    gtk_widget_set_size_request(eVConL[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eVConL[n], xoffset+(xsize+xspace)*2, yoffset+(ysize+yspace)*n);

    eIMonL[n] = gtk_entry_new();
    gtk_widget_set_size_request(eIMonL[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eIMonL[n], xoffset+(xsize+xspace)*3, yoffset+(ysize+yspace)*n);

    eV0SetL[n] = gtk_entry_new();
    gtk_widget_set_size_request(eV0SetL[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eV0SetL[n], xoffset+(xsize+xspace)*4, yoffset+(ysize+yspace)*n);

    eI0SetL[n] = gtk_entry_new();
    gtk_widget_set_size_request(eI0SetL[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eI0SetL[n], xoffset+(xsize+xspace)*5, yoffset+(ysize+yspace)*n);

    eRupL[n] = gtk_entry_new();
    gtk_widget_set_size_request(eRupL[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eRupL[n], xoffset+(xsize+xspace)*6, yoffset+(ysize+yspace)*n);

    eRdwnL[n] = gtk_entry_new();
    gtk_widget_set_size_request(eRdwnL[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eRdwnL[n], xoffset+(xsize+xspace)*7, yoffset+(ysize+yspace)*n);

    ePwL[n] = gtk_entry_new();
    gtk_widget_set_size_request(ePwL[n], xsize, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), ePwL[n], xoffset+(xsize+xspace)*8, yoffset+(ysize+yspace)*n);

    eStatusL[n] = gtk_entry_new();
    gtk_widget_set_size_request(eStatusL[n], xsize+20, ysize);
    gtk_fixed_put(GTK_FIXED(fixed), eStatusL[n], xoffset+(xsize+xspace)*9, yoffset+(ysize+yspace)*n);

    if(n<10)sprintf(strChan,"0%i",n+2);
    else sprintf(strChan,"%i",n+2);
    strcpy(labelChan,"CH");
    strcat(labelChan,strChan);
    lChannelL[n]=gtk_label_new(labelChan);
    gtk_fixed_put(GTK_FIXED(fixed), lChannelL[n], 20, yoffset+5+(ysize+yspace)*n);
  }

  xspace += xsize;
  lVMonL =  gtk_label_new("VMon (V)");
  gtk_fixed_put(GTK_FIXED(fixed), lVMonL, lxoff+xspace, yoffset-lyoff);

  lVConL =  gtk_label_new("VCon (V)");
  gtk_fixed_put(GTK_FIXED(fixed), lVConL, lxoff+(xspace*2), yoffset-lyoff);

  lIMonL =  gtk_label_new("IMon (A)");
  gtk_fixed_put(GTK_FIXED(fixed), lIMonL, lxoff+(xspace*3), yoffset-lyoff);

  lV0SetL =  gtk_label_new("VSet (V)");
  gtk_fixed_put(GTK_FIXED(fixed), lV0SetL, lxoff+(xspace*4), yoffset-lyoff);

  lI0SetL =  gtk_label_new("ISet (A)");
  gtk_fixed_put(GTK_FIXED(fixed), lI0SetL, lxoff+(xspace*5), yoffset-lyoff);

  lRupL =  gtk_label_new("Rup (ms)");
  gtk_fixed_put(GTK_FIXED(fixed), lRupL, lxoff+(xspace*6), yoffset-lyoff);

  lRdwnL =  gtk_label_new("Rdwn (ms)");
  gtk_fixed_put(GTK_FIXED(fixed), lRdwnL, lxoff+(xspace*7), yoffset-lyoff);

  lPwL =  gtk_label_new("   Pw");
  gtk_fixed_put(GTK_FIXED(fixed), lPwL, lxoff+(xspace*8), yoffset-lyoff);

  lStatusL =  gtk_label_new("  Status");
  gtk_fixed_put(GTK_FIXED(fixed), lStatusL, lxoff+(xspace*9), yoffset-lyoff);

  /////////////////////////////////////////////////////////////////////
  // ComboBox for the LV Channel selection
  /////////////////////////////////////////////////////////////////////
  GList *cLVsel = NULL;
  cLVsel = g_list_append (cLVsel, "CH02");
  cLVsel = g_list_append (cLVsel, "CH03");
  cLVsel = g_list_append (cLVsel, "All ");
  cLVSelector=gtk_combo_new();
  gtk_widget_set_size_request(cLVSelector, 80, 30);
  gtk_combo_set_popdown_strings (GTK_COMBO(cLVSelector), cLVsel);
  gtk_fixed_put(GTK_FIXED(fixed), cLVSelector, 100, 750);
  

  /////////////////////////////////////////////////////////////////////
  // ComboBox for the LV parameter selection
  /////////////////////////////////////////////////////////////////////
  GList *cLVitems = NULL;
  cLVitems = g_list_append (cLVitems, "V0Set");
  cLVitems = g_list_append (cLVitems, "I0Set");
  cLVitems = g_list_append (cLVitems, "RUpTime");
  cLVitems = g_list_append (cLVitems, "RDwTime");
  cLVitems = g_list_append (cLVitems, "POWER ON");
  cLVitems = g_list_append (cLVitems, "POWER OFF");
  cLVCommand=gtk_combo_new(); 
  gtk_combo_set_popdown_strings (GTK_COMBO(cLVCommand), cLVitems);
  gtk_widget_set_size_request(cLVCommand, 150, 30);
  gtk_fixed_put(GTK_FIXED(fixed), cLVCommand,220, 750);

  GtkAdjustment *LVadjustment = (GtkAdjustment *) gtk_adjustment_new (5.0, 3.0, 500.0, 1.0, 1.0, 0.0);
  sbValueLVCommand = gtk_spin_button_new (LVadjustment, 1.0, 2);
  gtk_fixed_put(GTK_FIXED(fixed), sbValueLVCommand, 450, 750);

  bSendLV=gtk_button_new_with_label("Send");
  gtk_widget_set_size_request(bSendLV, 100, 30);
  gtk_fixed_put(GTK_FIXED(fixed), bSendLV, 600, 750);
  g_signal_connect(G_OBJECT(bSendLV), "clicked", G_CALLBACK(SendLVCommand), NULL);


  /////////////////////////////////////////////////////////////////////
  // Scrolled window for the logbook
  /////////////////////////////////////////////////////////////////////
  GtkWidget *scrolled_window = gtk_scrolled_window_new( NULL, NULL );
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_widget_set_size_request(scrolled_window,650,450);
    
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
  gtk_fixed_put(GTK_FIXED(fixed), scrolled_window, 850, 150);


  /////////////////////////////////////////////////////////////////////
  // Clear alarm cycle button
  ///////////////////////////////////////////////////////////////////// 
  int xbsize=100, ybsize=60;
  int xpos=900, ypos=680;
  bAlarm = gtk_button_new_with_label("Clear\nAlarm");
  gtk_widget_set_size_request(bAlarm, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bAlarm, xpos, ypos);
  gtk_widget_set_sensitive(bAlarm, FALSE);
  g_signal_connect(G_OBJECT(bAlarm), "clicked", G_CALLBACK(ClearAlarm), NULL);


  /////////////////////////////////////////////////////////////////////
  // START/STOP/QUIT buttons
  ///////////////////////////////////////////////////////////////////// 
  bStart = gtk_button_new_with_label("Start");
  gtk_widget_set_size_request(bStart, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStart, xpos+120, ypos); 
  gtk_widget_set_sensitive(bStart, TRUE);
  g_signal_connect(G_OBJECT(bStart), "clicked", G_CALLBACK(startMonitor), NULL);
  
  bStop = gtk_button_new_with_label("Stop");
  gtk_widget_set_size_request(bStop, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStop, xpos+240, ypos);
  gtk_widget_set_sensitive(bStop, FALSE);
  g_signal_connect(G_OBJECT(bStop), "clicked", G_CALLBACK(stopMonitor), NULL);
  
  bQuit = gtk_button_new_with_label("Quit");
  gtk_widget_set_size_request(bQuit, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bQuit, xpos+360, ypos);
  gtk_widget_set_sensitive(bQuit, TRUE);
  g_signal_connect(G_OBJECT(bQuit), "clicked", G_CALLBACK(quitMonitor), NULL);


  /////////////////////////////////////////////////////////////////////
  // Update time display
  /////////////////////////////////////////////////////////////////////
  lUpdateTime =  gtk_label_new("Last Update");
  gtk_fixed_put(GTK_FIXED(fixed), lUpdateTime, 845, 90);
  eUpdateTime = gtk_entry_new();
  gtk_widget_set_size_request(eUpdateTime, 170, 30); 
  gtk_fixed_put(GTK_FIXED(fixed), eUpdateTime, 845, 110);
  gtk_entry_set_editable(GTK_ENTRY(eUpdateTime), FALSE);
  gtk_widget_set_can_focus(GTK_WIDGET(eUpdateTime), FALSE);


  /////////////////////////////////////////////////////////////////////
  // INFN logo
  /////////////////////////////////////////////////////////////////////
  GtkWidget* image = gtk_image_new_from_file("Setup/images/logoINFN200x117.png");
  gtk_widget_set_size_request(image,145,80);
  gtk_fixed_put(GTK_FIXED(fixed), image, 1300, 20);


  return window;
}



/*************************************************************************************************************/
void ClearAlarm() {
/*************************************************************************************************************/
  GetTime(timeNow);

  int ret=CAENHV_ExecComm(handle,"ClearAlarm");
  if(ret) {
    sprintf(text,"%s ERROR clearing alarm \n", timeNow);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    printf(text);
  }
  else {
    sprintf(text,"%s Alarms cleared \n",timeNow);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
    printf(text);
  }

}



/*************************************************************************************************************/
void SendCommand() {
/*************************************************************************************************************/
  int ret;

  int SelectedChannel=-1;
  char* channel = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (cHVSelector)->entry));
  if(strcmp(channel,"CH00")==0)      SelectedChannel=0;
  else if(strcmp(channel,"CH01")==0) SelectedChannel=1;
  else if(strcmp(channel,"CH02")==0) SelectedChannel=2;
  else if(strcmp(channel,"CH03")==0) SelectedChannel=3;
  else if(strcmp(channel,"CH04")==0) SelectedChannel=4;
  else if(strcmp(channel,"CH05")==0) SelectedChannel=5;
  else if(strcmp(channel,"CH06")==0) SelectedChannel=6;
  else if(strcmp(channel,"CH07")==0) SelectedChannel=7;
  else if(strcmp(channel,"CH08")==0) SelectedChannel=8;
  else if(strcmp(channel,"CH09")==0) SelectedChannel=9;
  else if(strcmp(channel,"CH10")==0) SelectedChannel=10;
  else if(strcmp(channel,"All ")==0) SelectedChannel=50;

  char* strCommand = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (cCommand)->entry));

  int isPower=-1;
  if(strcmp(strCommand,"POWER ON")==0) isPower=1;
  else if(strcmp(strCommand,"POWER OFF")==0) isPower=0;

  float value = gtk_spin_button_get_value((gpointer) sbValueCommand);
  GetTime(timeNow);
  if(isPower==0)  sprintf(text,"%s Setting POWER OFF on CH%u \n",timeNow,SelectedChannel);
  else if(isPower==1)  sprintf(text,"%s Setting POWER ON on CH%u \n",timeNow,SelectedChannel);
  else sprintf(text,"%s Setting %s to value %.2f on CH%u \n",timeNow,strCommand,value,SelectedChannel);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "yellow_bg", NULL);
  printf(text);

  if(SelectedChannel==50) { //All channels
    for(int i=0; i<nHV; i++) {
      if(isPower>=0) Power[i]=isPower;
      else ParVal[i]=value;
    }
    if(isPower>=0) ret=CAENHV_SetChParam(handle,slotA1536,"Pw",nHV,HVList,Power);
    else ret=CAENHV_SetChParam(handle,slotA1536,strCommand,nHV,HVList,ParVal);
  }
  else {  //Single Channel
    unsigned short singleList[1]; 
    int singlePower[1]; 
    float singleParVal[1];
 
    singleList[0]=HVList[SelectedChannel]; 
    if(isPower>=0) {
      singlePower[0]=isPower;
      ret=CAENHV_SetChParam(handle,slotA1536,"Pw",1,singleList,singlePower);
    }
    else {
      singleParVal[0]=value;
      ret=CAENHV_SetChParam(handle,slotA1536,strCommand,1,singleList,singleParVal);
      //printf(" ret on CAENHV_SetChParam: %u \n", ret); 
    }

    printf(" singleList[0] %u    isPower  %i    singleParVal[0] %f \n",singleList[0],isPower ,singleParVal[0]);

  }

  updateAllHVParam=1; //Force monitor refresh

}




/*************************************************************************************************************/
void SendLVCommand() {
/*************************************************************************************************************/
  int ret;

  int SelectedChannel=-1;
  char* channel = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (cLVSelector)->entry));
  if(strcmp(channel,"CH02")==0)      SelectedChannel=0;
  else if(strcmp(channel,"CH03")==0) SelectedChannel=1;
  else if(strcmp(channel,"All ")==0) SelectedChannel=50;

  char* strCommand = (char*) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (cLVCommand)->entry));

  int isPower=-1;
  if(strcmp(strCommand,"POWER ON")==0) isPower=1;
  else if(strcmp(strCommand,"POWER OFF")==0) isPower=0;

  float value = gtk_spin_button_get_value((gpointer) sbValueLVCommand);
  
  GetTime(timeNow);
  if(isPower==0) sprintf(text,"%s Setting POWER OFF on CH%u - LV module\n",timeNow,SelectedChannel);
  else if(isPower==1) sprintf(text,"%s Setting POWER ON on CH%u - LV module\n",timeNow,SelectedChannel);
  else sprintf(text,"%s Setting %s to value %.2f on CH%u - LV module\n",timeNow,strCommand,value,SelectedChannel);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "yellow_bg", NULL);
  printf(text);

  if(SelectedChannel==50) { //All channels
    for(int i=0; i<nLV; i++) {
      if(isPower>=0) PowerL[i]=isPower;
      else ParValL[i]=value;
    }
    if(isPower>=0) ret=CAENHV_SetChParam(handle,slotA2518,"Pw",nLV,LVList,PowerL);
    else ret=CAENHV_SetChParam(handle,slotA2518,strCommand,nLV,LVList,ParValL);
    printf(" command: %s   nLV %u  ret on CAENHV_SetChParam: 0x%X \n", strCommand, nLV,ret); 

  }
  else {  //Single Channel
    unsigned short singleList[1]; 
    int singlePower[1]; 
    float singleParVal[1];
 
    singleList[0]=LVList[SelectedChannel]; 
    if(isPower>=0) {
      singlePower[0]=isPower;
      ret=CAENHV_SetChParam(handle,slotA2518,"Pw",1,singleList,singlePower);
      printf(" command: %s     ret on CAENHV_SetChParam: 0x%X \n", strCommand, ret); 
    }
    else {
      singleParVal[0]=value;
      ret=CAENHV_SetChParam(handle,slotA2518,strCommand,1,singleList,singleParVal);
      printf(" command: %s     ret on CAENHV_SetChParam: 0x%X \n", strCommand, ret); 
    }
    printf(" singleList[0] %u    isPower  %i    singleParVal[0] %f \n",singleList[0],isPower ,singleParVal[0]);
  }

  updateAllLVParam=1; //Force monitor refresh

}

/*************************************************************************************************************/
void startMonitor() {
/*************************************************************************************************************/
  gtk_widget_set_sensitive(bAlarm, TRUE);
  gtk_widget_set_sensitive(bStart, FALSE);
  gtk_widget_set_sensitive(bStop, TRUE);
  gtk_widget_set_sensitive(bQuit, TRUE);

  //running=1;
  gcalDaqSharedMemory->gcalStatus |= (0x1 << 0);

  updateAllHVParam=1;   //Force the refresh all parameters every time the monitor is started
  updateAllLVParam=1;

  GetTime(timeNow);
  sprintf(text,"%s Starting the BaF slow control \n",timeNow);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
  printf(text);

  while( (gcalDaqSharedMemory->gcalStatus>>0) & 0x1) {

    while (gtk_events_pending()) gtk_main_iteration();

    sleep(SlowControlSleepTime);

    // Clear HV/LV alarms before the new monitor cycle
    gcalDaqSharedMemory->gcalStatus &= 0x1; 

    HVmonitor();

    LVmonitor();

    GetTime(timeNow);
    gtk_entry_set_text((gpointer) eUpdateTime, (gpointer) timeNow);
  }

}



/*************************************************************************************************************/
void HVmonitor() {
/*************************************************************************************************************/
  int ret, alarmStatus=0;

  ret=CAENHV_GetChParam(handle,slotA1536,"VMon",nHV,HVList,ParVMon);
  if(ret) MonitorError("VMon-HV",ret);

  ret=CAENHV_GetChParam(handle,slotA1536,"IMon",nHV,HVList,ParIMon);
  if(ret) MonitorError("IMon-HV",ret);

  ret=CAENHV_GetChParam(handle,slotA1536,"Status",nHV,HVList,ParStatus);
  if(ret) MonitorError("Status-HV",ret);

  if(updateAllHVParam) {
    ret=CAENHV_GetChParam(handle,slotA1536,"V0Set",nHV,HVList,ParV0Set);
    if(ret) MonitorError("V0Set-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA1536,"I0Set",nHV,HVList,ParI0Set);
    if(ret) MonitorError("I0Set-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA1536,"RUp",nHV,HVList,ParRup);
    if(ret) MonitorError("RUp-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA1536,"RDWn",nHV,HVList,ParRdwn);
    if(ret) MonitorError("RDWn-HV",ret);

    ret=CAENHV_GetChParam(handle,slotA1536,"Trip",nHV,HVList,ParTrip);
    if(ret) MonitorError("Trip-HV",ret);
  }

  for(int n=0; n<nHV; n++) {
    sprintf(strEntry,"%4.2f",ParVMon[n]);
    gtk_entry_set_text((gpointer) eVMon[n],(gpointer) strEntry);
    
    sprintf(strEntry,"%4.3f",ParIMon[n]);
    gtk_entry_set_text((gpointer) eIMon[n],(gpointer) strEntry);

    if((ParStatus[n] >> 0) & 0x1)  gtk_entry_set_text((gpointer) ePw[n],(gpointer) "ON");
    else {
      gtk_entry_set_text((gpointer) ePw[n],(gpointer) "OFF");
      gcalDaqSharedMemory->gcalStatus |= (0x1 << 1);
      gcalDaqSharedMemory->gcalStatus |= (0x1 << (5+n)); 
      sprintf(text,"HVmonitor:  gcalStatus %i   ch %i off\n",gcalDaqSharedMemory->gcalStatus,n);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    alarmStatus = DecodeGCALStatus(ParStatus[n], status);
    gtk_entry_set_text((gpointer) eStatus[n],(gpointer) status);
    if(alarmStatus) { 
      gcalDaqSharedMemory->gcalStatus |= (alarmStatus << 2);
      gcalDaqSharedMemory->gcalStatus |= (0x1 << (5+n)); 
      sprintf(text,"HVmonitor: %s  alarmStatus %i   gcalStatus %i   ch %i\n",status,alarmStatus,gcalDaqSharedMemory->gcalStatus,n);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    if(updateAllHVParam) {
      sprintf(strEntry,"%4.2f",ParV0Set[n]);
      gtk_entry_set_text((gpointer) eV0Set[n],(gpointer) strEntry);

      sprintf(strEntry,"%4.2f",ParI0Set[n]);
      gtk_entry_set_text((gpointer) eI0Set[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",ParRup[n]);
      gtk_entry_set_text((gpointer) eRup[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",ParRdwn[n]);
      gtk_entry_set_text((gpointer) eRdwn[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.1f",ParTrip[n]);
      gtk_entry_set_text((gpointer) eTrip[n],(gpointer) strEntry);
    }
  }

  updateAllHVParam=0;

}




/*************************************************************************************************************/
void LVmonitor() {
/*************************************************************************************************************/
  int ret, alarmStatus=0;

  ret=CAENHV_GetChParam(handle,slotA2518,"VMon",nLV,LVList,ParVMonL);
  if(ret) MonitorError("VMon-LV",ret);

  ret=CAENHV_GetChParam(handle,slotA2518,"VCon",nLV,LVList,ParVConL);
  if(ret) MonitorError("Trip-LV",ret);

  ret=CAENHV_GetChParam(handle,slotA2518,"IMon",nLV,LVList,ParIMonL);
  if(ret) MonitorError("IMon-LV",ret);

  ret=CAENHV_GetChParam(handle,slotA2518,"Status",nLV,LVList,ParStatusL);
  if(ret) MonitorError("Status-LV",ret);

  if(updateAllLVParam) {
    ret=CAENHV_GetChParam(handle,slotA2518,"V0Set",nLV,LVList,ParV0SetL);
    if(ret) MonitorError("V0Set-LV",ret);

    ret=CAENHV_GetChParam(handle,slotA2518,"I0Set",nLV,LVList,ParI0SetL);
    if(ret) MonitorError("I0Set-LV",ret);

    ret=CAENHV_GetChParam(handle,slotA2518,"RUpTime",nLV,LVList,ParRupL);
    if(ret) MonitorError("RUp-LV",ret);

    ret=CAENHV_GetChParam(handle,slotA2518,"RDwTime",nLV,LVList,ParRdwnL);
    if(ret) MonitorError("RDWn-LV",ret);
  }

  for(int n=0; n<nLV; n++) {
    sprintf(strEntry,"%5.3f",ParVMonL[n]);
    gtk_entry_set_text((gpointer) eVMonL[n],(gpointer) strEntry);

    sprintf(strEntry,"%5.3f",ParVConL[n]);
    gtk_entry_set_text((gpointer) eVConL[n],(gpointer) strEntry);

    sprintf(strEntry,"%6.3f",ParIMonL[n]);
    gtk_entry_set_text((gpointer) eIMonL[n],(gpointer) strEntry);

    if ((ParStatusL[n] >> 0) & 0x1)  gtk_entry_set_text((gpointer) ePwL[n],(gpointer) "ON");
    else {
      gtk_entry_set_text((gpointer) ePwL[n],(gpointer) "OFF");
      gcalDaqSharedMemory->gcalStatus |= (0x1 << 17);
      gcalDaqSharedMemory->gcalStatus |= (0x1 << (21+n)); 
      sprintf(text,"LVmonitor:  gcalStatus %i   ch %i off\n",gcalDaqSharedMemory->gcalStatus,n);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    alarmStatus=DecodeGCALStatus(ParStatusL[n], status);
    gtk_entry_set_text((gpointer) eStatusL[n],(gpointer) status);
    if(alarmStatus) { 
      gcalDaqSharedMemory->gcalStatus |= (alarmStatus<<18);
      gcalDaqSharedMemory->gcalStatus |= (0x1 << (21+n)); 
      sprintf(text,"LVmonitor: %s   alarmStatus %i    gcalStatus %i   ch %i\n",status,alarmStatus,gcalDaqSharedMemory->gcalStatus,n);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
    }

    if(updateAllLVParam) {
      sprintf(strEntry,"%5.3f",ParV0SetL[n]);
      gtk_entry_set_text((gpointer) eV0SetL[n],(gpointer) strEntry);

      sprintf(strEntry,"%5.2f",ParI0SetL[n]);
      gtk_entry_set_text((gpointer) eI0SetL[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",ParRupL[n]);
      gtk_entry_set_text((gpointer) eRupL[n],(gpointer) strEntry);

      sprintf(strEntry,"%3.0f",ParRdwnL[n]);
      gtk_entry_set_text((gpointer) eRdwnL[n],(gpointer) strEntry);
    }
  }

  updateAllLVParam=0;

}



/*************************************************************************************************************/
int DecodeGCALStatus(int val, char *status) {
/*************************************************************************************************************/
  char mesg[20];
  int alarm=0;
      
  if(val==0 || val==1) sprintf(mesg,"%s"," ");
  if(val & (0x1 << 1)) sprintf(mesg,"%s", "RampUp");
  if(val & (0x1 << 2)) sprintf(mesg,"%s", "RampDown");
  if(val & (0x1 << 3)) sprintf(mesg,"%s", "OvCurrent");
  if(val & (0x1 << 4)) sprintf(mesg,"%s", "OverVoltage");
  if(val & (0x1 << 5)) sprintf(mesg,"%s", "UnderVoltage");
  if(val & (0x1 << 6)) sprintf(mesg,"%s", "ExternalTrip");
  if(val & (0x1 << 7)) sprintf(mesg,"%s", "OverVMAX");
  if(val & (0x1 << 8)) sprintf(mesg,"%s", "ExternalDisable");
  if(val & (0x1 << 9)) {
    sprintf(mesg,"%s", "Trip");
    alarm |= (0x1 << 0);
  }
  if(val & (0x1 << 10)) sprintf(mesg,"%s", "CalibError"); 
  if(val & (0x1 << 11)) sprintf(mesg,"%s", "Unplugged");
  if(val & (0x1 << 13)) sprintf(mesg,"%s", "OverVoltageProt");
  if(val & (0x1 << 14)) sprintf(mesg,"%s", "PowerFail");
  if(val & (0x1 << 15)) {
    sprintf(mesg,"%s", "Temperature");
    alarm |= (0x1 << 1);
  }

  mesg[20]='\0';
  strcpy(status, mesg);

  if((val>>3) !=0  && alarm==0) alarm |= (0x1 << 2);

  return alarm;
}




/*************************************************************************************************************/
void MonitorError(char parName[20], int err_code) {
/*************************************************************************************************************/
   sprintf(text," ERROR reading %s: code 0x%X \n",parName, err_code);
   gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
   printf(text);

}


/*************************************************************************************************************/
void stopMonitor() {
/*************************************************************************************************************/
  //running=0;
  gcalDaqSharedMemory->gcalStatus &= ~(0x1 << 0);


  gtk_widget_set_sensitive(bAlarm, FALSE);
  gtk_widget_set_sensitive(bStop, FALSE);
  gtk_widget_set_sensitive(bStart, TRUE);

  GetTime(timeNow);
  sprintf(text,"%s Stopping the BaF slow control \n",timeNow);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "red_bg", NULL);
  printf(text);

}



/*************************************************************************************************************/
void quitMonitor() {
/*************************************************************************************************************/
  //running=0;
  //gcalDaqSharedMemory->gcalStatus |= (0x0 << 0);
  gcalDaqSharedMemory->gcalStatus &= ~(0x1 << 0);

  CAENHV_DeinitSystem(handle);
  GetTime(timeNow);
  sprintf(text,"%s Connection closed with the SY5527 mainframe \n",timeNow);
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "blue_fg", NULL);
  printf(text);


  fflush(stdout);
  fclose(stdout);  // close logFile

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
void CrateMap(int handle) {
/*************************************************************************************************************/

  unsigned short NrOfSl, *SerNumList, *NrOfCh;
  char                   *ModelList, *DescriptionList;
  unsigned char          *FmwRelMinList, *FmwRelMaxList;
      
  int ret = CAENHV_GetCrateMap(handle, &NrOfSl, &NrOfCh, &ModelList, &DescriptionList, &SerNumList,
                                       &FmwRelMinList, &FmwRelMaxList );
      
  int  ii;
  char *m = ModelList, *d = DescriptionList;

  sprintf(text,"             *** SY5527 CRATE MAP ***\n");
  gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "yellow_bg", NULL);
  printf(text);

  for(ii = 0; ii < NrOfSl; ii++ , m += strlen(m) + 1, d += strlen(d) + 1 ) {
    if( *m == '\0' ) {
      sprintf(text,"Board %2d: Not Present\n", ii);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "yellow_bg", NULL);
      printf(text);
    }
    else {
      sprintf(text,"Board %2d: %s %s  Nr. Ch: %d  Ser. %d   Rel. %d.%d\n",
                 ii, m, d, NrOfCh[ii], SerNumList[ii], FmwRelMaxList[ii],FmwRelMinList[ii]);
      gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,text, -1, "lmarg", "yellow_bg", NULL);
      printf(text);
    }
   }

  free(SerNumList); free(ModelList); free(DescriptionList); free(FmwRelMinList); free(FmwRelMaxList); free(NrOfCh);
}
