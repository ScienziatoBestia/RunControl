#ifndef RUN_CONTROL
#define RUN_CONTROL

#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <string.h>

#include "V1742Configurator.h"
#include "V812Configurator.h"
#include "DT5780Configurator.h"
#include "DT5743Configurator.h"
#include "V1495Configurator.h"
#include "AdvancedSetup.h"
#include "SharedMemorySetup.h"
#include "UtilsFunctions.h"
#include "Define.h"
#include "CheckRunningProcesses.h"

GtkWidget* create_main_window();
void WriteLogHeader();
void loadRecipeList();
void changeRecipe();
void updateRunNumber(int);
void updateChannelToPlot();
void updatePlotScaler();
void updateRate();
void updateMonitor();
void checkIncludedDevice();
void checkRunNumber();
void CheckTestHPGe();
void selectRunType();
void selectPlotType();
void startPlotter(char* device);
void checkGCALplotter();
void checkCSPECplotter();
void checkNRSSplotter();
void startConsumer(char* device);
void startProducer(char* device);
void ConfigDigi();
void StartSlowControl();
void StartDaq();
void StopDaq();
void Quit();
void GetTime(char*);
void DecodeSlowControlStatus(Device_t detector);
//int CheckRunningProcesses(process_t caller);

#endif
