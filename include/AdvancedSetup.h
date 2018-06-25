#ifndef ADVANCED_SETUP
#define ADVANCED_SETUP
#include "../include/Define.h"


typedef struct 
{
  GtkWidget *sbV1495LinkNum;
  GtkWidget* sbV1495BaseAddress;
} V1495Widget_t;


typedef struct 
{
  GtkWidget *sbV812LinkNum;
  GtkWidget* sbV812BaseAddress;
  GtkWidget* sbV812Majority;
  GtkWidget *sbV812DeadTime[MaxV812NGroups];
  GtkWidget *sbV812OutputWidth[MaxV812NGroups];
  GtkWidget *sbV812Threshold[MaxV812NChannels];
  GtkWidget *cV812Channel[MaxV812NChannels];
} V812Widget_t;


typedef struct 
{
  GtkWidget *sbDT5780LinkNum;
  GtkWidget *cbDT5780LinkType;
  GtkWidget *sbDT5780RecordLength;
  GtkWidget *sbDT5780PreTriggerSize;
  GtkWidget *cDT5780ExternalTrigger;
  GtkWidget *cbDT5780AcquisitionMode;
  GtkWidget *cbDT5780IOLevel;
  GtkWidget *cbDT5780ChannelTriggerMode;
  GtkWidget *cbDT5780ChannelEnable[MaxDT5780NChannels];
  GtkWidget *cbDT5780Polarity[MaxDT5780NChannels];
  GtkWidget *sbDT5780TriggerThreshold[MaxDT5780NChannels];
  GtkWidget *sbDT5780TrapezoidRiseTime[MaxDT5780NChannels];
  GtkWidget *sbDT5780TrapezoidFlatTop[MaxDT5780NChannels];
  GtkWidget *sbDT5780DecayTime[MaxDT5780NChannels];
  GtkWidget *sbDT5780PeakingTime[MaxDT5780NChannels];
  GtkWidget *cbDT5780TriggerSmoothing[MaxDT5780NChannels];
  GtkWidget *sbDT5780InputRiseTime[MaxDT5780NChannels];
  GtkWidget *sbDT5780TriggerHoldOff[MaxDT5780NChannels];
  GtkWidget *sbDT5780PeakHoldOff[MaxDT5780NChannels];
  GtkWidget *sbDT5780BaselineHoldOff[MaxDT5780NChannels];
  GtkWidget *cbDT5780InputRange[MaxDT5780NChannels];
  GtkWidget *sbDT5780DCOffset[MaxDT5780NChannels];
  GtkWidget *cbDT5780SelfTrigger[MaxDT5780NChannels];
  GtkWidget *cbDT5780TriggerMode[MaxDT5780NChannels];
} DT5780Widget_t;

typedef struct 
{
  GtkWidget *sbV1742LinkNum[MaxV1742];
  GtkWidget *sbV1742PostTrigger[MaxV1742];
  GtkWidget *cbV1742Frequency[MaxV1742];
  GtkWidget *cbV1742FastTrgDigitizing[MaxV1742];
  GtkWidget *cbV1742TriggerEdge[MaxV1742];
  GtkWidget *cV1742GroupEnable[MaxV1742NGroups][MaxV1742];
  GtkWidget *sbV1742DCOffset[MaxV1742NChannels][MaxV1742];
  GtkWidget *sbV1742FastTriggerOffset[MaxV1742NGroups/2][MaxV1742];
  GtkWidget *sbV1742FastTriggerThreshold[MaxV1742NGroups/2][MaxV1742];

} V1742Widget_t;


typedef struct 
{
  GtkWidget *sbDT5743LinkNum;
  GtkWidget *cbDT5743LinkType;
  GtkWidget *cbDT5743AcquisitionMode;
  GtkWidget *sbDT5743RecordLength;
  GtkWidget *sbDT5743PosTrigger;
  GtkWidget *cbDT5743IOLevel;
  GtkWidget *cDT5743TestPattern;
  GtkWidget *cDT5743ExternalTrigger;
  GtkWidget *cbDT5743TriggerMode;
  GtkWidget *cDT5743TriggerOut;
  GtkWidget *sbDT5743TriggerGate;
  GtkWidget *sbDT5743TriggerPairLogic;
  GtkWidget *sbDT5743GlobalTriggerLogic;
  GtkWidget *sbDT5743GroupMask;

  GtkWidget *sbDT5743DCOffset[MaxDT5743NChannels];
  GtkWidget *cbDT5743SelfTrigger[MaxDT5743NChannels];
  GtkWidget *sbDT5743TriggerThreshold[MaxDT5743NChannels];
  GtkWidget *sbDT5743ChThres[MaxDT5743NChannels];
  GtkWidget *sbDT5743ChRefCell[MaxDT5743NChannels];
  GtkWidget *sbDT5743ChLenght[MaxDT5743NChannels];

} DT5743Widget_t;


char* getCurrentRecipe(GtkWidget* combo);
char* getConfigFile(GtkWidget* combo, Device_t device);
void FillRecipeList(GtkWidget* combo, char* stractive, int isFirst);
void openAdvancedSetup();

#endif
