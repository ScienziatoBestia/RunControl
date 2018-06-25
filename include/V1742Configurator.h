#ifndef V1742_CONFIGURATION
#define V1742_CONFIGURATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CAENDigitizer.h>
#include "CAENDigitizerType.h"

#define MaxV1742NGroups 4
#define MaxV1742NChannels 32

typedef struct {
  int LinkType;
  int LinkNum;
  int ConetNode;
  uint32_t BaseAddress;
  int NumEvents;
  uint32_t RecordLength;
  int PostTrigger;
  int InterruptNumEvents;
  int TriggerEdge;
  CAEN_DGTZ_IOLevel_t FPIOtype;
  CAEN_DGTZ_TriggerMode_t ExtTriggerMode;
  uint16_t GroupEnableMask;
  int32_t DCOffset[MaxV1742NChannels];
  uint32_t FastTriggerDCOffset[MaxV1742NGroups];
  uint32_t FastTriggerThreshold[MaxV1742NGroups];
  CAEN_DGTZ_TriggerMode_t	FastTriggerMode;
  uint32_t	 FastTriggerEnabled;
  CAEN_DGTZ_DRS4Frequency_t DRS4Frequency;
} V1742Params_t;

void WriteConfigFileV1742(char* filename, V1742Params_t *Params);
int ParseConfigFileV1742(char* filename, V1742Params_t *V1742Config);
int ProgramDigitizerV1742(int connectionParams[4], char* filename, int isCalo);
#endif
