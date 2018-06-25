#ifndef V812_CONFIGURATION
#define V812_CONFIGURATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CAENVMElib.h>

#define MaxV812NChannels 16
#define MaxV812NGroups 2

////////////////////////////////////////////////////////////////////////////////////////////////
// Registers address
////////////////////////////////////////////////////////////////////////////////////////////////

#define REG_V812_THRESHOLD_CH0   0x0000  /* Threshold Ch0 register relative address */
#define V812_THRESHOLD_STEP	      2	 /* Threshold registers address step for V812 */
#define REG_V812_OUTWIDTH_0_7	 0x0040	 /* Output width register Ch 0-7 relative address */ 
#define REG_V812_OUTWIDTH_8_15	 0x0042	 /* Output width register Ch 8-15 relative address */
#define REG_V812_DEADTIME_0_7	 0x0044	 /* Dead time register Ch 0-7 relative address */
#define REG_V812_DEADTIME_8_15	 0x0046	 /* Dead time register Ch 8-15 relative address */
#define REG_V812_MAJORITY	 0x0048	 /* Majority threshold register relative address */
#define REG_V812_INHIBITPATTERN  0x004A	 /* Pattern inhibit register relative address */
#define REG_V812_TESTPULSE	 0x004C	 /* Test pulse register relative address */
#define REG_V812_FIXEDCODE	 0x00FA	 /* Fixed code register relative address */
#define REG_V812_MANUFACTURER	 0x00FC	 /* Manufacturer and Module type register relative address */
#define REG_V812_VERSION	 0x00FE	 /* Version and serial number register relative address */



typedef struct
{
  int LinkNum;
  uint32_t BaseAddress;
  int Majority;
  uint32_t InhibitPattern; 
  uint8_t Threshold[MaxV812NChannels];
  uint8_t OutputWidth[MaxV812NGroups];
  uint8_t DeadTime[MaxV812NGroups];
} V812Params_t;

void WriteConfigFileV812(char* filename, V812Params_t *Params);
void ParseConfigFileV812(char* filename, V812Params_t *Params);
int  ProgramV812(char* filename);


#endif
