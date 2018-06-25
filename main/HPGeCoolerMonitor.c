#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "rs232.h"

GtkWidget* create_main_window(void);
void startMonitor();
void stopMonitor();
void quitMonitor();

GtkWidget *main_window;
GtkWidget *eMonitor[8], *eUpdateTime; //entry
GtkWidget *tMonitor;    //table
GtkWidget *lColdTipT, *lCoolerStatus, *lColdHeadT, *lCompressorT, *lBoardT, *lFaultStatus, *lHVinhibit, *lPower, *lUpdateTime;  //labels
GtkWidget *bStart, *bStop, *bQuit;  //buttons

int n=0, nCMD=8, ready;
int cport_nr=16;
int bdrate=9600;       
unsigned char buf[4096];
char mode[]={'8','N','1',0};

time_t now;
struct tm *tm;
char bufTime[20];
char mesgTime[33];

int running=0;

int main(int argc, char* argv[]) {

  GtkWidget* main_window;
  gtk_init(&argc, &argv);
  main_window = create_main_window();
  gtk_widget_show_all(main_window);

  if(RS232_OpenComport(cport_nr, bdrate, mode)) {
    printf("Can not open comport %i\n",cport_nr);
    return(0);
  } 

  gtk_main();

return 0;

}

void startMonitor() {
  int i,j,first=0;
  char str[10][512];

  running=1;

  gtk_widget_set_sensitive(bStart, FALSE);
  gtk_widget_set_sensitive(bStop, TRUE);
  gtk_widget_set_sensitive(bQuit, TRUE);

  strcpy(str[0], "COOLER\r\n");
  strcpy(str[1], "TEMPC\r\n");
  strcpy(str[2], "TWE\r\n");
  strcpy(str[3], "TCO\r\n");
  strcpy(str[4], "TBRD\r\n");
  strcpy(str[5], "ERROR\r\n");
  strcpy(str[6], "HVINH\r\n");
  strcpy(str[7], "PWR\r\n");

  while(running) {
    if(first) sleep(20);
    else first=1;

    while (gtk_events_pending()) gtk_main_iteration();

    for(i=0; i<nCMD; i++) {
      RS232_cputs(cport_nr, str[i]);
      usleep(100000);  /* sleep for 100 milliSeconds */      
      ready=0;
      while(n==0 || ready==0)  {
        n = RS232_PollComport(cport_nr, buf, 4095);
        if(n > 0 ) {
          buf[n] = 0;   // always put a "null" at the end of a string
          for(j=0; j<n; j++) { // replace unreadable control-codes by empty spaces
            if(buf[j]<32) buf[j] = ' ';
          }
          //printf(" %s:   %s\n", str[i], (char *)buf);
          gtk_entry_set_text((gpointer) eMonitor[i], (gpointer) buf);
          ready = 1;
        }    
      }
    }

    now = time(NULL);
    tm = localtime(&now);
    strftime(bufTime, sizeof(bufTime), "%d-%m-%Y %H:%M:%S", tm);
    sprintf(mesgTime,"Last update: %s\n", bufTime);
    mesgTime[32]='\0';
    gtk_entry_set_text((gpointer) eUpdateTime, (gpointer) mesgTime); 
  }  
 
}


/* Create the main window */
GtkWidget* create_main_window(void) {

  GdkColor color; // 16-bit are used. Multiply RGB values (range 1-255) by 65535/255 
  color.red = 0x9999;
  color.green = 0xFFFF;
  color.blue = 0x3333;

  main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(main_window), "HPGe COOLER MONITOR");
  gtk_window_set_default_size(GTK_WINDOW(main_window), 100, 300);
  gtk_container_set_border_width(GTK_CONTAINER(main_window), 10);
  gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
  gtk_widget_modify_bg(main_window, GTK_STATE_NORMAL, &color);
  GtkWidget *fixed = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(main_window), fixed);

  int xpos=185, ypos=30;
  tMonitor=gtk_table_new (9,2,TRUE);
  gtk_fixed_put(GTK_FIXED(fixed), tMonitor, xpos, ypos);

  int et, xe=60, ye=30;
  for(et=0;et<nCMD;et++) {
    eMonitor[et] = gtk_entry_new();
    gtk_widget_set_size_request(eMonitor[et], xe, ye);
    gtk_table_attach_defaults (GTK_TABLE(tMonitor), eMonitor[et], 0, 1, et, et+1);
  }
  xpos=25; ypos+=10;
  GtkWidget* lCoolerStatus = gtk_label_new("Cooler Status (ON/OFF)");
  gtk_fixed_put(GTK_FIXED(fixed), lCoolerStatus, xpos, ypos);

  ypos += ye;
  GtkWidget* lColdTipT = gtk_label_new("Cold-tip temperature (C)");
  gtk_fixed_put(GTK_FIXED(fixed), lColdTipT, xpos, ypos);

  ypos += ye;
  GtkWidget* lColdHeadT = gtk_label_new("Cold-head temperature (C)");
  gtk_fixed_put(GTK_FIXED(fixed), lColdHeadT, xpos, ypos);

  ypos += ye;
  GtkWidget* lCompressorT = gtk_label_new("Compressor temperature (C)");
  gtk_fixed_put(GTK_FIXED(fixed), lCompressorT, xpos, ypos);

  ypos += ye;
  GtkWidget* lBoardT = gtk_label_new("Board Temperature (C)");
  gtk_fixed_put(GTK_FIXED(fixed), lBoardT, xpos, ypos);

  ypos += ye;
  GtkWidget* lFaultStatus = gtk_label_new("Fault status");
  gtk_fixed_put(GTK_FIXED(fixed), lFaultStatus, xpos, ypos);

  ypos += ye;
  GtkWidget* lHVinhibit = gtk_label_new("HV inhibit status (ON/OFF)");
  gtk_fixed_put(GTK_FIXED(fixed), lHVinhibit, xpos, ypos);

  ypos += ye;
  GtkWidget* lPower = gtk_label_new("Power (W)");
  gtk_fixed_put(GTK_FIXED(fixed), lPower, xpos, ypos);


  ypos += ye;
  eUpdateTime = gtk_entry_new();
  gtk_widget_set_size_request(eUpdateTime, 220, ye);
  gtk_fixed_put(GTK_FIXED(fixed), eUpdateTime, xpos, ypos);

  //GtkWidget* lUpdateTime = gtk_label_new("Last update");
  //gtk_fixed_put(GTK_FIXED(fixed), lUpdate, xpos, ypos);


// START/STOP/QUIT buttons
  int xbsize=70;
  int ybsize=40;
  ypos += 2*ye;
  bStart = gtk_button_new_with_label("Start");
  gtk_widget_set_size_request(bStart, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStart, 25, ypos);
  gtk_widget_set_sensitive(bStart, TRUE);
  g_signal_connect(G_OBJECT(bStart), "clicked", G_CALLBACK(startMonitor), NULL);  

  bStop = gtk_button_new_with_label("Stop");
  gtk_widget_set_size_request(bStop, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bStop, 100, ypos);
  gtk_widget_set_sensitive(bStop, FALSE);
  g_signal_connect(G_OBJECT(bStop), "clicked", G_CALLBACK(stopMonitor), NULL);  

  bQuit = gtk_button_new_with_label("Quit");
  gtk_widget_set_size_request(bQuit, xbsize,ybsize);
  gtk_fixed_put(GTK_FIXED(fixed), bQuit, 200, ypos);
  gtk_widget_set_sensitive(bQuit, TRUE);
  g_signal_connect(G_OBJECT(bQuit), "clicked", G_CALLBACK(quitMonitor), NULL);  

  return main_window;
}


void stopMonitor() {
  running=0;
  gtk_widget_set_sensitive(bStop, FALSE);
  gtk_widget_set_sensitive(bStart, TRUE);
}

void quitMonitor() {
  running=0;
  gtk_main_quit();
}

