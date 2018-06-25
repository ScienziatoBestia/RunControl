#include "SharedMemorySetup.h"

/************************************************************************************ */
DaqSharedMemory* configDaqSharedMemory(char* caller) {
/************************************************************************************ */

  printf("configDaqSharedMemory: Configuring the shared memory for %s \n",caller);

  DaqSharedMemory* myDaqSharedMemory = malloc(sizeof(*myDaqSharedMemory));

  int shmSize=sizeof(*myDaqSharedMemory);

  // Create the segment
  //keyDaq=7789;
  keyDaq=9515;

  if(strcmp(caller,"RunControl")==0) shmidDaq = shmget(keyDaq, shmSize, IPC_CREAT | 0666);
  else shmidDaq = shmget(keyDaq, shmSize, 0666);
  if(shmidDaq<0) {
    printf("*** ERROR: DAQ memory segment not created \n");
    return NULL;
  } 
  printf("DaqSharedMemory created (size %i) \n",shmSize);

  // Attach the segment to our data space
  myDaqSharedMemory = shmat(shmidDaq, NULL,0);
  if( myDaqSharedMemory == (void *)-1) {
    printf("*** ERROR: memory segment not attached to the data space \n");
    return NULL;
  }
  printf("DaqSharedMemory attached to %s address space: key %i/0x%X\n",caller,keyDaq,keyDaq);

  return myDaqSharedMemory;
}


/*************************************************************************************/
void deleteDaqSharedMemory(DaqSharedMemory* shMem, int isServer) {
/*************************************************************************************/
  if ( (shmidDaq = shmget(keyDaq, 0, 0)) >= 0 ) {
    printf("Deleting DaqSharedMemory key: %i/0x%X\n",keyDaq,keyDaq);
    shmdt((void *) shMem);
    if(isServer) {
      int ret = shmctl(shmidDaq, IPC_RMID, NULL);
      if(ret < 0) printf("***ERROR in deleting DaqSharedMemory \n");
      else printf("DaqSharedMemory deleted \n");
    }
  }

}



/*************************************************************************************/
circBuffer* configSharedBuffer(char* caller, key_t key) {
/*************************************************************************************/
  printf("configSharedBuffer: Configuring the shared buffer for %s with key %i\n",caller,key);

  circBuffer* SharedBuffer = malloc(sizeof(*SharedBuffer));

  uint32_t shmSize=CIRCBUFFER_SIZE;
  SharedBuffer->buffer=(char*)malloc(shmSize);

  // Create the segment
  if(strcmp(caller,"Producer")==0) shmidBuffer = shmget(key, shmSize, IPC_CREAT | 0666);
  else shmidBuffer = shmget(key, shmSize, 0666);
  if(shmidBuffer<0) {
    printf("*** ERROR: memory segment not created \n");
    return NULL;
  } 
  else {
    printf("circBuffer created (size %i) \n",shmSize);
  }

  // Attach the segment to our data space
  SharedBuffer = shmat(shmidBuffer, NULL,0);
  if(SharedBuffer == (void*)-1) {
    printf("*** ERROR: memory segment not attached to the data space \n");
    return NULL;
  }

  SharedBuffer->maxSize=shmSize;

  printf("circBuffer configured for %s: key %i/0x%X\n",caller,key,key);

  return SharedBuffer;
}


/*************************************************************************************/
void deleteSharedBuffer(key_t key, circBuffer* cB, int isServer) {
/*************************************************************************************/
  if((shmidBuffer = shmget(key, 0, 0)) >= 0 ) {
    printf("Deleting the  SharedBuffer key: %i / 0x%X\n",key,key);
    shmdt((void *) cB);
    if(isServer) {
      int ret = shmctl(shmidBuffer, IPC_RMID, NULL);
      if(ret < 0) printf("***ERROR in deleting the circBuffer \n");
      else printf("circBuffer deleted \n");
    }
  }

}


/*******************************************************************************************/
void writeCircularBuffer(circBuffer* sharedBuffer, char* bufferData, uint32_t bufferSize) {
/*******************************************************************************************/
  uint32_t size1, size2;

  int32_t dist=sharedBuffer->tail-sharedBuffer->head;
  if(dist <= 0) dist+=sharedBuffer->maxSize;
  printf("writeCircularBuffer: tail %u    head %u   bufferSize %u   dist %i \n",sharedBuffer->tail,sharedBuffer->head,bufferSize,dist);

  if(bufferSize > dist && sharedBuffer->tail!=-1 ) { //se tail=-1 il consumer non sta girando
    printf("***WARNING: No space left on sharedBuffer \n");
    sharedBuffer->head -= evtHeaderSize;
  } 
  else { 
    if(sharedBuffer->head+bufferSize <= sharedBuffer->maxSize) {
      // write in a single step
      printf("writeCircularBuffer: one step => bufferSize %u - head before writing %u \n",bufferSize,sharedBuffer->head);
      memcpy((void*) &(sharedBuffer->buffer)+sharedBuffer->head, (void*)bufferData, bufferSize);
      sharedBuffer->head += bufferSize;
      if(sharedBuffer->head == sharedBuffer->maxSize) sharedBuffer->head =0;
      printf("writeCircularBuffer: one step => head after writing %u \n",sharedBuffer->head);
      fflush(stdout);
    } 
    else {
      // write in two steps 
      printf("writeCircularBuffer: two steps => bufferSize %u - head before writing %u \n",bufferSize,sharedBuffer->head);
      size1=sharedBuffer->maxSize-sharedBuffer->head;
      memcpy((void*) &(sharedBuffer->buffer)+sharedBuffer->head, (void*)bufferData, size1);
      size2=bufferSize-size1;
      memcpy((void*) &(sharedBuffer->buffer), (void*)bufferData+size1, size2);
      sharedBuffer->head=size2;
      printf("writeCircularBuffer: two steps => head after writing %u \n",sharedBuffer->head);
      fflush(stdout);
    }
  } 
}



/*************************************************************************************/
void writeTimeStamp(circBuffer* sharedBuffer) {
/*************************************************************************************/
  int timeWordSize=4;
  struct timespec currTime;
  clock_gettime(CLOCK_REALTIME, &currTime);
  uint32_t mysec=currTime.tv_sec;
  uint32_t mynsec=currTime.tv_nsec;
  //printf("mysec %i sizeof %i\n", mysec,sizeof(mysec));
  //printf("mynsec %i sizeof %i\n", mynsec,sizeof(mynsec));
  writeCircularBuffer(sharedBuffer, (char*) &mysec, timeWordSize);
  writeCircularBuffer(sharedBuffer, (char*) &mynsec,timeWordSize);
}




/************************************************************************************************************/
uint32_t readCircularBuffer(circBuffer* sharedBuffer, char* readData, uint32_t bufferSize, uint32_t mytail) {
/************************************************************************************************************/
  uint32_t size1, size2;

  if(mytail+bufferSize <= sharedBuffer->maxSize) { // Read in a single step
    printf("readCircularBuffer: one step => bufferSize %u - mytail before reading %u \n",bufferSize,mytail);
    memcpy((void*) readData, (void*) &(sharedBuffer->buffer)+mytail,bufferSize);
    mytail += bufferSize;
    if(mytail == sharedBuffer->maxSize) mytail=0;
    printf("readCircularBuffer: one step => mytail after reading %u \n",mytail);
    fflush(stdout);
  } 
  else {  // Read in two steps 
    printf("readCircularBuffer: two steps => bufferSize %u - mytail %u \n",bufferSize,mytail);
    size1=sharedBuffer->maxSize-mytail;
    memcpy((void*) readData, (void*) &(sharedBuffer->buffer)+mytail, size1);
    size2=bufferSize-size1;
    memcpy((void*) readData +size1,(void*) &(sharedBuffer->buffer), size2);
    mytail=size2;
    printf("readCircularBuffer: two steps => mytail after reading %u \n",mytail);
    fflush(stdout);
  }

  return mytail;
}




/*************************************************************************************/
uint32_t readEventSize(circBuffer* sharedBuffer, uint32_t mytail) {
/*************************************************************************************/
  char* caenHeader = NULL;
  caenHeader = (char*) malloc(caenHeaderSize);
  mytail += evtHeaderSize;
  readCircularBuffer(sharedBuffer, caenHeader, caenHeaderSize, mytail);


  uint32_t bufferSize=*(long *) (caenHeader) & 0x0FFFFFFF;
  bufferSize=bufferSize*4;
  printf("readEventSize: bufferSize %u   sharedBuffer->tail %u   mytail %u  \n",bufferSize,sharedBuffer->tail,mytail);
  free(caenHeader);

  return bufferSize;
}
