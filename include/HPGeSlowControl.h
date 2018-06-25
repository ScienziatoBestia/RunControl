#ifndef HPGE_MONITOR
#define HPGE_MONITOR

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
#include "rs232.h"
#include <CAENDigitizer.h>
#include "CAENDigitizerType.h"
#include "../include/SharedMemorySetup.h"

#define HV_VSET_ADDR        (0x1020)
#define HV_ISET_ADDR        (0x1024)
#define HV_RAMPUP_ADDR      (0x1028)
#define HV_RAMPDOWN_ADDR    (0x102C)
#define HV_VMAX_ADDR        (0x1030)
#define HV_VMON_ADDR        (0x1040)
#define HV_IMON_ADDR        (0x1044)
#define HV_POWER_ADDR       (0x1034)
#define HV_STATUS_ADDR      (0x1038)
#define HV_CONTROL_ADDR     HV_POWER_ADDR

#define HV_REGBIT_PWDOWN        (1)
#define HV_PWDOWN_BITVALUE_RAMP (1)
#define HV_PWDOWN_BITVALUE_KILL (0)

typedef enum
{
    HighVoltage_PWDown_Kill = 0L,   
    HighVoltage_PWDown_Ramp = 1L,   
} HighVoltage_PWDown_t;


// The following structure contains informations to convert
// the HighVoltage parameters values from Units (Volts, uAmpere, etc.)
// to LSB (value to be written on board register).
typedef struct
{
    double Min; // Units
    double Max; // Units
    double Res; // Units/LSB
} HighVoltageParameterInfos_t;


typedef struct
{
    HighVoltageParameterInfos_t Infos;
    double Value; // Units
} HighVoltageParameter_t;
    

typedef struct
{
    HighVoltageParameter_t VSet;
    HighVoltageParameter_t ISet;
    HighVoltageParameter_t VMax;    
    HighVoltageParameter_t RampDown;
    HighVoltageParameter_t RampUp;
    HighVoltage_PWDown_t PWDownMode;
} HighVoltageParams_t;


GtkWidget* create_main_window(void);
void GetTime(char timeNow[21]);
void maskHPGe();
void SwitchOnHPGe();
void startMonitor();
void stopMonitor();
void quitMonitor();
void SendCommand();
void hpgeHVMonitor(uint32_t);
//void ConfigureParams(HighVoltageParams_t HVParams[]);
void ConfigureParams();
uint32_t HighVoltageUnitsToLSB(HighVoltageParameter_t *parameter);
void FillHVParameterDT5780(HighVoltageParams_t *Params);
void updateLogFile();

#endif
