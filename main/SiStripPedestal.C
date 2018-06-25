/**********************************************************************************************
* per compilare in modo standalone: 
* scl enable devtoolset-2 tcsh
* g++ -std=gnu++11 -g `root-config --cflags --libs` src/SiStripPedestal.C -o ./SiStripPedestal
*
*  usage: ./SiStripPedestal -f datafile
* 
***********************************************************************************************/
#include <sys/time.h>
#include <time.h>

#include "../CAENDigitizer.h"
#include "../include/SiPedestal.h"

#define headerSize 4
#define scstatusSize 4
#define timeSecSize 4
#define timenSecSize 4
#define runHeaderSize 4

#define nStrip 1024
#define nChip 8
#define nStripChip 128

void LoopOnData(int);
int ChipNoise(int);
void PrintSummary();

double xPed[nStrip], xSigma[nStrip], yPed[nStrip], ySigma[nStrip];
int xBadChannel[nStrip],yBadChannel[nStrip]; 
double xSigmaMeanChip[nChip], xNoiseChip[nChip], xCutNoiseChip[nChip], ySigmaMeanChip[nChip], yNoiseChip[nChip], yCutNoiseChip[nChip];
double xCMN[nChip], yCMN[nChip];
int nx[nChip], ny[nChip];

char* buffer = NULL;
char* header = NULL;
FILE* file;
uint32_t bufferSize=0;

uint16_t* SiBuff16;

uint16_t dataV1485[2][1024];

double sumx[nStrip], sumx2[nStrip], sumy[nStrip], sumy2[nStrip];
int nEvents;

SiPedestal* mySiPed;

int main(int argc, char **argv) {
  char* scstatus= NULL;
  char* timesec= NULL;
  char* timensec= NULL;
  char* runconfig= NULL;
  char* runnumber= NULL;
  char* fileName=NULL; 
  int myc=0;
  int timeStampSize=timeSecSize+timenSecSize;
  uint32_t evTime=0,startTime=0,nsecTime=0,nsecStartTime=0;
  Double_t timeStamp;

  SiBuff16 = (uint16_t *)malloc(4096);
  
  while ((myc = getopt (argc, argv, "f:")) != -1) {
    switch (myc)
      {
      case 'f':
	fileName=(char *) malloc (strlen (optarg) + 1);
	strcpy ((char *) fileName, optarg);
	break;
      }
  }

  if(fileName==NULL) {
    printf("usage: SiStripPedestal -f fileName \n");
    exit(1);
  }
  file = fopen (fileName, "r");


  header=(char*) malloc(headerSize);
  timesec=(char*) malloc(timeSecSize);
  timensec=(char*) malloc(timenSecSize);
  scstatus=(char*) malloc(scstatusSize);

  // Read the run number
  runnumber=(char*) malloc(runHeaderSize);
  if(!fread(runnumber,runHeaderSize,1,file)) {
    printf("*** Cannot find the run number! ");
    return(0);
  }
  uint32_t runNumber=*(uint32_t *)(runnumber);
  printf("SiStripPedestal: building events for run %i \n",runNumber);
    
  char rootFile[30],rn[10];
  strcpy(rootFile,"Run");
  sprintf(rn,"%i",runNumber);
  strcat(rootFile,rn);
  strcat(rootFile,"_SiStripPedestal.root");
  mySiPed= new SiPedestal(fileName,rootFile);
    
  // Read the run configuration
  runconfig=(char*) malloc(runHeaderSize);
  if(!fread(runconfig,runHeaderSize,1,file)) {
    printf("*** Cannot find the run configuration! ");
    exit(1);
  }
  uint32_t runConfig=*(uint32_t *)(runconfig);
  if(!(runConfig & 0x0000F000) || !(runConfig & 0x000F0000)) {
    printf("*** ERROR: SiStrip and SoftwareTrigger are not present in the run configuration\n");
    //exit(1);
  }


////////////////////////////////////////////////////////////////////////////////////
// Iterate evaluation of pedestal and sigma until no bad channels are found
////////////////////////////////////////////////////////////////////////////////////
  for(int i=0; i<nStrip; i++) { xBadChannel[i]=0; yBadChannel[i]=0; } 
 
  int np=0;
  int LoopAgain=1;
  while(LoopAgain) {
    mySiPed->FillnLoop(np);
    printf("\nLoop on data: %i \n",np);
    rewind (file);
    fseek(file, 8, SEEK_CUR);  //Skip 2 words (runNumber+ runConfiguration)
    LoopOnData(np);
    mySiPed->FillPedestal(xPed,yPed,np);
    mySiPed->FillSigma(xSigma,ySigma,np);
    LoopAgain=ChipNoise(np);
    mySiPed->FillBadChannel(xBadChannel,yBadChannel,np);
    np++;
  }

  PrintSummary();

  delete mySiPed;    
  fclose(file);

  int err_code;
  char command[200];
  strcpy(command,"");
  strcpy(command,"mv -f ./"); 
  strcat(command,rootFile);
  strcat(command," Setup/pedestals/");
  err_code=system(command);

  strcpy(command,"");
  strcat(command,"ln -fs Setup/pedestals/");
  strcat(command,rootFile);
  strcat(command," SiStripPedestal.root");
  err_code=system(command);

}


////////////////////////////////////////////////////////////////////////////////////
void LoopOnData(int pass) {
////////////////////////////////////////////////////////////////////////////////////
  nEvents=0;
  for(int i=0; i<nStrip; i++) {
    sumx[i]=0.; sumx2[i]=0; 
    sumy[i]=0.; sumy2[i]=0;
  }  

  while(!feof(file)) {    
    nEvents++;
    fseek(file, 12, SEEK_CUR); //Skip 3 words (status+2xtimestamps)
    if(!fread(header,headerSize,1,file)) break;
    bufferSize=*(long *) (header) & 0x0FFFFFFF;
    bufferSize=bufferSize*4;
    buffer=(char *)malloc(bufferSize);	
    fseek(file, -4, SEEK_CUR);
    if(!fread(buffer, bufferSize, 1, file)) break;

    SiBuff16=(uint16_t *) buffer;

    if(pass==0) mySiPed->FillRawData(SiBuff16);

    // CommonNoise
    int chip=0;
    for(int i=0; i<nChip; i++) {
      xCMN[i]=0.; yCMN[i]=0.; nx[i]=0; ny[i]=0; 
    }  
    if(pass>0) {  // Special treatment for the first pass - Assume CommonNoise=0 
      for(int j=0; j<nStrip; j++) {
        chip = (int) j/nStripChip;
        //if(xBadChannel[j]==0) {
        if(xBadChannel[j]==0  && SiBuff16[j*2]<4000) {
          xCMN[chip] += ((double) SiBuff16[j*2]) - xPed[j];
          nx[chip]++;
        }
        //if(yBadChannel[j]==0) {
        if(yBadChannel[j]==0 && SiBuff16[j*2+1]<4000) {
          yCMN[chip] += ((double) SiBuff16[j*2+1]) - yPed[j];
          ny[chip]++;
        }
      }

      for(int j=0; j<nChip; j++) {
        xCMN[j] /= nx[j];
        yCMN[j] /= ny[j];
        //printf("xChip %i    nx %i     xCommonNoise %f  \n",j,nx[j],xCMN[j]);
        //printf("yChip %i    ny %i     yCommonNoise %f  \n",j,ny[j],yCMN[j]);
      }
      mySiPed->FillCommonNoise(xCMN,yCMN,pass);
    }

    // Pedestal and sigma with CommonNoise subtraction
    for(int j=0; j<nStrip; j++) {
      chip= (int) j/nStripChip;
      //if(xBadChannel[j]==0) {
      if(xBadChannel[j]==0 && SiBuff16[j*2]<4000) {
         sumx[j] += (double) SiBuff16[j*2] - xCMN[chip];
         sumx2[j] += ((double) SiBuff16[j*2] - xCMN[chip]) * ((double) SiBuff16[j*2] - xCMN[chip]);
      }
      //if(yBadChannel[j]==0) {
      if(yBadChannel[j]==0 && SiBuff16[j*2+1]<4000) {
         sumy[j] += (double) SiBuff16[j*2+1] - yCMN[chip];
         sumy2[j] += ((double) SiBuff16[j*2+1] - yCMN[chip])*((double) SiBuff16[j*2+1] - yCMN[chip]);
      }
    }

  } // end while loop on data
  free (buffer);  
  nEvents = nEvents-1;
  printf("Pass %i - n. of events read: %i \n", pass,nEvents);

  for(int i=0; i<nStrip; i++) {
    if(xBadChannel[i]==0) {
      xSigma[i] = sqrt((sumx2[i] - (sumx[i]*sumx[i])/nEvents) / (nEvents - 1));
      xPed[i] = sumx[i]/nEvents;
      printf("Pass %i:  xStrip %i   xPed %f    xSigma %f    \n",pass,i,xPed[i],xSigma[i]);
    }
    if(yBadChannel[i]==0) {
      ySigma[i] = sqrt((sumy2[i] - (sumy[i]*sumy[i])/nEvents) / (nEvents - 1));
      yPed[i] = sumy[i]/nEvents;
      printf("Pass %i:  yStrip %i   yPed %f    ySigma %f    \n",pass,i,yPed[i],ySigma[i]);
    }
  }

}


////////////////////////////////////////////////////////////////////////////////////
int ChipNoise(int pass) {
////////////////////////////////////////////////////////////////////////////////////
  int chip=0;

  for(int c=0; c<nChip; c++) {
    xSigmaMeanChip[c]=0.; xNoiseChip[c]=0.; xCutNoiseChip[c]=0.; 
    ySigmaMeanChip[c]=0.; yNoiseChip[c]=0.; yCutNoiseChip[c]=0.;
    nx[c]=0; ny[c]=0; 
  }  

  for(int j=0; j<nStrip; j++) {
    chip = (int) j/nStripChip;
    if(xBadChannel[j]==0) {
      xSigmaMeanChip[chip] +=  xSigma[j];
      xNoiseChip[chip] += xSigma[j]*xSigma[j];
      nx[chip]++;
    }
    if(yBadChannel[j]==0) {
      ySigmaMeanChip[chip] += ySigma[j];
      yNoiseChip[chip] += ySigma[j]*ySigma[j];
      ny[chip]++;
    }
  }

  for(int c=0; c<nChip; c++) {
    xNoiseChip[c] = sqrt((xNoiseChip[c] - (xSigmaMeanChip[c]*xSigmaMeanChip[c])/nx[c])/(nx[c] - 1));
    xSigmaMeanChip[c] /= nx[c];
    xCutNoiseChip[c]= mySiPed->GetNoiseCut(1,pass,c,xSigmaMeanChip[c],xNoiseChip[c]);
    printf("xChip %i   nx %i     xSigmaMeanChip %f    xNoiseChip %f    xCutNoiseChip %f\n",
      c,nx[c],xSigmaMeanChip[c],xNoiseChip[c],xCutNoiseChip[c]);
 
    yNoiseChip[c] = sqrt((yNoiseChip[c] - (ySigmaMeanChip[c]*ySigmaMeanChip[c])/ny[c])/(ny[c] - 1));
    ySigmaMeanChip[c] /= ny[c];
    yCutNoiseChip[c]= mySiPed->GetNoiseCut(0,pass,c,ySigmaMeanChip[c],yNoiseChip[c]);
    printf("yChip %i   ny %i     ySigmaMeanChip %f    yNoiseChip %f    yCutNoiseChip %f\n",
      c,ny[c],ySigmaMeanChip[c],yNoiseChip[c],yCutNoiseChip[c]);
  }


  int sumBadx=0, sumBady=0;
  for(int j=0; j<nStrip; j++) {
    chip = (int) j/nStripChip;
    if(xBadChannel[j]==0) {
      if(abs(xSigma[j]-xSigmaMeanChip[chip]) > 5*min(xNoiseChip[chip],xCutNoiseChip[chip]) ) {
        printf("Chip %i   xStrip %i   sigma %f   cut %f   ***BAD x strip \n",chip,j,xSigma[j],5*min(xNoiseChip[chip],xCutNoiseChip[chip]));
        xBadChannel[j]=1;  xPed[j]=0; xSigma[j]=0;  sumBadx++;  
      }
    }
    if(yBadChannel[j]==0) {
      if(abs(ySigma[j]-ySigmaMeanChip[chip]) > 5*min(yNoiseChip[chip],yCutNoiseChip[chip]) ) {
        printf("Chip %i   yStrip %i   sigma %f   cut %f   ***BAD y strip \n",chip,j,ySigma[j],5*min(yNoiseChip[chip],yCutNoiseChip[chip]));
        yBadChannel[j]=1;  yPed[j]=0; ySigma[j]=0;  sumBady++;
      }
    }

  }
  printf("Pass %i: xBadChannel %i    yBadChannel %i\n",pass,sumBadx,sumBady);

  return(sumBadx+sumBady);
}



////////////////////////////////////////////////////////////////////////////////////
void PrintSummary() {
////////////////////////////////////////////////////////////////////////////////////
  int chip=0,sumx=0,sumy=0;

  printf("\n*************************************** \n");
  printf("*** Bad strip summary \n");
  printf("*************************************** \n");
  for(int i=0; i<nStrip; i++) {
    chip = (int) i/nStripChip;
    if(xBadChannel[i]==1) {
      printf("xStrip %i   Chip %i\n",i,chip);
      sumx++;
    }
  }
  printf("\n");
  for(int i=0; i<nStrip; i++) {
    chip = (int) i/nStripChip;
    if(yBadChannel[i]==1) {
      printf("yStrip %i   Chip %i\n",i,chip);
      sumy++;
    }
  }
  printf("\n*** Total number of bad strips: xSide %i   ySide %i \n",sumx,sumy);

  printf("\n*************************************** \n");
  printf("*** Strip pedestal and sigma \n");
  printf("*************************************** \n");
  for(int i=0; i<nStrip; i++) {
    chip = (int) i/nStripChip;
    printf("xStrip %i    Chip %i   ped %f    sigma %f\n",i,chip,xPed[i],xSigma[i]);
  }
  printf("\n");
  for(int i=0; i<nStrip; i++) {
    chip = (int) i/nStripChip;
    printf("yStrip %i    Chip %i   ped %f    sigma %f\n",i,chip,yPed[i],ySigma[i]);
  }

}

