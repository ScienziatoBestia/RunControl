#ifndef DT5780_CONFIGURATION
#define DT5780_CONFIGURATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CAENDigitizer.h>
#include "CAENDigitizerType.h"

#define MaxDT5780NChannels 2

typedef enum  {
  INDIVIDUAL,
  COINCIDENCE,
  ANTICOINCIDENCE,
} TriggerModeDT5780;

typedef struct
{
  CAEN_DGTZ_ConnectionType LinkType;
  int LinkNum;
  uint32_t RecordLength;
  uint32_t ChannelMask;
  uint32_t TriggerOut;
  CAEN_DGTZ_DPP_AcqMode_t AcqMode;
  CAEN_DGTZ_IOLevel_t IOLevel;
  int ExtTrigger;
  int PreTriggerSize;
  int InputRange[MaxDT5780NChannels];
  int DCOffset[MaxDT5780NChannels];
  int PulsePolarity[MaxDT5780NChannels];
  int SelfTrigger[MaxDT5780NChannels];
  TriggerModeDT5780 TrgMode[MaxDT5780NChannels];
} DT5780Params_t;

int ParseConfigFileDT5780(char* filename, DT5780Params_t *Params,CAEN_DGTZ_DPP_PHA_Params_t *DPPParams );
void WriteConfigFileDT5780(char* filename, DT5780Params_t *Params,CAEN_DGTZ_DPP_PHA_Params_t *DPPParams );

int  ProgramDigitizerDT5780(int connectionParams[4], char* filename);

#endif
