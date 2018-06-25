#ifndef _SHAREDMEMORYSETUP__H
#define _SHAREDMEMORYSETUP__H

#include <sys/shm.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "Define.h"

typedef struct {
  int stopDAQ;
  int stopCSPECproducer;
  int stopGCALproducer;
  int stopNRSSproducer;
  int stopCSPECconsumer;
  int stopGCALconsumer;
  int stopNRSSconsumer;
  int stopCSPECplotter;
  int stopGCALplotter;
  int stopNRSSplotter;
  int softwareTrigger;
  int testHPGe;
  int SoftwareTrgRate;
  int IncludeBaF; 
  int IncludeSiStrip; 
  int IncludeHPGe;
  int IncludeGCAL; 
  int IncludeNRSS; 
  int connectionParamsV1742BaF[4];
  int connectionParamsV1742GCAL[4];
  int connectionParamsV1495[2];
  int connectionParamsDT5780[4];
  int connectionParamsDT5743[4];
  int runNumber;
  int NumberOfEvents;
  int RunTimeLength;
  int PlotChannelBaF;
  int PlotChannelHPGe;
  int PlotChannelGCAL;
  int PlotChannelNRSS;
  int CSPECplotScaler;
  int GCALplotScaler;
  int NRSSplotScaler;
  int isGCALplotGroup;
  int isBaFplotGroup;
  int isNRSSplotGroup;
  int AlfaScaler;
  int EventsProd[6];
  int EventsCons[6];

  uint32_t bafStatus;
  uint32_t sistripStatus;
  uint32_t hpgeStatus;
  uint32_t gcalStatus;
  uint32_t nrssStatus;

  double startTime;

  uint32_t DCOffset[MaxDT5743NChannels];
  int TriggerLevel[MaxDT5743NChannels];

} DaqSharedMemory;

int shmidDaq;
key_t keyDaq;
DaqSharedMemory* configDaqSharedMemory(char* caller); 
//void deleteDaqSharedMemory(); 
void deleteDaqSharedMemory(DaqSharedMemory*, int); 

typedef struct {
  uint32_t head; 
  uint32_t tail;   
  uint32_t maxSize;
  char* buffer; 
} circBuffer; 

int shmidBuffer;
circBuffer* configSharedBuffer(char* caller, key_t key); 
//void deleteSharedBuffer();
void deleteSharedBuffer(key_t, circBuffer*, int);
void writeCircularBuffer(circBuffer* sharedBuffer, char* bufferData, uint32_t bufferSize);
void writeTimeStamp(circBuffer* sharedBuffer);
uint32_t readCircularBuffer(circBuffer* sharedBuffer, char* readData, uint32_t bufferSize, uint32_t mytail);
uint32_t readEventSize(circBuffer* sharedBuffer, uint32_t mytail);

#endif // _SHAREDMEMORYSETUP__H
