/******************************************************************************************** 
* tupleMakerNRSS                                                                            *
*  standalone build:                                                                        *
*  g++ -std=gnu++11 -g `root-config --cflags --libs` tupleMakerNRSS.C -o ./tupleMakerNRSS   *
*                                                                                           *
*  usage: ./tupleMakerNRSS -f datafile [-r nRawEvents]                                     *
*                                                                                           *
*********************************************************************************************/

#include <sys/time.h>
#include <time.h>

#include "../include/DecodeDT5743.h"
#include "../CAENDigitizer.h"
#include "../include/NRSSTree.h"


#define headerSize 4
#define scstatusSize 4
#define timeSecSize 4
#define timenSecSize 4
#define runHeaderSize 4


#define MAX_CH 8
#define MAX_RL 1024

/* Board Id (registro 0xEF08) */
//#define DT5743Id   ??? //To be determined


int DecodeHeader(uint32_t header);

int main(int argc, char **argv) {
prova di modifica;
int ret;

FILE* file;
//stringhe di comodo per copiare il file 
char* fileName=NULL;    
char* buffer = NULL;
char* header = NULL;
char* scstatus= NULL;
char* timesec= NULL;     
char* timensec= NULL;
char* runnumber= NULL;
char* runconfig= NULL;

// run variables
uint32_t bufferSize=0, test, boardId;
//time variables 
int timeStampSize=timeSecSize+timenSecSize;
uint32_t evTime=0,startTime=0,nsecTime=0,nsecStartTime=0;
uint32_t status=0;

  char  raw;
  int nEvents=0;
  int myc=0;
  int maxEvt=0;
  int deviceConfig;
  int verbose=0, writeRaw=0;

CAEN_DGTZ_X743_EVENT_t *Evt743= NULL;

//root file and Trees;


//Open date file--- 
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
    printf("usage: tupleMakerNRSS -f fileName [-r nRawEvents] \n");
    exit(1);
  }
  file = fopen (fileName, "r");

header=(char*) malloc(headerSize);

// Read the run header...
runnumber=(char*) malloc(runHeaderSize);
if(!fread(runnumber,runHeaderSize,1,file)) {
    printf("*** Cannot find the run number! ");
    return(0);
	}
uint32_t runNumber=*(uint32_t *)(runnumber);
printf("tupleMakerNRSS: building events for run %i \n",runNumber);
runconfig=(char*) malloc(runHeaderSize);
if(!fread(runconfig,runHeaderSize,1,file)) { 
    printf("*** Cannot find the run configuration! ");
    return(0);
  }
uint32_t runConfig=*(uint32_t *)(runconfig);
deviceConfig=DecodeHeader(runConfig);  


//Create the root file
char rootFile[30],rn[10];
strcpy(rootFile,"Run");
sprintf(rn,"%i",runNumber);
strcat(rootFile,rn);
strcat(rootFile,"_eventsNRSS.root");
NRSSEvTree* myEvt= new NRSSEvTree(rootFile);

TFile *frawDT5743;
TTree *treeDT5743;
float dataNRSS[MAX_CH][MAX_RL];

if(writeRaw) {
    printf("Opening NRSS raw event tree \n");
    strcpy(rootFile,"Run");
    sprintf(rn,"%i",runNumber);
    strcat(rootFile,rn);
    strcat(rootFile,"_RawDataNRSS.root");
    frawDT5743 = new TFile(rootFile,"recreate");
    treeDT5743 = new TTree("rawdata","rawdata");
    treeDT5743->Branch("dataNRSS",dataNRSS,"dataNRSS[8][1024]/F ");
  }

  scstatus=(char*) malloc(scstatusSize);
  timesec=(char*) malloc(timeSecSize);
  timensec=(char*) malloc(timenSecSize);
  header=(char*) malloc(headerSize);


//Main loop over datafile

  while(!feof(file)) {        
    nEvents++;

    if((nEvents % 100)==0) printf("Decoding Event %i \n", nEvents);
    if(writeRaw>0 && nEvents>writeRaw) {
       nEvents--;
       printf("%i raw events written out to NRSSRawDataDT5743.root \n", nEvents);
       break;
    }
    if(maxEvt>0 && nEvents>maxEvt) {
       nEvents--;
       printf("%i events processed\n", nEvents);
       break;
    }

    //status
    if(!fread(scstatus,scstatusSize,1,file)) break;
    status= *(uint32_t *)(scstatus);
    printf("status: %d \n",status);

    //PC timestamp 
    if(!fread(timesec,timeSecSize,1,file)) break;
    evTime=*(uint32_t *)(timesec);
    if(nEvents==1) startTime=evTime;
    printf("   evTime: %u \n",evTime);
    printf("startTime: %u \n",startTime);
    time_t mysec=*(uint32_t *)(timesec);
    if((nEvents % 100)==0)  printf("DataTime: %s \n",ctime(&mysec));
    if(!fread(timensec,timenSecSize,1,file)) break;
    nsecTime=*(uint32_t*)(timensec);
    if(nEvents==1) nsecStartTime=nsecTime;
    printf("     nsecTime: %u \n",nsecTime);
    printf("nsecStartTime: %u \n",nsecStartTime);
    double timeStamp=(evTime-startTime)*1.e9 + nsecTime - nsecStartTime;
    timeStamp /= 1.e9;
    printf("timeStamp: %f \n",timeStamp);

    //buffer size
    if(!fread(header,headerSize,1,file)) break;
    bufferSize=*(long *) (header) & 0x0FFFFFFF;
    bufferSize=bufferSize*4;				
    printf("buffer size = %i\n",bufferSize);
    fseek(file, -4, SEEK_CUR);
    buffer=(char *)malloc(bufferSize);	//<--questo è il buffer evento!
	if(!fread(buffer,bufferSize,1,file)) {
    		printf("*** Cannot fill the event buffer! ");
    		return(0);
	}
    //uint32_t *Buffer = (uint32_t*)buffer;
    //Decode event

    myEvt->ResetNRSS();
    ret = CAEN_DGTZ_DecodeEventDT5743(buffer,(void **)&Evt743);


    printf("Decoded --> Storing\n");	
    //extract all info from Evt743
    //and put them inside the base tree (EvTree)

    myEvt->Store(Evt743, timeStamp, nEvents, runNumber);

    printf("Stored\n");	

    myEvt->FillTree();


    if(writeRaw){
	for (int i = 0; i < MAX_CH; ++i)
		{
		int gridx = i/2;
		for (int j = 0; j < MAX_RL; ++j)
			 {
			 float value;
			 if(i%2==0)	value = (Evt743->DataGroup[gridx]).DataChannel[0][j];
			 else value = (Evt743->DataGroup[gridx]).DataChannel[1][j];
	 		 dataNRSS[i][j] = value;
	 		} 
		}
 	treeDT5743->Fill();
	}  //end of if writeraw


  } //end of main loop

printf("NRSS read %i events \n",nEvents);

//distruttore
delete myEvt;

fclose(file);
free(Evt743);

if(writeRaw){
	frawDT5743->Write();
	frawDT5743->Close();
}

return 0;
}



int DecodeHeader(uint32_t header) {
  unsigned int myHeader=0;
  
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
  if(myHeader != 0) printf("deviceConfig: %u \n",myHeader);
     
  return myHeader;
}  

