/**********************************************************************************************
* per compilare in modo standalone: 
* scl enable devtoolset-2 tcsh
* g++ -std=gnu++11 -g `root-config --cflags --libs` src/tupleMakerCSPEC.C -o ./tupleMakerCSPEC
*
*  usage: ./tupleMakerCSPEC -f datafile [-n nevents] [-r nRawEvents]
* 
***********************************************************************************************/
#include <sys/time.h>
#include <time.h>

#include "../include/x742.h"
#include "../include/X742CorrectionRoutines.h"
#include "../include/DecodeDT5780.h"
#include "../CAENDigitizer.h"
#include "../include/CSPECTree.h"

#define headerSize 4
#define scstatusSize 4
#define timeSecSize 4
#define timenSecSize 4
#define runHeaderSize 4

/* Board Id (registro 0xEF08) */
#define DT5780Id   3758096384 //0x1C
#define V1742BaFId 4160749568 //0x1F

CAEN_DGTZ_DPP_PHA_Event_t  *Events[2] ={NULL,NULL};// events buffer
CAEN_DGTZ_DPP_PHA_Waveforms_t *Waveform=NULL;      // waveforms buffer
Double_t HPGeEnergy[2]={0};
Double_t myEnergy, CommonMode, SumSamples, xMax, TriggerTime;
Double_t timeStamp;

int DecodeHeader(uint32_t);
int HPGeInit();


int main(int argc, char **argv) {
  FILE* file;
  char* fileName=NULL;
  char* buffer = NULL;
  char* header = NULL;
  char* scstatus= NULL;
  char* timesec= NULL;
  char* timensec= NULL;
  char* runnumber= NULL;
  char* runconfig= NULL;
  char  raw;
  int nEvents=0;
  int myc=0;
  int writeRaw=0, maxEvt=0;

  char chanMask;
  uint32_t bufferSize=0, test, boardId;
  int timeStampSize=timeSecSize+timenSecSize;
  int deviceConfig;
  uint32_t evTime=0,startTime=0,nsecTime=0,nsecStartTime=0;
  uint32_t status=0;

  CAEN_DGTZ_X742_EVENT_t *Evt742= NULL;
  CAEN_DGTZ_DRS4Correction_t X742Tables[MAX_X742_GROUP_SIZE];
  CAEN_DGTZ_DRS4Frequency_t freq;
  double dataSignalV1742[18][1024],dataAlfaV1742[18][1024];
  char stringa[1000];
  int corrTableLoaded = 0;
  int isSignal=0, isAlfa=0;
  int nEventsV1742Signal=0,nEventsV1742Alfa=0;
  bool bafReady=false;
  TFile *frawV1742;
  TTree *treeV1742;

  Int_t dataDT5780[8][10000];
  int ret;
  uint32_t NumEvents[2]={1,1};
  int size;
  int16_t *WaveLine1;
  int16_t *WaveLine2;
  uint8_t *DigitalWaveLine1;
  uint8_t *DigitalWaveLine2;
  int nEventsDT5780Ch0=0,nEventsDT5780Ch1=0;
  bool hpgeReady=false;
  TFile *frawDT5780;
  TTree *treeDT5780;

  uint16_t dataV1495[2][1024];
  uint16_t* PamSibuff16;
  PamSibuff16 = (uint16_t *)malloc(4096);
  int nEventsV1495=0;
  bool pamsiReady=false;
  TFile *frawV1495;
  TTree *treeV1495;


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
    printf("usage: tupleMakerCSPEC -f fileName [-n nevents] [-r nRawEvents] \n");
    exit(1);
  }
  file = fopen (fileName, "r");

  header=(char*) malloc(headerSize);
  timesec=(char*) malloc(timeSecSize);
  timensec=(char*) malloc(timenSecSize);
  scstatus=(char*) malloc(scstatusSize);

  // Read the run header
  runnumber=(char*) malloc(runHeaderSize);
  if(!fread(runnumber,runHeaderSize,1,file)) {
    printf("\n*** Cannot find the run number! ");
    return(0);
  }
  uint32_t runNumber=*(uint32_t *)(runnumber);
  printf("Building events for run %i \n",runNumber);
  
  char rootFile[30],rn[10];
  strcpy(rootFile,"Run");
  sprintf(rn,"%i",runNumber);
  strcat(rootFile,rn);
  strcat(rootFile,"_eventsCSPEC.root");
  CSPECTree* myEvt= new CSPECTree(rootFile);
  
  // Read the run configuration
  runconfig=(char*) malloc(runHeaderSize);
  if(!fread(runconfig,runHeaderSize,1,file)) { 
    printf("\n*** Cannot find the run configuration! ");
    return(0);
  }
  uint32_t runConfig=*(uint32_t *)(runconfig);
  deviceConfig=DecodeHeader(runConfig);   

  CSPECTree* myAlpha;
  if(runConfig & (0x1 << 0)) {
    myEvt->switchOnBranches("BaF");

    strcpy(rootFile,"Run");
    strcat(rootFile,rn);
    strcat(rootFile,"_alfaCSPEC.root");
    myAlpha = new CSPECTree(rootFile);
    myAlpha->switchOnBranches("BaF");

    if(writeRaw) {
      printf("Opening BaF raw event tree \n");
      strcpy(rootFile,"Run");
      strcat(rootFile,rn);
      strcat(rootFile,"_RawDataBaF.root");
      frawV1742=new TFile(rootFile,"recreate");
      treeV1742=new TTree("rawdata","rawdata");
      treeV1742->Branch("dataV1742",dataSignalV1742,"dataV1742[18][1024]/D ");
    }
  }
  if(runConfig & (0x1 << 1)) {
    myEvt->switchOnBranches("HPGe");
    HPGeInit();
    if(writeRaw) {
      printf("Opening HPGe raw event tree \n");
      strcpy(rootFile,"Run");
      strcat(rootFile,rn);
      strcat(rootFile,"_RawDataHPGe.root");
      frawDT5780=new TFile(rootFile,"recreate");
      treeDT5780=new TTree("rawdata","rawdata");
      treeDT5780->Branch("dataDT5780",dataDT5780,"dataDT5780[8][10000]/I ");
    } 
  }
  if(runConfig & (0x1 << 3)) {
    myEvt->switchOnBranches("SiStrip");
    myEvt->InitSiStrip();
    if(writeRaw) {
      printf("Opening SiStrip raw event tree \n");
      strcpy(rootFile,"Run");
      strcat(rootFile,rn);
      strcat(rootFile,"_RawDataSiStrip.root");
      frawV1495=new TFile(rootFile,"recreate");
      treeV1495=new TTree("rawdata","rawdata");
      treeV1495->Branch("dataV1495",dataV1495,"dataV1495[2][1024]/I ");
    }
  }
    
 //------------------------------------------------------------
  while(!feof(file)) {                 //Main loop on datafile
 //------------------------------------------------------------
    nEvents++;
    if((nEvents % 100)==0) printf("Event %i \n", nEvents);
    if(writeRaw>0 && nEvents>writeRaw) {
       nEvents--;
       printf("%i raw events written out \n", nEvents);
       break;
    }
    if(maxEvt>0 && nEvents>maxEvt) {
       nEvents--;
       printf("%i events processed\n", nEvents);
       break;
    }

    if(!fread(scstatus,scstatusSize,1,file)) break;
    status= *(uint32_t *)(scstatus);
    //printf("status: %d \n",status);

    if(!fread(timesec,timeSecSize,1,file)) break;
    evTime=*(uint32_t *)(timesec);
    if(nEvents==1) startTime=evTime;
    //printf("   evTime: %u \n",evTime);
    //printf("startTime: %u \n",startTime);
    time_t mysec=*(uint32_t *)(timesec);
    //if((nEvents % 100)==0)  printf("DataTime: %s \n",ctime(&mysec));

    if(!fread(timensec,timenSecSize,1,file)) break;
    nsecTime=*(uint32_t*)(timensec);
    if(nEvents==1) nsecStartTime=nsecTime;
    //printf("     nsecTime: %u \n",nsecTime);
    //printf("nsecStartTime: %u \n",nsecStartTime);
    timeStamp=(evTime-startTime)*1.e9 + nsecTime - nsecStartTime;
    timeStamp /= 1.e9;
    //printf("timeStamp: %f \n",timeStamp);


    if(!fread(header,headerSize,1,file)) break;
    bufferSize=*(long *) (header) & 0x0FFFFFFF;
    bufferSize=bufferSize*4;
    buffer=(char *)malloc(bufferSize);	

    fseek(file, -4, SEEK_CUR);
    if(!fread(buffer, bufferSize, 1, file)) break;
    test= *(long *) (buffer+8) & 0x3FFFFF;
    //printf("Event counter %u \n",test);

    boardId= *(long *) (buffer+4) & 0xF8000000;
    //printf("board id %u \n",boardId);

    chanMask = *(long *)(buffer+4) & 0x0000000F;

    //--------------------------------------------------
    if(boardId == V1742BaFId) {       // V1742 digitizer
    //--------------------------------------------------
      if(!(runConfig & (0x1<<0))) {
       printf("*** ERROR: Reading V1742 data block but BaF is not present in the run configuration\n");
       exit(1);
      }
      if(bafReady) {
       printf("*** ERROR: bafReady=true when reading BaF data block\n");
       printf("*** ERROR: A data block is missing, skipping event %i \n",nEvents-1);
       bafReady=false;
       //exit(1);
      }

      myEvt->ResetBaF();
      isSignal=0;
      isAlfa=0;

      X742_DecodeEvent (buffer, (void **)&Evt742);

      for(int o=0; o<MAX_X742_GROUP_SIZE; o++) {
	if(o<2) {
	  for(int j=0; j<MAX_X742_CHANNEL_SIZE; j++) { 
            for(int k = 0; k < 1024 ; k++) {	 		    
              dataSignalV1742[j+9*o][k]=-100;
              dataAlfaV1742[j+9*o][k]=-100;
            }
          }
        }
	if(Evt742->GrPresent[o] == 1) {
	  if(o<2) isSignal=1;
	  else isAlfa=1;
					
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
             printf("\n");
	     for(int i=0; i<MAX_X742_GROUP_SIZE; i++) {
               bzero(stringa, 1000);
               sprintf(stringa, "CorrectionTables/BaF/Corrections_freq%d_gr%d",Evt742->DataGroup[o].Frequency, i);
               LoadCorrectionTable(stringa, &(X742Tables[i]));
             }
	     corrTableLoaded = 1;
             printf("\n");
	   }
	      
	   ApplyDataCorrection(&(X742Tables[o]), freq, 7, &(Evt742->DataGroup[o]));
	   for(int j=0; j<MAX_X742_CHANNEL_SIZE; j++) {        
             for(int k = 0; k < Evt742->DataGroup[o].ChSize[j] ; k++) {
               if(o<2) dataSignalV1742[j+9*o][k]=Evt742->DataGroup[o].DataChannel[j][k];
               else dataAlfaV1742[j+9*(o-2)][k]=Evt742->DataGroup[o].DataChannel[j][k];
             }

             //Essendo DataChannel un puntatore, va deallocato manualmente, cioe' non basta solo il free(Evt742) alla fine
             free(Evt742->DataGroup[o].DataChannel[j]);
           } //for (j=0; j<MAX_X742_CHANNEL_SIZE; j++)
        } 
      } //for (o=0; o<MAX_X742_GROUP_SIZE; o++)
 
      if(isAlfa) {
	nEventsV1742Alfa++;
        myAlpha->ResetBaF();
        myAlpha->StoreBaF(dataAlfaV1742,timeStamp,status);
        myAlpha->FillTree();
      }

      if(isSignal) {
        nEventsV1742Signal++;
        myEvt->StoreBaF(dataSignalV1742,timeStamp,status);
        bafReady=true;
        if(deviceConfig==1) { myEvt->FillTree(); bafReady=false; }
        if(writeRaw) treeV1742->Fill();
      }

      //---------------------------
    } // end v1742 Digitizer
      //---------------------------

    //------------------------------------------------------
    else if(boardId == DT5780Id) {  // HPGe - DT5780 module
    //------------------------------------------------------
      if(!(runConfig & (0x1 << 1))) {
       printf("*** ERROR: Reading DT5780 data block but HPGe is not present in the run Configuration\n");
       exit(1);
      }
      if(hpgeReady) {
       printf("*** ERROR: hpgeReady=true when reading HPGe data block\n");
       printf("*** ERROR: A data block is missing, skipping event %i \n",nEvents-1);
       hpgeReady=false;
       //exit(1);
      }

      myEvt->ResetHPGe();

      int nHPGeChan=0; 
      if ((chanMask >> 0) & 0x1) {
	nEventsDT5780Ch0++;
	//printf("nEventsDT5780 Ch0 %i \n",nEventsDT5780Ch0);
        nHPGeChan=1; 
      } 
      if((chanMask >> 1) & 0x1) {
	nEventsDT5780Ch1++;
	printf("nEventsDT5780 Ch1 %i \n",nEventsDT5780Ch1);
        nHPGeChan=2; 
      }
      
      ret = DT5780_DPP_PHA_GetDPPEvents(buffer, bufferSize,  Events, NumEvents);
      
      for(int ch=0; ch<nHPGeChan; ch++) {
      //printf("ch %i numevents %i energy %i \n",ch,NumEvents[ch],Events[ch][0].Energy);

        for(int ev=0; ev<NumEvents[ch]; ev++) {

      //printf("ch %i   ev %i    energy %u \n",ch,ev,Events[ch][ev].Energy);
      //printf("ch %i   ev %i    Format %u \n",ch,ev,Events[ch][ev].Format);
      //printf("ch %i   ev %i    TimeTag %u \n",ch,ev,Events[ch][ev].TimeTag);
      //printf("ch %i   ev %i    Extras %u \n",ch,ev,Events[ch][ev].Extras);
      //printf("ch %i   ev %i    Extras2 %u \n",ch,ev,Events[ch][ev].Extras2);

          if(Events[ch][ev].Energy >= 0) {
            HPGeEnergy[ch]=Events[ch][ev].Energy;
	    DT5780_DPP_PHA_DecodeDPPWaveforms(&Events[ch][ev], Waveform);

            // Use waveform data here...
	    size = (int)(Waveform->Ns); // Number of samples
	    WaveLine1 = Waveform->Trace1; // First trace (VIRTUALPROBE1 set with CAEN_DGTZ_SetDPP_PSD_VirtualProbe)
	    WaveLine2 = Waveform->Trace2; // Second Trace (if single trace mode, it is a sequence of zeroes)
	    DigitalWaveLine1 = Waveform->DTrace1; // First Digital Trace (DIGITALPROBE1 set with CAEN_DGTZ_SetDPP_PSD_VirtualProbe)
	    DigitalWaveLine2 = Waveform->DTrace2; // Second Digital Trace (for DPP-PHA it is ALWAYS Trigger)
	      
            for(int np=0; np<10000; np++) { 
	      if(np<size) {
                dataDT5780[4*ch][np] = WaveLine1[np]; 
                dataDT5780[4*ch+1][np] = WaveLine2[np];
                dataDT5780[4*ch+2][np] = DigitalWaveLine1[np]; 
                dataDT5780[4*ch+3][np] = DigitalWaveLine2[np];
                //printf("np %i    w1 %i    w2 %i    d1 %i    d2 %i\n",np,WaveLine1[np],WaveLine2[np],DigitalWaveLine1[np],DigitalWaveLine2[np]);
              } 
              else {
                dataDT5780[4*ch][np]   = 0; 
                dataDT5780[4*ch+1][np] = 0;
                dataDT5780[4*ch+2][np] = 0; 
                dataDT5780[4*ch+3][np] = 0;
              }
	    } 

	    if(ch==0) { //Only channel 0
 
              SumSamples=0;
              for(int np=0; np<300; np++) {
                SumSamples+=WaveLine2[np];
              }
              CommonMode=SumSamples/300;

              myEnergy=0; // Energy evaluated as max value of the input analogical signal
              for(int np=0; np<size; np++) {
                if(DigitalWaveLine2[np]>0) TriggerTime=np*10;
                if((WaveLine2[np]-CommonMode)>myEnergy) {
		  myEnergy=WaveLine2[np]-CommonMode;
		  xMax=np*10;  //(ns)
                }
              }
              printf("Event  %i   TriggerTime (ns) %6.1f \n",nEvents,TriggerTime);
	    }  // end ch==0 block
	  }
	}
      }  // loop sui canali

      myEvt->StoreHPGe(HPGeEnergy,timeStamp,status,CommonMode,xMax);
      hpgeReady=true;
      if(deviceConfig==2) { myEvt->FillTree(); hpgeReady=false; }

      if(writeRaw) treeDT5780->Fill();                        
      //---------------------------
    } // end HPGe - DT5780 board
      //---------------------------
                     
    //-------------------------------------------------
    else {            // Pamela siStrip - V1495 module
    //-------------------------------------------------
      if(!(runConfig & (0x1 << 3))) {
        printf("*** ERROR: Reading V1495 data block but SiStrip is not present in the run configuration\n");
        exit(1);
      }
      if(pamsiReady) {
       printf("*** ERROR: pamsiReady=true when reading V1495 data block\n");
       printf("*** ERROR: A data block is missing, skipping event %i \n",nEvents-1);
       pamsiReady=false;
       //exit(1);
      }

      myEvt->ResetSiStrip();
      nEventsV1495++;
      PamSibuff16=(uint16_t *) buffer;
      for(int j=0; j<1024; j++) {
	dataV1495[0][j]=PamSibuff16[j*2];
	dataV1495[1][j]=PamSibuff16[j*2+1];
      }

      myEvt->StoreSiStrip(dataV1495,timeStamp,status);
      pamsiReady=true;
      if(deviceConfig==4) { myEvt->FillTree(); pamsiReady=false; }

      //---------------------------
    } // end PamSi
      //---------------------------

    switch(deviceConfig) {
      case 3 :
        if(bafReady && hpgeReady) {
         myEvt->FillTree();
          bafReady=false; hpgeReady=false;
          break;
        }
      case 5 :
        if(bafReady && pamsiReady) {
          myEvt->FillTree();
          bafReady=false; pamsiReady=false;
          break;
        }
      case 6 :
        if(hpgeReady && pamsiReady) {
          myEvt->FillTree();
          hpgeReady=false; pamsiReady=false;
          break;
        }
      case 7 :  
        if(bafReady && hpgeReady && pamsiReady) {
          myEvt->FillTree();
          bafReady=false; hpgeReady=false; pamsiReady=false;
          break;
        }
    }
       
    //---------------------------
  } // end while loop on data
    //---------------------------
    
  printf("\nEvent statistics:\n");
  printf(" nEventsV1742Signal %i\n nEventsV1742Alfa %i\n nEventsDT5780 [Ch0] %i\n nEventsDT5780 [Ch1] %i\n nEventsV1495 %i\n\n", 
           nEventsV1742Signal, nEventsV1742Alfa, nEventsDT5780Ch0, nEventsDT5780Ch1, nEventsV1495);

  fclose(file);

  free(Evt742);

  if(writeRaw) {
    if(runConfig & (0x1 << 0)) {
      frawV1742->Write();
      frawV1742->Close();
    }

    if(runConfig & (0x1 << 1)) {
      frawDT5780->Write();
      frawDT5780->Close();
    }

    if(runConfig & (0x1 << 3)) {
      frawV1495->Write();
      frawV1495->Close();
    }
  }

  if(runConfig & (0x1 << 0)) delete myAlpha;
  delete myEvt;
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



int HPGeInit() {
  for(int ch=0; ch<2; ch++) {
    Events[ch] = (CAEN_DGTZ_DPP_PHA_Event_t *) malloc (sizeof (CAEN_DGTZ_DPP_PHA_Event_t)); 
    if (Events[ch] == NULL) return -1;
    memset (Events[ch], 0, sizeof (CAEN_DGTZ_DPP_PHA_Event_t));
  }

  Waveform = (CAEN_DGTZ_DPP_PHA_Waveforms_t *) malloc(sizeof(CAEN_DGTZ_DPP_PHA_Waveforms_t));
  if (Waveform == NULL) return -1;
  memset (Waveform, 0, sizeof (CAEN_DGTZ_DPP_PHA_Waveforms_t));
  
  Waveform->Trace1=(int16_t*) malloc(30000);
  Waveform->Trace2=(int16_t*) malloc(30000);
  Waveform->DTrace1=(uint8_t*) malloc(30000);
  Waveform->DTrace2=(uint8_t*) malloc(30000);

}

