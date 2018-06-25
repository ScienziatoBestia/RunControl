#include "x742.h"
#include "X742CorrectionRoutines.h"
#include "CAENDigitizerType.h"
#include <CAENDigitizer.h>
#include "DecodeDT5780.h"
#include "SharedMemorySetup.h"
#include "Define.h"

/*************************************************************************************************************/
void removeTempFile(char* filename , char* device) {
/*************************************************************************************************************/
  if( access(filename, F_OK ) != -1 ) {
    char command[100];
    strcpy(command,"rm -f ");
    strcat(command,filename);
    system(command);
    printf("%s Plotter: Removing %s \n",device,filename);
  } 
  else printf("%s Plotter: File %s not exists \n",device,filename);

}
/*************************************************************************************************************/


int main(int argc, char **argv) {
  int ret, currRun;
  char* myRead = NULL;  
 
  DaqSharedMemory* plDaqSharedMemory;
  circBuffer* plSharedBuffer;
  FILE * tempV1742, *tempDT5780, *tempV1495;
  int pltail;
  int myc=0;
  CAEN_DGTZ_X742_EVENT_t *Evt742= NULL;
  CAEN_DGTZ_DPP_PHA_Event_t       *Events[2] ={NULL,NULL};// events buffer
  CAEN_DGTZ_DPP_PHA_Waveforms_t   *Waveform=NULL;     // waveforms buffer
  int16_t *WaveLine1,*WaveLine2;
  uint8_t *DigitalWaveLine1;
  uint8_t *DigitalWaveLine2;

  // int o,j,k,i,ev;
  int corrTableLoaded = 0;
  CAEN_DGTZ_DRS4Correction_t X742Tables[MAX_X742_GROUP_SIZE];
  char stringa[1000];
  int nEventsDT5780, nEventsV1742, nEventsV1495;
  int Channel;
  FILE * gnuplotV1742, *gnuplotDT5780, *gnuplotV1495;
  char Device[10];
  int isGCAL=0, isCSPEC=0, isNRSS=0;
  uint16_t	*PamSibuff16 ;         // 
  uint16_t	*PamSiXside ;         // 
  uint16_t	*PamSiYside ;         // 
  uint32_t test, bufferSize=0;
  int size;
  int bufcnt=0;
  char chanMask;
  uint32_t boardId;
  uint32_t NumEvents[2]={1,1};
  int scaler;
  int group;
  float TimeScaler;
  CAEN_DGTZ_DRS4Frequency_t freq;
  char* tempfileV1742GCAL = "dataV1742GCAL.temp";
  char* tempfileV1742BaF = "dataV1742BaF.temp";
  char* tempfileDT5780 = "dataDT5780.temp";
  char* tempfileV1495 = "dataV1495.temp";
  int isPlotGroup;
  char logFilename[100];

  while ((myc = getopt (argc, argv, "d:")) != -1)
    switch (myc)
      {
      case 'd':
	strcpy ((char *) Device, optarg);
	if(strstr(Device, "GCAL")!=NULL) isGCAL=1;
	else if(strstr(Device, "CSPEC")!=NULL) isCSPEC=1;
	else if(strstr(Device, "NRSS")!=NULL) isNRSS=1;
	break;
      }
  printf("%s Plotter: Configuring SharedMemory \n",Device);
  plDaqSharedMemory=configDaqSharedMemory("Plotter");

  sprintf(logFilename,"Log/LogFile_Run%i",plDaqSharedMemory->runNumber);
  freopen (logFilename,"a",stdout);

  printf("%s Plotter: Configuring SharedBuffer \n",Device);
  if(isCSPEC) {
    plSharedBuffer=configSharedBuffer("Plotter",keyCSPEC);
    gnuplotDT5780 = popen ("gnuplot 2>/dev/null", "w");
    fprintf(gnuplotDT5780, "set xlabel '%s'\n", "Time (ns)");
    fprintf(gnuplotDT5780, "set ylabel '%s'\n", "ADC counts");
    fprintf(gnuplotDT5780, "set yrange [0:16384]\n");

    gnuplotV1495 = popen ("gnuplot 2>/dev/null", "w");
    fprintf(gnuplotV1495, "set xlabel '%s'\n", "# Strip");
    fprintf(gnuplotV1495, "set ylabel '%s'\n", "ADC counts");
    fprintf(gnuplotV1495, "set yrange [0:4096]\n");
  }
  else if(isGCAL) {
    plSharedBuffer=configSharedBuffer("Plotter",keyGCAL);
  }
  else if(isNRSS) {
    plSharedBuffer=configSharedBuffer("Plotter",keyNRSS);
  }

  gnuplotV1742 = popen ("gnuplot 2>/dev/null", "w");
  fprintf(gnuplotV1742, "set xlabel '%s'\n", "Time (ns)");
  fprintf(gnuplotV1742, "set ylabel '%s'\n", "ADC counts");
  fprintf(gnuplotV1742, "set yrange [0:4096]\n");
  //fprintf(gnuplotV1742, "set xrange [0:1000]\n");
  //fprintf(gnuplot, "set autoscale y\n");

  pltail=plSharedBuffer->head;	

  if(isCSPEC) {
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

    PamSibuff16 = (uint16_t *)malloc(4096);
    PamSiXside = (uint16_t *)malloc(4096);
    PamSiYside = (uint16_t *)malloc(4096);
  }

  nEventsV1742=0;
  nEventsDT5780=0;
  nEventsV1495=0;

  int stop=0;
  currRun=plDaqSharedMemory->runNumber;

  while(!stop) {

    if(plDaqSharedMemory->runNumber!=currRun) {
      currRun=plDaqSharedMemory->runNumber;
      sprintf(logFilename,"Log/LogFile_Run%i",plDaqSharedMemory->runNumber);
      freopen (logFilename,"a",stdout);
      printf("\n%s Plotter: Starting new run %i\n",Device,currRun);
    }

    if(isCSPEC) scaler=plDaqSharedMemory->CSPECplotScaler;
    else if(isGCAL) scaler=plDaqSharedMemory->GCALplotScaler;
    else if(isNRSS) scaler=plDaqSharedMemory->NRSSplotScaler;

    if(plSharedBuffer->head < pltail) pltail=plSharedBuffer->head;

    if(plSharedBuffer->head == pltail) {
      if(isCSPEC) stop=plDaqSharedMemory->stopCSPECplotter;
      else if(isGCAL) stop=plDaqSharedMemory->stopGCALplotter;
      else if(isNRSS) stop=plDaqSharedMemory->stopNRSSplotter;
      usleep(1000); // waiting for new data
      continue; 
    }

    bufferSize=readEventSize(plSharedBuffer, pltail);
    pltail+=evtHeaderSize;
    myRead=(char*)malloc(bufferSize);  
    
    pltail = readCircularBuffer(plSharedBuffer,myRead,bufferSize,pltail);

    chanMask = *(long *)(myRead+caenHeaderSize) & 0x0000000F;
    boardId =  *(long *)(myRead+caenHeaderSize) & 0xF8000000;
    if(boardId == V1742BaFId) Channel=plDaqSharedMemory->PlotChannelBaF;
    else if(boardId == V1742GCALId) Channel=plDaqSharedMemory->PlotChannelGCAL;
    else if(boardId == DT5780Id) Channel=plDaqSharedMemory->PlotChannelHPGe;

    if(boardId == V1742BaFId || boardId == V1742GCALId ) {  // e' V1742
      nEventsV1742++;

      if((nEventsV1742%scaler)==0) {
	X742_DecodeEvent (myRead, (void **)&Evt742);
	int group,nch;
	if((isGCAL && plDaqSharedMemory->isGCALplotGroup) || (isCSPEC && plDaqSharedMemory->isBaFplotGroup) ||
           (isNRSS && plDaqSharedMemory->isNRSSplotGroup) ) {
	  isPlotGroup=1;
	  group=Channel;
	  nch = 0;
	} 
        else {
	  isPlotGroup=0;
	  group=(int) Channel/9 ;
	  nch=Channel%9;
	}

	if(Evt742->GrPresent[group] == 1) {	    
	  if(!corrTableLoaded) {
	    for (int i=0; i<MAX_X742_GROUP_SIZE; i++) {
	       bzero(stringa, 1000);
	       if(isGCAL) sprintf(stringa, "CorrectionTables/Calo/Corrections_freq%d_gr%d",Evt742->DataGroup[group].Frequency, i);
	       else sprintf(stringa, "CorrectionTables/BaF/Corrections_freq%d_gr%d",Evt742->DataGroup[group].Frequency, i);
	       LoadCorrectionTable(stringa, &(X742Tables[i]));
	    }
	    corrTableLoaded = 1;
	    switch(Evt742->DataGroup[group].Frequency)
	     {
	      case 0:
		freq = CAEN_DGTZ_DRS4_5GHz;
		TimeScaler = 0.2;
		break;
	      case 1:
		freq = CAEN_DGTZ_DRS4_2_5GHz;
		TimeScaler = 0.4;
		break;
		case 2:
		  freq = CAEN_DGTZ_DRS4_1GHz;
		  TimeScaler = 1.0;
		  break;
	     }
	    
	  }
	  fflush(stdout);
	  ApplyDataCorrection(&(X742Tables[group]), freq, 7, &(Evt742->DataGroup[group]));
	  if(isGCAL) tempV1742 = fopen(tempfileV1742GCAL, "w");
	  else tempV1742 = fopen(tempfileV1742BaF, "w");

	  for (int k = 0; k < 1000; k++) {
	      fprintf(tempV1742, "%f\t",k*TimeScaler);
	      for (int jj = 0; jj < 9; jj++) 
		if(isPlotGroup || jj==nch) fprintf(tempV1742, "%f\t",Evt742->DataGroup[group].DataChannel[jj][k]); 
	      fprintf(tempV1742, "\n");
	   }

	   fprintf(gnuplotV1742, "plot ");
	    for(int jj=0; jj<9; jj++) { 
	      if(isPlotGroup && jj>0) 
		fprintf(gnuplotV1742, ", ");
	      if(isPlotGroup==1 || jj==nch) {
		int column;
		if(isPlotGroup) column=jj+2;
		else column=2;
		if(isGCAL) fprintf(gnuplotV1742, "'%s' ",tempfileV1742GCAL );
		else fprintf(gnuplotV1742, "'%s' ", tempfileV1742BaF);
		if(jj==8) fprintf(gnuplotV1742, "using ($1):($%i) title '%s GR%i TR%i' with step linecolor %d ", column, Device, group,group/2, jj+1);
		else fprintf(gnuplotV1742, "using ($1):($%i) title '%s GR%i CH%i' with step linecolor %d ", column,Device,group,jj, jj+1);
		free(Evt742->DataGroup[group].DataChannel[jj]);
	      }
	    }
	    fprintf(gnuplotV1742, "\n"); 
	    fflush(gnuplotV1742);
	    fclose(tempV1742);
	    //	    free(Evt742->DataGroup[group].DataChannel[j]);
	  } //  if (Evt742->GrPresent[o] == 1)
      } //  if((nEvents%100)==0)
    } 
    else if(boardId == DT5780Id) {    // e' DT5780

      if ((chanMask >> Channel) & 0x1) 
	nEventsDT5780++;

      if((nEventsDT5780%scaler)==0) {
	ret = DT5780_DPP_PHA_GetDPPEvents(myRead, bufferSize,  Events, NumEvents);
	for (int ev = 0; ev < NumEvents[Channel]; ev++) {	      
	  DT5780_DPP_PHA_DecodeDPPWaveforms(&Events[Channel][ev], Waveform);
	  
	  size = (int)(Waveform->Ns); // Number of samples
	  WaveLine1 = Waveform->Trace1; // Trapezoid
	  WaveLine2 = Waveform->Trace2; // Input
	  DigitalWaveLine1 = Waveform->DTrace1; // Peaking
	  DigitalWaveLine2 = Waveform->DTrace2; // Trigger
	  tempDT5780 = fopen(tempfileDT5780, "w");
	  for(int np=0; np<size; np++) 
	    fprintf(tempDT5780, "%i %d %d %d %d \n",np*10,WaveLine1[np],WaveLine2[np],1000*DigitalWaveLine1[np],1000*DigitalWaveLine2[np]); //Write the data to a temporary file
	  fprintf(gnuplotDT5780,"plot ");
	  fprintf(gnuplotDT5780,"'%s' using ($1):($2) title 'HpGe Ch%i Trapezoid'  with step linecolor 1",tempfileDT5780, Channel);
	  fprintf(gnuplotDT5780, ", ");
	  fprintf(gnuplotDT5780,"'%s' using ($1):($3) title 'HpGe Ch%i Input' with step linecolor 2",tempfileDT5780, Channel);
	  fprintf(gnuplotDT5780, ", ");
	  fprintf(gnuplotDT5780,"'%s' using ($1):($4) title 'HpGe Ch%i Peaking' with step linecolor 3",tempfileDT5780, Channel);
	  fprintf(gnuplotDT5780, ", ");
	  fprintf(gnuplotDT5780,"'%s' using ($1):($5) title 'HpGe Ch%i Trigger' with step linecolor 4",tempfileDT5780, Channel);
	  fprintf(gnuplotDT5780, "\n"); 
	  fflush(gnuplotDT5780);
	  fclose(tempDT5780);
	}
      }
    } 
    else {   // e' V1495
      nEventsV1495++;
      if((nEventsV1495%scaler)==0) {
	tempV1495 = fopen(tempfileV1495, "w");
	PamSibuff16=(uint16_t *) myRead;

	for (int j=0;j<(1024);j++) {
	  PamSiXside[j]=PamSibuff16[j*2];
	  PamSiYside[j]=PamSibuff16[j*2+1];
	  fprintf(tempV1495, "%i %i %i \n",j, PamSiXside[j], PamSiYside[j]); //Write the data to a temporary file
	}
	fprintf(gnuplotV1495, "plot ");
	fprintf(gnuplotV1495, "'%s' using ($1):($2) title '%s'",tempfileV1495,"X side");
	fprintf(gnuplotV1495, ", ");
	fprintf(gnuplotV1495, "'%s' using ($1):($3) title '%s'",tempfileV1495,"Y side");
	fprintf(gnuplotV1495, "\n"); 
	fflush(gnuplotV1495);
	fclose(tempV1495);
      }
    }

    free(myRead);
    if(isCSPEC) stop=plDaqSharedMemory->stopCSPECplotter;
    else if(isGCAL) stop=plDaqSharedMemory->stopGCALplotter;
    else if(isNRSS) stop=plDaqSharedMemory->stopNRSSplotter;
  }  

  pclose(gnuplotV1742);
  if(isCSPEC){
    pclose(gnuplotDT5780);
    pclose(gnuplotV1495);
    removeTempFile(tempfileV1742BaF,Device);
    removeTempFile(tempfileDT5780,Device);
    removeTempFile(tempfileV1495,Device);
  }
  else if(isGCAL) removeTempFile(tempfileV1742GCAL,Device);
  printf("\n%s Plotter: Read %i events  \n",Device,nEventsDT5780+nEventsV1742+nEventsV1495);

  printf("%s Detaching the Plotter shared buffer \n",Device);
  if(isCSPEC) deleteSharedBuffer(keyCSPEC,plSharedBuffer,0);
  else if(isGCAL) deleteSharedBuffer(keyGCAL,plSharedBuffer,0);
  else if(isNRSS) deleteSharedBuffer(keyNRSS,plSharedBuffer,0);

  printf("%s Detaching the Plotter shared memory \n",Device);
  deleteDaqSharedMemory(plDaqSharedMemory,0);

  fclose (stdout);

  exit(0);
}
