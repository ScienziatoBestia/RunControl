/**********************************************************************************************
* per compilare in modo standalone: 
* scl enable devtoolset-2 tcsh
* g++ -std=gnu++11 -g `root-config --cflags --libs` src/tupleMakerGCAL.C -o ./tupleMakerGCAL
*
*  usage: ./tupleMakerGCAL -f datafile [-r nRawEvents] 
* 
***********************************************************************************************/
#include <sys/time.h>
#include <time.h>

#include "../include/x742.h"
#include "../include/X742CorrectionRoutines.h"
#include "../include/CAENDigitizer.h"
#include "../include/GCALTree.h"

#define headerSize 4 
#define scstatusSize 4
#define timeSecSize 4
#define timenSecSize 4
#define runHeaderSize 4

/* Board Id (registro 0xEF08) */
#define V1742CaloId  4026531840 //0x1E

int DecodeHeader(uint32_t);

int main(int argc, char **argv) {
  int ret;
  char* buffer = NULL;
  char* header = NULL;
  char* timesec= NULL;
  char* timensec= NULL;
  char* runnumber= NULL;
  char* runconfig= NULL;
  char* scstatus= NULL;
  FILE* file;
  int nEvents=0;
  int verbose=0;
  CAEN_DGTZ_X742_EVENT_t *Evt742= NULL;
  CAEN_DGTZ_DRS4Correction_t X742Tables[MAX_X742_GROUP_SIZE];
  CAEN_DGTZ_DRS4Frequency_t freq;
  int writeRaw=0, maxEvt=0;
  uint32_t bufferSize=0, test, boardId;
  int i, j, k, o, l;
  int corrTableLoaded = 0;
  char stringa[1000];
  double dataV1742[36][1024];
  char* fileName=NULL;
  char raw;
  int myc=0;
  int deviceConfig;
  int timeStampSize=timeSecSize+timenSecSize;
  
  uint32_t evTime=0,startTime=0,nsecTime=0,nsecStartTime=0;
  uint32_t GCALstatus=0;
  Double_t timeStamp;

  while ((myc = getopt (argc, argv, "f:r:n:")) != -1) {
    switch (myc)
      {
      case 'f':
        fileName=(char *) malloc (strlen (optarg) + 1);
        strcpy ((char *) fileName, optarg);
        break;
      case 'r':
        strcpy(&raw,optarg);
        writeRaw=atoi(&raw);
        printf("\n%i raw events will be written out \n", writeRaw);
        break;
      case 'n':
        strcpy(&raw,optarg);
        maxEvt=atoi(&raw);
        printf("\nProcessing %i events \n", maxEvt);
        break;
      }
  }

  if(fileName==NULL) {
    printf("usage: tupleMakerGCAL -f fileName [-r 1] \n");
    return(0);
  }
  file = fopen (fileName, "r");

  // Read the run header -> to be used with new data ***NOT FOR LABEC TEST DATA!!
  runnumber=(char*) malloc(runHeaderSize);
  if(!fread(runnumber,runHeaderSize,1,file)) {
    printf("*** Cannot find the run number! ");
    return(0);
  }
  uint32_t runNumber=*(uint32_t *)(runnumber);
  printf("tupleMakerGCAL: building events for run %i \n",runNumber);

  char rootFile[30],rn[10];
  strcpy(rootFile,"Run");
  sprintf(rn,"%i",runNumber);
  strcat(rootFile,rn);
  strcat(rootFile,"_eventsGCAL.root");
  GCALTree* myGCALclass = new GCALTree(rootFile);

  TFile *frawV1742;
  TTree *treeV1742;
  if(writeRaw) {
    printf("Opening GCAL raw event tree \n");
    strcpy(rootFile,"Run");
    sprintf(rn,"%i",runNumber);
    strcat(rootFile,rn);
    strcat(rootFile,"_RawDataGCAL.root");
    frawV1742=new TFile(rootFile,"recreate");
    treeV1742=new TTree("rawdata","rawdata");
    treeV1742->Branch("dataV1742",dataV1742,"dataV1742[36][1024]/D ");
  }

  runconfig=(char*) malloc(runHeaderSize);
  if(!fread(runconfig,runHeaderSize,1,file)) {
    printf("*** Cannot find the run configuration! ");
    return(0);
  }
  uint32_t runConfig=*(uint32_t *)(runconfig);
  deviceConfig=DecodeHeader(runConfig);

  scstatus=(char*) malloc(scstatusSize);
  timesec=(char*) malloc(timeSecSize);
  timensec=(char*) malloc(timenSecSize);    
  header=(char*) malloc(headerSize);

 //------------------------------------------------------------
  while(!feof(file)) {                 //Main loop on datafile
 //------------------------------------------------------------
    nEvents++;
    if((nEvents % 100)==0) printf("Decoding Event %i \n", nEvents);
    if(writeRaw>0 && nEvents>writeRaw) {
       nEvents--;
       printf("%i raw events written out to GCALRawDataV1742.root \n", nEvents);
       break;
    }
    if(maxEvt>0 && nEvents>maxEvt) {
       nEvents--;
       printf("%i events processed\n", nEvents);
       break;
    }

    if(!fread(scstatus,scstatusSize,1,file)) break;
    GCALstatus= *(uint32_t *)(scstatus);
    printf("Event: %d  GCALstatus: %d \n",nEvents,GCALstatus);

    if(!fread(timesec,timeSecSize,1,file)) break;
    evTime=*(uint32_t *)(timesec);
    if(nEvents==1) startTime=evTime;
    time_t mysec=*(uint32_t *)(timesec);
    if((nEvents % 100)==0)  printf("DataTime: %s \n",ctime(&mysec));

    if(!fread(timensec,timenSecSize,1,file)) break;
    nsecTime=*(uint32_t*)(timensec);
    if(nEvents==1) nsecStartTime=nsecTime;
    timeStamp=(evTime-startTime)*1.e9 + nsecTime - nsecStartTime;
    timeStamp /= 1.e9;


    if(!fread(header,headerSize,1,file)) break;
    bufferSize=*(long *) (header) & 0x0FFFFFFF;
    bufferSize=bufferSize*4;
    buffer=(char *)malloc(bufferSize);	

    fseek(file, -4, SEEK_CUR);
    if(!fread(buffer, bufferSize, 1, file)) break;
    test= *(long *) (buffer+8) & 0x3FFFFF;
    if(verbose) printf("Event counter %u \n",test);

    boardId= *(long *) (buffer+4) & 0xF8000000;
    if(verbose) printf("board id %u \n",boardId);

    //------------------------------------------------------
    if(boardId == V1742CaloId) {    // V1742 GCAL digitizer
    //------------------------------------------------------
      if(!(runConfig & (0x1 << 4))) {
       printf("*** ERROR: Reading Calo data block but GCAL is not present in the run configuration\n");
       exit(1);
      }

      myGCALclass->ResetGCAL();

      X742_DecodeEvent (buffer, (void **)&Evt742);

      for(o=0; o<MAX_X742_GROUP_SIZE; o++) {
        for(j=0; j<MAX_X742_CHANNEL_SIZE; j++) { 
          for(k = 0; k < 1024 ; k++) {	 		    
            dataV1742[j+9*o][k]=-100;
          }
        }

	if(Evt742->GrPresent[o] == 1) {					
	  switch(Evt742->DataGroup[o].Frequency)
	  {
	   case 0:
	     freq = CAEN_DGTZ_DRS4_5GHz;
	     break;
	   case 1:
             freq = CAEN_DGTZ_DRS4_2_5GHz;
             break;
           case 2:
             freq = CAEN_DGTZ_DRS4_1GHz;
             break;
           }

           // al primo evento carico le tabelle di calibrazione 
	   if(!corrTableLoaded) {
	     for(i=0; i<MAX_X742_GROUP_SIZE; i++) {
               bzero(stringa, 1000);
               sprintf(stringa, "CorrectionTables/Calo/Corrections_freq%d_gr%d",Evt742->DataGroup[o].Frequency, i);
               LoadCorrectionTable(stringa, &(X742Tables[i]));
             }
	     corrTableLoaded = 1;
	   }
	      
	   ApplyDataCorrection(&(X742Tables[o]), freq, 7, &(Evt742->DataGroup[o]));

	   for(j=0; j<MAX_X742_CHANNEL_SIZE; j++) {        
             for(k = 0; k < Evt742->DataGroup[o].ChSize[j] ; k++) {
               dataV1742[j+9*o][k]=Evt742->DataGroup[o].DataChannel[j][k];
             }
             free(Evt742->DataGroup[o].DataChannel[j]); // Essendo DataChannel un puntatore, 
                                                        // va deallocato manualmente, cioe' non 
                                                        // basta solo il free(Evt742) alla fine
           }
        } 
      }
 
      myGCALclass->StoreGCAL(dataV1742,timeStamp,GCALstatus);
      myGCALclass->FillTree();

      if(writeRaw) treeV1742->Fill();

      //---------------------------
    } // end v1742 Digitizer
      //---------------------------
 

    //---------------------------
  } // end while loop on data
    //---------------------------
    
  printf("GCAL read %i events \n ", nEvents);

  fclose(file);
  free(Evt742);

  if(writeRaw) {
    frawV1742->Write();
    frawV1742->Close();
  }

  delete myGCALclass;
}


  
int DecodeHeader(uint32_t header) {
  unsigned int myHeader=0;

  printf("\n************************** \n");
  printf(" Run configuration \n");
  printf("************************** \n");
  
  if(header & (0x1 << 0)) {
     myHeader |= 1;
     printf("BaF selected \n");
     printf("AlfaScaler %i \n", header>>18);
  }
  if(header & (0x1 << 1)) {
     myHeader |= (0x1 << 1); 
     printf("HPGe selected \n");
  }
  if(header & (0x1 << 2)) {
    printf("TestHPGe selected \n");
  }
  if(header & (0x1 << 3)) {
     myHeader |= (0x1 << 2);
    printf("SiStrip selected \n");
  }       

  if(header & (0x1 << 4)) printf("GCAL selected \n");
  if(header & (0x1 << 5)) printf("NRSS selected \n");
  if(header & (0x1 << 6)) printf("Software Trigger is on \n");
  if(header & (0x1 << 6) || header & (0x1 << 2))  printf("Trigger Rate %i Hz\n", (header>>7) & 0x000007FF);
  printf("deviceConfig: %u \n",myHeader);
  
  return myHeader;
}
