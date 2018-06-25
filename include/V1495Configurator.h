#ifndef V1495_CONFIGURATION
#define V1495_CONFIGURATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CAENVMElib.h>

////////////////////////////////////////////////////////////////////////////////////////////////
// Registers address
////////////////////////////////////////////////////////////////////////////////////////////////

#define REG_V1495_SOFTTRG      0x1018  // Software trigger register: write 1 to bit  9 to start one software trigger 
                                       //                            write 1 to bit 10 to trigger the pulser
#define DATA_V1495_SENDSOFTTRG 0x201
#define DATA_V1495_SENDEXTTRG  0x401
#define REG_V1495_DATAREADY    0x1030  // Data register ready: if bit 0 = 1 data are ready 

typedef struct
{
  int LinkNum;
  uint32_t BaseAddress;
} V1495Params_t;



void WriteConfigFileV1495(char* filename, V1495Params_t *Params);
void ParseConfigFileV1495(char* filename, V1495Params_t *Params);
int TestConnection(V1495Params_t *Params);

#endif
