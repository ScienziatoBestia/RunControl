#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/shm.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "SharedMemorySetup.h"
#include "Define.h"

int main(int argc, char **argv) {
  int verbose=0;

  char* myRead = NULL;
  FILE* file;
  char* recipeName;
  char fileName[200];

  DaqSharedMemory* cDaqSharedMemory;
  circBuffer* cSharedBuffer;
  int myc=0;
  int scalerAlfa;
  char chanMask;
  uint32_t boardId;
  int NAlfa=0;
  struct timespec runTimeStamp;
  char Device[10];
  uint32_t test, bufferSize=0;
  int bufcnt=0;
  int isGCAL=0, isCSPEC=0, isNRSS=0;
  char strnrun[100];
  char strDate[30];
  char logFilename[100];
  uint32_t RunConfig = 0x00000000;
  unsigned short debugging=1;
    
  while ((myc = getopt (argc, argv, "f:d:")) != -1)
  switch (myc) {
    case 'f':
      recipeName=(char *) malloc (strlen (optarg) + 1);
      strcpy ((char *) recipeName, optarg);
      break;
    case 'd':
      strcpy ((char *) Device, optarg);
      if(strstr(Device, "GCAL")!=NULL) isGCAL=1;
      else if(strstr(Device, "CSPEC")!=NULL) isCSPEC=1;
      else if(strstr(Device, "NRSS")!=NULL) isNRSS=1;
      break;
  }

  printf("%s Consumer: recipeName %s\n",Device,recipeName);

  printf("\n%s Consumer: Configuring  DAQSharedMemory \n",Device);
  cDaqSharedMemory=configDaqSharedMemory("Consumer");
  fflush(stdout);

  // Make sure that the Consumer allocates an already existing memory segment
  int wait=0, stopProducer;
  if(isCSPEC) stopProducer=cDaqSharedMemory->stopCSPECproducer;
  else if(isGCAL) stopProducer=cDaqSharedMemory->stopGCALproducer;
  else if(isNRSS) stopProducer=cDaqSharedMemory->stopNRSSproducer;
  while(stopProducer != 0) {
    if(wait>5) exit(1);
    sleep(1);
    printf("%s Consumer: Waiting the start of Producer %is - stopProducer%i \n", Device,wait+1,stopProducer);
    wait++;
  }
  printf("%s Consumer: Configuring the SharedBuffer \n",Device);
  if(isCSPEC) {
    cSharedBuffer=configSharedBuffer("Consumer",keyCSPEC);
    scalerAlfa=cDaqSharedMemory->AlfaScaler;
    cDaqSharedMemory->stopCSPECconsumer=0;
  } 
  else if(isGCAL) {
    cSharedBuffer=configSharedBuffer("Consumer",keyGCAL);
    cDaqSharedMemory->stopGCALconsumer=0;
  }
  else if(isNRSS) {
    cSharedBuffer=configSharedBuffer("Consumer",keyNRSS);
    cDaqSharedMemory->stopNRSSconsumer=0;
  }
  fflush(stdout);
 

  while(!cDaqSharedMemory->stopDAQ) {
    if(cSharedBuffer->head==0) break;
  }
  cSharedBuffer->tail=0;


  while(1) {
    printf("\n%s Consumer: cDaqSharedMemory->stopDAQ %i \n",Device,cDaqSharedMemory->stopDAQ);
    if(cDaqSharedMemory->stopDAQ) break;

    printf("\n%s Consumer: Resetting event counters \n",Device);
    if(isCSPEC) {
      cDaqSharedMemory->EventsCons[BaFGamma]=0;
      cDaqSharedMemory->EventsCons[BaFAlfa]=0;
      cDaqSharedMemory->EventsCons[HPGe]=0;
      cDaqSharedMemory->EventsCons[SiStrip]=0;

      if(cDaqSharedMemory->IncludeBaF)     RunConfig|= (0x1 << 0);
      if(cDaqSharedMemory->IncludeHPGe)    RunConfig|= (0x1 << 1);
      if(cDaqSharedMemory->testHPGe)       RunConfig|= (0x1 << 2);
      if(cDaqSharedMemory->IncludeSiStrip) RunConfig|= (0x1 << 3);
      RunConfig|= (cDaqSharedMemory->AlfaScaler << 18);
    } 
    else if(isGCAL){
      RunConfig |= (0x1 << 4);
      cDaqSharedMemory->EventsCons[GCAL]=0;
    }
    else if(isNRSS){
      RunConfig |= (0x1 << 5);
      cDaqSharedMemory->EventsCons[NRSS]=0;
    }

    if(cDaqSharedMemory->softwareTrigger) {
      RunConfig|= (0x1 << 6);
      RunConfig|= (cDaqSharedMemory->SoftwareTrgRate << 7);
    }
    printf("%s Consumer: RunConfig %u \n",Device,RunConfig);
    
    int nRun=cDaqSharedMemory->runNumber;
    sprintf(strnrun,"%i",nRun);
    if(debugging) sprintf(logFilename,"Log/Consumer_Run%i",nRun);
    else sprintf(logFilename,"Log/LogFile_Run%i",nRun);
    freopen (logFilename,"a",stdout);
    freopen (logFilename,"a",stderr);  
    printf("\n%s Consumer: Opening %s \n",Device,logFilename);
    fflush(stdout);



    strcpy(fileName,"");
  
    strcat(fileName,"Data/Run");
    strcat(fileName,strnrun);
    strcat(fileName,"_");
    strcat(fileName,Device);
    strcat(fileName,"_");
    strcat(fileName,recipeName);
    strcat(fileName,"_");

    clock_gettime(CLOCK_REALTIME, &runTimeStamp);
    struct tm* mytime=localtime(&runTimeStamp.tv_sec);
    sprintf(strDate,"%02i%02i%i_%02i%02i%02i",mytime->tm_mday,mytime->tm_mon+1,mytime->tm_year+1900,mytime->tm_hour, mytime->tm_min, mytime->tm_sec);

    printf(" >>>> filename : %s\n",fileName);

    strcat(fileName,strDate);
    printf("%s Consumer: Opening %s \n",Device,fileName);
    file = fopen (fileName, "w+");
    if(file==NULL) printf("*** Consumer: ERROR IN OPENING FILE %s \n",fileName);
    fflush(stdout);

    // Write the run header: run number + run configuration word
    myRead = (char*)malloc(runHeaderSize);  
    myRead = (char*) &nRun;
    fwrite (myRead, runHeaderSize, 1, file);

    myRead = (char*)malloc(runHeaderSize);  
    myRead = (char*) &RunConfig;
    fwrite (myRead, runHeaderSize, 1, file);

    while(cDaqSharedMemory->runNumber==nRun) { // Loop on run number
      if(debugging) printf("\n%s Consumer: stopDAQ %i     runNumber %i      head %i      tail %i\n",
              Device,cDaqSharedMemory->stopDAQ,cDaqSharedMemory->runNumber,cSharedBuffer->head,cSharedBuffer->tail);

      if((cSharedBuffer->head - cSharedBuffer->tail)<=evtHeaderSize) {
        if(cDaqSharedMemory->stopDAQ) break;
        if(debugging) printf("%s Consumer: Waiting for new data - head %u - tail %u \n",Device,cSharedBuffer->head,cSharedBuffer->tail);
        usleep(1000);
        continue; 
      }

      bufferSize=readEventSize(cSharedBuffer, cSharedBuffer->tail);
      if(debugging) printf("%s Consumer: bufferSize %u \n",Device,bufferSize);
      myRead=(char*)malloc(bufferSize+evtHeaderSize);  
      cSharedBuffer->tail = readCircularBuffer(cSharedBuffer,myRead,bufferSize+evtHeaderSize,cSharedBuffer->tail);
      if(debugging) printf("%s Consumer: head %u - tail %u \n",Device,cSharedBuffer->head,cSharedBuffer->tail);
      fwrite(myRead, bufferSize+evtHeaderSize, 1, file);

      if(isCSPEC) {
	chanMask = *(long *)(myRead+evtHeaderSize+caenHeaderSize) & 0x0000000F;
	 boardId = *(long *)(myRead+evtHeaderSize+caenHeaderSize) & 0xF8000000;
	if(boardId==DT5780Id) {
          printf("%s Consumer: DT5780Id \n",Device);
	  cDaqSharedMemory->EventsCons[HPGe]++;
	}
	else if(boardId == V1742BaFId) { //is BaF
          printf("%s Consumer: V1742BaFId \n",Device);
	  if(!(chanMask & 0x1)) {  // is alfa ONLY
	    NAlfa++;
	    if((NAlfa%scalerAlfa)==0) cDaqSharedMemory->EventsCons[BaFAlfa]++;
            printf("%s Consumer: BaF alfa only \n",Device);
	  } 
          else {  
	    cDaqSharedMemory->EventsCons[BaFGamma]++;
            printf("%s Consumer: BaF gamma \n",Device);
	    if((chanMask>>2) & 0x1) { cDaqSharedMemory->EventsCons[BaFAlfa]++; printf("%s Consumer: BaF alfa \n",Device); }
	  }
	} 
        else {  // is SiStrip
	  cDaqSharedMemory->EventsCons[SiStrip]++;
          printf("%s Consumer: SiStrip \n",Device);
	}
      } 
      else if (isGCAL){  // is GCAL
	cDaqSharedMemory->EventsCons[GCAL]++;
        printf("%s Consumer: GCAL \n",Device);
      }
       else if (isNRSS){  // is NRSS
        cDaqSharedMemory->EventsCons[NRSS]++;
        printf("%s Consumer: NRSS \n",Device);
      }

      if(debugging) printf("%s Consumer: Event BaFgamma %u    BaFalfa %u    HPGe %u    SiStrip %u   GCAL %u   NRSS %u\n\n",
        Device,cDaqSharedMemory->EventsCons[BaFGamma], cDaqSharedMemory->EventsCons[BaFAlfa],
        cDaqSharedMemory->EventsCons[HPGe], cDaqSharedMemory->EventsCons[SiStrip],
        cDaqSharedMemory->EventsCons[GCAL], cDaqSharedMemory->EventsCons[NRSS]);
      fflush(stdout);

      free(myRead);
     
    } // end loop on runNumber
 
    fclose(file);    
 
   if(isCSPEC) {
      printf("\n\n%s Consumer: Written %i events to file %s \n",Device,
      cDaqSharedMemory->EventsCons[BaFGamma]+cDaqSharedMemory->EventsCons[BaFAlfa]+cDaqSharedMemory->EventsCons[HPGe]+cDaqSharedMemory->EventsCons[SiStrip],fileName);
      printf("%s Consumer: written %i BaF Gamma events \n",Device,cDaqSharedMemory->EventsCons[BaFGamma]);
      printf("%s Consumer: written %i BaF Alfa events \n",Device,cDaqSharedMemory->EventsCons[BaFAlfa]);
      printf("%s Consumer: written %i HPGe events \n",Device,cDaqSharedMemory->EventsCons[HPGe]);
      printf("%s Consumer: written %i SiStrip events \n",Device,cDaqSharedMemory->EventsCons[SiStrip]);
   }
   if(isGCAL) printf("\n\n%s Consumer: Written %i events to file %s \n",Device,cDaqSharedMemory->EventsCons[GCAL],fileName);
   if(isNRSS) printf("\n\n%s Consumer: Written %i events to file %s \n",Device,cDaqSharedMemory->EventsCons[NRSS],fileName);
   fflush(stdout);

  }// end while DAQ active


  printf("%s Detaching the Consumer shared buffer \n",Device);
  if(isCSPEC){ 
    deleteSharedBuffer(keyCSPEC,cSharedBuffer,0);
    cDaqSharedMemory->stopCSPECconsumer=1;
  }
  else if(isGCAL) {
    deleteSharedBuffer(keyGCAL,cSharedBuffer,0);
    cDaqSharedMemory->stopGCALconsumer=1;
  }
  else if(isNRSS){
    deleteSharedBuffer(keyNRSS,cSharedBuffer,0);
    cDaqSharedMemory->stopNRSSconsumer=1;
  }

  printf("%s Detaching the Consumer shared memory \n",Device);
  deleteDaqSharedMemory(cDaqSharedMemory,0);

  fclose(stdout); 

  exit(0);
}
