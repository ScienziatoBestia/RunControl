#ifndef SLOW_CONTROL
#define SLOW_CONTROL

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
#include "CheckRunningProcesses.h"

#define nbafHV 1
#define nbafLV 2

#define nsistripHV 1
#define nsistripLV 4

#define ngcalHV 11
#define ngcalLV 2

#define nrssHV 8


typedef struct {
  unsigned short HVList[nbafHV];
  unsigned short LVList[nbafLV];

  int ParStatus[nbafHV], ParStatusL[nbafLV];

  float ParVMon[nbafHV],ParIMon[nbafHV], ParV0Set[nbafHV], ParI0Set[nbafHV], ParRup[nbafHV], ParRdwn[nbafHV], ParTrip[nbafHV];
  float ParVMonL[nbafLV],ParIMonL[nbafLV], ParV0SetL[nbafLV], ParI0SetL[nbafLV], ParRupL[nbafLV], ParRdwnL[nbafLV], ParVConL[nbafLV];

  GtkWidget *eVMon[nbafHV], *eIMon[nbafHV], *eV0Set[nbafHV], *eI0Set[nbafHV], *ePw[nbafHV], *eStatus[nbafHV], *eRup[nbafHV], *eRdwn[nbafHV], *eTrip[nbafHV];
  GtkWidget *eVMonL[nbafLV], *eVConL[nbafLV], *eIMonL[nbafLV], *eV0SetL[nbafLV], *eI0SetL[nbafLV], *ePwL[nbafLV], *eStatusL[nbafLV], *eRupL[nbafLV], *eRdwnL[nbafLV];

  GtkWidget *sbHVvalue, *sbLVvalue;

  GtkWidget *cHVCommand, *cHVSelector, *cLVCommand, *cLVSelector;

} bafWidget_t;



typedef struct {
  unsigned short HVList[nsistripHV];
  unsigned short LVList[nsistripLV];

  int ParStatus[nsistripHV], ParStatusL[nsistripLV];

  float ParVMon[nsistripHV],ParIMon[nsistripHV], ParV0Set[nsistripHV], ParI0Set[nsistripHV], ParRup[nsistripHV], ParRdwn[nsistripHV], ParTrip[nsistripHV];
  float ParVMonL[nsistripLV],ParIMonL[nsistripLV], ParV0SetL[nsistripLV], ParI0SetL[nsistripLV], ParRupL[nsistripLV], ParRdwnL[nsistripLV], ParVConL[nsistripLV];

  GtkWidget *eVMon[nsistripHV], *eIMon[nsistripHV], *eV0Set[nsistripHV], *eI0Set[nsistripHV], *ePw[nsistripHV], *eStatus[nsistripHV], *eRup[nsistripHV], *eRdwn[nsistripHV], *eTrip[nsistripHV];
  GtkWidget *eVMonL[nsistripLV], *eVConL[nsistripLV], *eIMonL[nsistripLV], *eV0SetL[nsistripLV], *eI0SetL[nsistripLV], *ePwL[nsistripLV], *eStatusL[nsistripLV], *eRupL[nsistripLV], *eRdwnL[nsistripLV];

  GtkWidget *sbHVvalue, *sbLVvalue;

  GtkWidget *cHVCommand, *cHVSelector, *cLVCommand, *cLVSelector;

} sistripWidget_t;


typedef struct {
  unsigned short HVList[ngcalHV];
  unsigned short LVList[ngcalLV];

  int ParStatus[ngcalHV], ParStatusL[ngcalLV];

  float ParVMon[ngcalHV],ParIMon[ngcalHV], ParV0Set[ngcalHV], ParI0Set[ngcalHV], ParRup[ngcalHV], ParRdwn[ngcalHV], ParTrip[ngcalHV];
  float ParVMonL[ngcalLV],ParIMonL[ngcalLV], ParV0SetL[ngcalLV], ParI0SetL[ngcalLV], ParRupL[ngcalLV], ParRdwnL[ngcalLV], ParVConL[ngcalLV];

  GtkWidget *eVMon[ngcalHV], *eIMon[ngcalHV], *eV0Set[ngcalHV], *eI0Set[ngcalHV], *ePw[ngcalHV], *eStatus[ngcalHV], *eRup[ngcalHV], *eRdwn[ngcalHV], *eTrip[ngcalHV];
  GtkWidget *eVMonL[ngcalLV], *eVConL[ngcalLV], *eIMonL[ngcalLV], *eV0SetL[ngcalLV], *eI0SetL[ngcalLV], *ePwL[ngcalLV], *eStatusL[ngcalLV], *eRupL[ngcalLV], *eRdwnL[ngcalLV];

  GtkWidget *sbHVvalue, *sbLVvalue;

  GtkWidget *cHVCommand, *cHVSelector, *cLVCommand, *cLVSelector;

} gcalWidget_t;


typedef struct {
  unsigned short HVList[nrssHV];

  int ParStatus[nrssHV];

  float ParVMon[nrssHV],ParIMon[nrssHV], ParV0Set[nrssHV], ParI0Set[nrssHV], ParRup[nrssHV], ParRdwn[nrssHV], ParTrip[nrssHV];

  GtkWidget *eVMon[nrssHV], *eIMon[nrssHV], *eV0Set[nrssHV], *eI0Set[nrssHV], *ePw[nrssHV], *eStatus[nrssHV], *eRup[nrssHV], *eRdwn[nrssHV], *eTrip[nrssHV];

  GtkWidget *sbHVvalue;

  GtkWidget *cHVCommand, *cHVSelector;

} nrssWidget_t;


// Slot number for HV/LV boards in the SY5527 mainframe
unsigned short slotA7030, slotA2518, slotA1536;


// main
void create_main_window();
void setupFrameBaF();
void setupFrameSiStrip();
void setupFrameGCAL();
void setupFrameNRSS();
void startSC();
void stopSC();
void exitSC();
void UpdateLogFile();
void ClearCrateAlarm();
void CrateMap(int);
void GetTime(char timeNow[21]);
void ApplyCommand(int, int, char command[30], float);
int DecodeStatus(int, char status[20]);
void maskBaF();
void maskSiStrip();
void maskGCAL();
void maskNRSS();

//BaF
void bafHVmonitor();
void bafLVmonitor();
void bafSendHVCommand();
void bafSetHVMax();
void bafSendLVCommand();

//SiStrip
void sistripHVmonitor();
void sistripLVmonitor();
void sistripSendHVCommand();
void sistripSendLVCommand();

// GCAL
void gcalHVmonitor();
void gcalLVmonitor();
void gcalSendHVCommand();
void gcalSendLVCommand();
void MonitorError(char parName[20], int);

// NRSS()
void nrssSendHVCommand();
void nrssHVmonitor();
void nrssSetHVMax();

#endif
