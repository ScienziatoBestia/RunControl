#ifndef GCAL_MONITOR
#define GCAL_MONITOR

#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <CAENDigitizer.h>
#include "CAENDigitizerType.h"
#include <CAENHVWrapper.h>
#include "rs232.h"
#include "SharedMemorySetup.h"
#include "Define.h"

GtkWidget* create_main_window();
void SendCommand();
void SendLVCommand();
void startMonitor();
void stopMonitor();
void quitMonitor();
void GetTime(char timeNow[21]);
void HVmonitor();
void LVmonitor();
void MonitorError(char parName[20], int);
int DecodeGCALStatus(int, char status[20]);
void ClearAlarm();
void CrateMap(int);

#endif
