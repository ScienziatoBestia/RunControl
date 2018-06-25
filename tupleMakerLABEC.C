/**********************************************************************************************
* per compilare in modo standalone: 
* scl enable devtoolset-2 tcsh
* g++ -std=gnu++11 -g `root-config --cflags --libs` src/tupleMakerGCAL.C -o ./tupleMakerGCAL
*
*  usage: ./tupleMakerGCAL -f datafile -r 1 -n 1
* 
***********************************************************************************************/
#include <sys/time.h>
#include <time.h>

#include "../include/x742.h"
#include "../include/X742CorrectionRoutines.h"
#include "../CAENDigitizer.h"
#include "../include/GCALTree.h"

#define headerSize 4 
#define timeSecSize 4
#define timenSecSize 4
#define runHeaderSize 4

/* Board Id (registro 0xEF08) */
#define V1742CaloId  4026531840 //0x1E


int main(int argc, char **argv) {
  int ret;
  char* buffer = NULL;
  char* header = NULL;
  char* timesec= NULL;
  char* timensec= NULL;
  char* runheader= NULL;
  FILE* file;
  int nEvents=0;
  int verbose=1;
  CAEN_DGTZ_X742_EVENT_t *Evt742= NULL;
  CAEN_DGTZ_DRS4Correction_t X742Tables[MAX_X742_GROUP_SIZE];
  CAEN_DGTZ_DRS4Frequency_t freq;
  int newTree=0, writeRaw=0;
  uint32_t bufferSize=0, test, boardId;
  int i, j, k, o, l;
  int corrTableLoaded = 0;
  char stringa[1000];
  //double dataV1742[36][1024];
  double dataV1742[18][1024];
  char* fileName=NULL;
  char  raw, newt;
  int myc=0;
  int timeStampSize=timeSecSize+timenSecSize;
  
  uint32_t evTime=0,startTime=0,nsecTime=0,nsecStartTime=0;
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
        break;
      case 'n':
	strcpy(&newt,optarg);
	newTree=atoi(&newt);
        break;
      }
  }

  if(fileName==NULL) {
    printf("usage: tupleMakerGCAL -f fileName [-r 1] [-n 1] \n");
    return(0);
  }
  file = fopen (fileName, "r");

  GCALTree* mySignalClass= new GCALTree("./signalTreeGCAL.root",newTree);

  TFile *frawV1742;
  TTree *treeV1742;
  if(writeRaw) {
    frawV1742=new TFile("./GCALRawDataV1742.root","recreate");
    treeV1742=new TTree("rawdata","rawdata");
   // treeV1742->Branch("dataV1742",dataV1742,"dataV1742[36][1024]/D ");
    treeV1742->Branch("dataV1742",dataV1742,"dataV1742[18][1024]/D ");
  }


  header=(char*) malloc(headerSize);
  timesec=(char*) malloc(timeSecSize);
  timensec=(char*) malloc(timenSecSize);

  // Read the run header -> to be used with new data ***NOT FOR LABEC TEST DATA!!
//  runheader=(char*) malloc(runHeaderSize);
//  if(!fread(runheader,runHeaderSize,1,file)) {
//    printf("*** Cannot find the run header! ");
//    return(0);
//  }
//  uint32_t runHeader=*(uint32_t *)(runheader);
    
 //------------------------------------------------------------
  while(!feof(file)) {                 //Main loop on datafile
 //------------------------------------------------------------
    nEvents++;
    if((nEvents % 100)==0) printf("Event %i \n", nEvents);

//    if(!fread(timesec,timeSecSize,1,file)) break;
//    evTime=*(uint32_t *)(timesec);
//    if(nEvents==1) startTime=evTime;
//    time_t mysec=*(uint32_t *)(timesec);
//    //if((nEvents % 100)==0)  printf("DataTime: %s \n",ctime(&mysec));

//    if(!fread(timensec,timenSecSize,1,file)) break;
//    nsecTime=*(uint32_t*)(timensec);
//    if(nEvents==1) nsecStartTime=nsecTime;
//    timeStamp=(evTime-startTime)*1.e9 + nsecTime - nsecStartTime;
//    timeStamp /= 1.e9;
    timeStamp = 0.;


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
//    if(boardId == V1742CaloId) {    // V1742 GCAL digitizer
    //------------------------------------------------------
//      if(runHeader != 0x00F00000) {
//       printf("*** ERROR: Reading Calo data block but GCAL is not present in the runHeader\n");
//       exit(1);
//      }

      printf("board id %u \n",boardId);

      mySignalClass->ResetCalo();

      X742_DecodeEvent (buffer, (void **)&Evt742);

      for(o=0; o<MAX_X742_GROUP_SIZE; o++) {
if(o<2) { 
        for(j=0; j<MAX_X742_CHANNEL_SIZE; j++) { 
          for(k = 0; k < 1024 ; k++) {	 		    
            dataV1742[j+9*o][k]=-100;
          }
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
               if(o<2) dataV1742[j+9*o][k]=Evt742->DataGroup[o].DataChannel[j][k];
             }
             free(Evt742->DataGroup[o].DataChannel[j]); // Essendo DataChannel un puntatore, 
                                                        // va deallocato manualmente, cioe' non 
                                                        // basta solo il free(Evt742) alla fine
           }
        } 
      }
 
      mySignalClass->StoreCalo(dataV1742,timeStamp);
      mySignalClass->FillTree();

      if(writeRaw) treeV1742->Fill();

      //---------------------------
//    } // end v1742 Digitizer
      //---------------------------
 

    //---------------------------
  } // end while loop on data
    //---------------------------
    
  printf(" GCAL read %i events \n ", nEvents);

  fclose(file);
  free(Evt742);

  if(writeRaw) {
    frawV1742->Write();
    frawV1742->Close();
  }

  delete mySignalClass;
}
