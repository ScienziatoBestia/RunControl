#include <CAENDigitizer.h>
#include "../include/DT5780Configurator.h"
#include "../include/UtilsFunctions.h"

#define verbose 0

/*************************************************************************************************************/
void WriteConfigFileDT5780(char* filename, DT5780Params_t *Params,CAEN_DGTZ_DPP_PHA_Params_t *DPPParams ) {
/*************************************************************************************************************/
  
  FILE* file = fopen(filename,"w");
  if(Params->LinkType==0) fprintf(file,"LINKTYPE USB \n\n");
  else fprintf(file,"LINKTYPE PCI \n\n");
  fprintf(file,"LINKNUM %i \n\n",Params->LinkNum);
  fprintf(file,"RECORDLENGTH %i \n\n",Params->RecordLength);
  fprintf(file,"PRETRIGGERSIZE %i \n\n",Params->PreTriggerSize);
  if(Params->AcqMode==0) fprintf(file,"ACQUISITIONMODE OSCILLOSCOPE \n\n");
  else if(Params->AcqMode==1) fprintf(file,"ACQUISITIONMODE LIST \n\n");
  else fprintf(file,"ACQUISITIONMODE MIXED \n\n");
  if(Params->IOLevel == 0) fprintf(file,"IOLEVEL NIM \n\n");
  else fprintf(file,"IOLEVEL TTL \n\n");
  fprintf(file,"CHANNELMASK 0x%x \n\n",Params->ChannelMask);
  if(Params->TriggerOut==3) fprintf(file,"TRIGGEROUT OR \n\n");
  else fprintf(file,"TRIGGEROUT AND \n\n");

  if(Params->ExtTrigger==0) fprintf(file,"EXTTRIGGER ENABLE \n\n");
  else fprintf(file,"EXTTRIGGER DISABLE \n\n");
  
  int ch;
  for(ch=0;ch<MaxDT5780NChannels;ch++) {
      fprintf(file,"[CH%i] \n\n",ch);
      if(Params->PulsePolarity[ch]==1) fprintf(file,"POLARITY POSITIVE \n\n");
      else fprintf(file,"POLARITY NEGATIVE \n\n");
      fprintf(file,"TRIGGERTHRESHOLD %i \n\n",DPPParams->thr[ch]);
      fprintf(file,"TRAPEZOIDRISETIME %i \n\n",DPPParams->k[ch]);
      fprintf(file,"TRAPEZOIDFLATTOP %i \n\n",DPPParams->m[ch]);
      fprintf(file,"DECAYTIME %i \n\n",DPPParams->M[ch]);
      fprintf(file,"PEAKINGTIME %i \n\n",DPPParams->ftd[ch]);
      fprintf(file,"TRIGGERSMOOTHING %i \n\n",DPPParams->a[ch]);
      fprintf(file,"INPUTRISETIME %i \n\n",DPPParams->b[ch]);
      fprintf(file,"TRIGGERHOLDOFF %i \n\n",DPPParams->trgho[ch]);
      fprintf(file,"PEAKHOLDOFF %i \n\n",DPPParams->pkho[ch]);
      fprintf(file,"BASELINEHOLDOFF %i \n\n",DPPParams->blho[ch]);
      fprintf(file,"INPUTRANGE %i \n\n",Params->InputRange[ch]);
      fprintf(file,"DCOFFSET %i \n\n",Params->DCOffset[ch]);
      if(Params->TrgMode[ch] == 0) fprintf(file,"TRIGGERMODE INDIVIDUAL \n\n");
      else if(Params->TrgMode[ch] == 1) fprintf(file,"TRIGGERMODE COINCIDENCE \n\n");
      else fprintf(file,"TRIGGERMODE ANTICOINCIDENCE \n\n");
      if(Params->SelfTrigger[ch] == 0x0000000)  fprintf(file,"SELFTRIGGER ENABLE \n\n");
      else if(Params->SelfTrigger[ch] == 0x1000000)  fprintf(file,"SELFTRIGGER DISABLE \n\n");
  }

  fclose(file);
}



/*************************************************************************************************************/
int ParseConfigFileDT5780(char* filename, DT5780Params_t *Params,CAEN_DGTZ_DPP_PHA_Params_t *DPPParams ) {
/*************************************************************************************************************/
  FILE* file = fopen(filename,"r");

  char str[1000], str1[1000];
  int read;
  int value;
  int ch;
  while(!feof(file)) {
    
    read = fscanf(file, "%s", str);
    if( !read || (read == EOF) || !strlen(str)) continue;
    if(str[0] == '#') {fgets(str, 1000, file); continue;}    // skip comments
    if (strstr(str, "LINKTYPE")!=NULL) 
      {
	read = fscanf(file, "%s", str1);
	if (strcmp(str1, "USB")==0)
	  Params->LinkType = CAEN_DGTZ_USB;
	else if (strcmp(str1, "PCI")==0)
	  Params->LinkType = CAEN_DGTZ_OpticalLink;
	 if(verbose) printf("Link Type %s \n", str1);
	continue;
      }  
    else if (strstr(str, "LINKNUM")!=NULL) 
      {
	read = fscanf(file, "%i", &Params->LinkNum);
	 if(verbose) printf("Link Number %i \n", Params->LinkNum);
	continue;
      }  

   else if (strstr(str, "RECORDLENGTH")!=NULL) 
      {
	read = fscanf(file, "%i", &Params->RecordLength);
	 if(verbose) printf("RecordLength %i ns \n", 10*Params->RecordLength);
	continue;
      }  
   else if (strstr(str, "PRETRIGGERSIZE")!=NULL) 
      {
	read = fscanf(file, "%i", &Params->PreTriggerSize);
	 if(verbose) printf("PreTrigger Size %i \n", Params->PreTriggerSize);
	continue;
      }
   else if (strstr(str, "ACQUISITIONMODE")!=NULL) 
      {
	read = fscanf(file, "%s", str1);
	if (strcmp(str1, "LIST")==0)
	  Params->AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_List;
	else if (strcmp(str1, "MIXED")==0)
	  Params->AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;
	else if (strcmp(str1, "OSCILLOSCOPE")==0)
	  Params->AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Oscilloscope;
	 if(verbose) printf("Acquisition Mode %s \n", str1);
	continue;
      }  

   else if (strstr(str, "IOLEVEL")!=NULL) 
      {
	read = fscanf(file, "%s", str1);
	if (strcmp(str1, "NIM")==0)
	  Params->IOLevel = CAEN_DGTZ_IOLevel_NIM;
	else if (strcmp(str1, "TTL")==0)
	  Params->IOLevel = CAEN_DGTZ_IOLevel_TTL;
	 if(verbose) printf("Front Panel IO Level %s \n", str1);
	continue;
      }  

   else if (strstr(str, "CHANNELMASK")!=NULL) 
      {
	read = fscanf(file, "%i", &Params->ChannelMask );
	 if(verbose) printf("Channel Mask %x \n",Params->ChannelMask );
	continue;
      }

   else if (strstr(str, "TRIGGEROUT")!=NULL) 
      {
	read = fscanf(file, "%s", str1 );
	if (strcmp(str1, "AND")==0)
	  Params->TriggerOut = 0x103;
	else if (strcmp(str1, "OR")==0)
	  Params->TriggerOut = 0x3;
	 if(verbose) printf("Trigger Out %s \n",str1 );
	continue;
      }

   else if (strstr(str, "EXTTRIGGER")!=NULL) 
      {
	read = fscanf(file, "%s", str1 );
	if (strcmp(str1, "ENABLE")==0)
	  Params->ExtTrigger = 0x0;
	else if (strcmp(str1, "DISABLE")==0)
	  Params->ExtTrigger = 0x1;
	 if(verbose) printf("External Trigger  %s \n",str1 );
	continue;
      }


    else if (strstr(str, "[CH0]")!=NULL) {
      ch=0;
       if(verbose) printf("[CH%i] \n",ch);
      continue;
    }
    else if (strstr(str, "[CH1]")!=NULL) {
      ch=1;
       if(verbose) printf("[CH%i] \n",ch);
      continue;
    }
    else if (strstr(str, "POLARITY")!=NULL) 
      {
	read = fscanf(file, "%s", str1 );
	if (strcmp(str1, "POSITIVE")==0)
	  Params->PulsePolarity[ch]=1;
	else if (strcmp(str1, "NEGATIVE")==0)
	  Params->PulsePolarity[ch]=0;
	if(Params->PulsePolarity[ch]==1 && verbose==1) printf("Ch%i: Pulse Polarity Positive \n",ch);
	else if(verbose)  printf("Ch%i: Pulse Polarity Negative \n",ch);
	continue;
      }
    else if (strstr(str, "TRIGGERTHRESHOLD")!=NULL) 
      {
	read = fscanf(file, "%i",&DPPParams->thr[ch]);
	 if(verbose) printf("Ch%i: Trigger Threshold %i ADC counts \n",ch,DPPParams->thr[ch]);
	continue;
      }
    else if (strstr(str, "TRAPEZOIDRISETIME")!=NULL) 
      {
	read = fscanf(file, "%i",&DPPParams->k[ch]);
	 if(verbose) printf("Ch%i: Trapezoid Rise Time %i ns \n",ch,DPPParams->k[ch]);
	continue;
      }
   else if (strstr(str, "TRAPEZOIDFLATTOP")!=NULL) 
      {
	read = fscanf(file, "%i",&DPPParams->m[ch]);
	 if(verbose) printf("Ch%i: Trapezoid Flat Top %i ns \n",ch,DPPParams->m[ch]);
	continue;
      }
   else if (strstr(str, "DECAYTIME")!=NULL) 
      {
	read = fscanf(file, "%i",&DPPParams->M[ch]);
	 if(verbose) printf("Ch%i: Decay Time constant %i ns \n",ch,DPPParams->M[ch]);
	continue;
      }
   else if (strstr(str, "PEAKINGTIME")!=NULL) 
      {
	read = fscanf(file, "%i",&DPPParams->ftd[ch]);
	 if(verbose) printf("Ch%i: Peaking Time %i ns \n",ch,DPPParams->ftd[ch]);
	continue;
     }
   else if (strstr(str, "TRIGGERSMOOTHING")!=NULL) 
      {
	read = fscanf(file, "%i",&DPPParams->a[ch]);
	if(verbose) printf("Ch%i: Trigger Smoothing %i \n",ch,DPPParams->a[ch]);
	continue;
      }
   else if (strstr(str, "INPUTRISETIME")!=NULL) 
      {
	read = fscanf(file, "%i",&DPPParams->b[ch]);
	if(verbose) printf("Ch%i: Input Rise Time %i ns \n",ch,DPPParams->b[ch]);
	continue;
      }
   else if (strstr(str, "TRIGGERHOLDOFF")!=NULL) 
      {
	read = fscanf(file, "%i",&DPPParams->trgho[ch]);
	if(verbose) printf("Ch%i: Trigger Hold Off %i \n",ch,DPPParams->trgho[ch]);
	continue;
      }
   else if (strstr(str, "PEAKHOLDOFF")!=NULL) 
      {
	read = fscanf(file, "%i",&DPPParams->pkho[ch]);
	if(verbose) printf("Ch%i: Peak Hold Off %i \n",ch,DPPParams->pkho[ch]);
	continue;
      }
   else if (strstr(str, "BASELINEHOLDOFF")!=NULL) 
      {
	read = fscanf(file, "%i",&DPPParams->blho[ch]);
	if(verbose) printf("Ch%i: Baseline Hold Off %i \n",ch,DPPParams->blho[ch]);
	continue;
      }
   else if (strstr(str, "INPUTRANGE")!=NULL) 
      {
	read = fscanf(file, "%i", &Params->InputRange[ch]);
	if(verbose) {
	  printf("Ch%i: ",ch);
	  if(Params->InputRange[ch] == 0) printf("Range ADC 0.6V \n");
	  else if(Params->InputRange[ch] == 1) printf("Range ADC 1.4V \n");
	  else if(Params->InputRange[ch] == 2) printf("Range ADC 3.7V \n");
	  else if(Params->InputRange[ch] == 3) printf("Range ADC 9V \n");
	}
	continue;	
      }  

   else if (strstr(str, "DCOFFSET")!=NULL) 
      {
	read = fscanf(file, "%i", &Params->DCOffset[ch]);
	if(verbose) printf("Ch%i: DC OffSet %i \n", ch,Params->DCOffset[ch]);
	continue;
      }
   else if (strstr(str, "TRIGGERMODE")!=NULL) 
      {
	int tmp;
	read = fscanf(file, "%s", str1);
	if (strcmp(str1, "COINCIDENCE")==0)
	  Params->TrgMode[ch] = COINCIDENCE;
	else if (strcmp(str1, "ANTICOINCIDENCE")==0)
	  Params->TrgMode[ch] = ANTICOINCIDENCE;
	else Params->TrgMode[ch]= INDIVIDUAL;
	if(verbose) printf("Ch%i: Trigger Mode %i \n", ch,Params->TrgMode[ch]);
	continue;
      }
   else if (strstr(str, "SELFTRIGGER")!=NULL) 
      {
	read = fscanf(file, "%s", str1 );
	if (strcmp(str1, "ENABLE")==0)
	  Params->SelfTrigger[ch] = 0x0000000;
	else if (strcmp(str1, "DISABLE")==0)
	  Params->SelfTrigger[ch] = 0x1000000;
	if(verbose) printf("Ch%i: Self Trigger  %s \n",ch, str1 );
	continue;

      }
  }

  fclose(file);

}


/*************************************************************************************************************/
int  ProgramDigitizerDT5780(int connectionParams[4], char* filename) {
/*************************************************************************************************************/
  CAEN_DGTZ_DPP_PHA_Params_t DPPParams;
  DT5780Params_t Params;

  CAEN_DGTZ_BoardInfo_t           BoardInfo;
  int MajorNumber;
  int ret=0, handle;
  uint32_t InputRangeRegister[2]={0x10B4,0x11B4};
  uint32_t InputRangeValues[4]={0x5,0x6,0x9,0xA};
  uint32_t TriggerModeRegister[2]={0x1080,0x1180};
  uint32_t TriggerModeMask= 0xC0000;
  uint32_t TriggerModeValues[3]={0x00000,0x80000,0xC0000};

  memset(&Params, 0, sizeof(DT5780Params_t));
  memset(&DPPParams, 0, sizeof(CAEN_DGTZ_DPP_PHA_Params_t));
    
  ParseConfigFileDT5780(filename,&Params,&DPPParams);

  for(int ch=0; ch<MaxDT5780NChannels; ch++) {
 
    // Number of Samples for Baseline Mean (allowed values are from 0 to 6, where 
    // 0=0, 1=16, 2=64, 3=256, 4=1024, 5=4096, 6=16384)
    DPPParams.nsbl[ch] = 6; 

    // Number of Samples for Peak Mean (allowed values are from 0 to 3, 
    // where 0=1, 1=4, 2=16, 3=64)
    DPPParams.nspk[ch] = 2; // 3 = 64 samples

    DPPParams.enf[ch] = 1.0; // Energy Normalization Factor
    DPPParams.decimation[ch] = 0;
    DPPParams.dgain[ch] = 0x0;
    DPPParams.otrej[ch] = 0;
    DPPParams.trgwin[ch] = 0;
    DPPParams.twwdt[ch] = 0;
  }
 
  /* *************************************************************************************** */
  /* Open the digitizer and read board information                                           */
  /* *************************************************************************************** */
  /* The following function is used to open the digitizer with the given connection parameters
    and get the handler to it */

  ret = CAEN_DGTZ_OpenDigitizer(Params.LinkType,Params.LinkNum , 0, 0, &handle);

  if (ret) {
    sleep(1);
    ret = CAEN_DGTZ_OpenDigitizer(Params.LinkType,Params.LinkNum , 0, 0, &handle); //retry opening
    if(ret) {
      printf("Can't open digitizer DT5780\n");
      return ret;
    }
  }
        

  /* Once we have the handler to the digitizer, we use it to call the other functions */
  ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
  if (ret) {
    printf("Can't read board info\n");
    return ret;
  }
  printf("\nConnected to CAEN Digitizer Model %s, recognized as board %d\n", BoardInfo.ModelName, 0);
  printf("ROC FPGA Release is %s\n", BoardInfo.ROC_FirmwareRel);
  printf("AMC FPGA Release is %s\n", BoardInfo.AMC_FirmwareRel);

  /* Check firmware revision (only DPP-PHA firmwares can be used) */
  sscanf(BoardInfo.AMC_FirmwareRel, "%d", &MajorNumber);
  if(MajorNumber != V1724_DPP_PHA_CODE && MajorNumber != V1730_DPP_PHA_CODE) {
    printf("This digitizer has not a DPP-PHA firmware\n");
    return -1;
  }
    
  /* This function uses the CAENDigitizer API functions to perform the digitizer's initial configuration */
  int i;
  uint32_t val;

  /* Reset the digitizer */
  ret |= CAEN_DGTZ_Reset(handle);
  if (ret) {
      printf("ERROR: can't reset the digitizer.\n");
      return -1;
  }

  ret |= CAEN_DGTZ_WriteRegister(handle, 0x8000, 0x01040114);  // This register contains general settings for the board configuration.
  ret |= CAEN_DGTZ_WriteRegister(handle, 0x811C, 0xC003C);  // This register manages the front panel I/O connectors
  // ret |= CAEN_DGTZ_WriteRegister(handle, 0x811C, 0x801);  // This register manages the front panel I/O connectors

  /* Set the DPP acquisition mode
    This setting affects the modes Mixed and List (see CAEN_DGTZ_DPP_AcqMode_t definition for details)
    CAEN_DGTZ_DPP_SAVE_PARAM_EnergyOnly      Only energy (DPP-PHA) or charge (DPP-PSD/DPP-CI v2) is returned
    CAEN_DGTZ_DPP_SAVE_PARAM_TimeOnly        Only time is returned
    CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime   Both energy/charge and time are returned
    CAEN_DGTZ_DPP_SAVE_PARAM_None            No histogram data is returned 
  */

  ret |= CAEN_DGTZ_SetDPPAcquisitionMode(handle, Params.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    
  // Set the digitizer acquisition mode (CAEN_DGTZ_SW_CONTROLLED or CAEN_DGTZ_S_IN_CONTROLLED)
  ret |= CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
    
  // Set the number of samples for each waveform
  ret |= CAEN_DGTZ_SetRecordLength(handle, Params.RecordLength);
  int rl;
  CAEN_DGTZ_GetRecordLength(handle,&rl);
  // Set the I/O level (CAEN_DGTZ_IOLevel_NIM or CAEN_DGTZ_IOLevel_TTL)
  ret |= CAEN_DGTZ_SetIOLevel(handle, Params.IOLevel);

  CAEN_DGTZ_WriteRegister(handle, 0xEF08, 0x1C);  // Board id set to 1C (=11100)

  /* Set the digitizer's behaviour when an external trigger arrives:
    CAEN_DGTZ_TRGMODE_DISABLED: do nothing
    CAEN_DGTZ_TRGMODE_EXTOUT_ONLY: generate the Trigger Output signal
    CAEN_DGTZ_TRGMODE_ACQ_ONLY = generate acquisition trigger
    CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT = generate both Trigger Output and acquisition trigger
  */
  ret |= CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT);

  // This function enables/disables the channels for the acquisition. Disabled channels don’t give any trigger and don’t participate to the event data.
  ret |= CAEN_DGTZ_SetChannelEnableMask(handle, Params.ChannelMask);

  // Set how many events to accumulate in the board memory before being available for readout
  ret |= CAEN_DGTZ_SetNumEventsPerAggregate(handle, 1, 0);
  ret |= CAEN_DGTZ_SetDPPEventAggregation(handle, 1, 0);
  ret |= CAEN_DGTZ_SetMaxNumAggregatesBLT(handle,1); 

  /* Set the mode used to syncronize the acquisition between different boards.
     In this example the sync is disabled 
  */
  ret |= CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled);

  ret |= CAEN_DGTZ_SetInterruptConfig( handle, CAEN_DGTZ_ENABLE, 1,0xAAAA ,1, CAEN_DGTZ_IRQ_MODE_ROAK);

  // Set the DPP specific parameters for the channels in the given channelMask
  ret |= CAEN_DGTZ_SetDPPParameters(handle, Params.ChannelMask, &DPPParams);

  for(i=0; i<MaxDT5780NChannels; i++) {
    if(Params.ChannelMask & (1<<i)) {
                        
      // Set the Pre-Trigger size (in samples)
      ret |= CAEN_DGTZ_SetDPPPreTriggerSize(handle, i, Params.PreTriggerSize);

      // Set the polarity for the given channel (CAEN_DGTZ_PulsePolarityPositive or CAEN_DGTZ_PulsePolarityNegative)
      if(Params.PulsePolarity[i]==1) 
         ret |= CAEN_DGTZ_SetChannelPulsePolarity(handle, i, CAEN_DGTZ_PulsePolarityPositive);
      else
         ret |= CAEN_DGTZ_SetChannelPulsePolarity(handle, i, CAEN_DGTZ_PulsePolarityNegative);
	   
      // Set a DC offset to the input signal to adapt it to digitizer's dynamic range 
      ret |= CAEN_DGTZ_SetChannelDCOffset(handle, i, Params.DCOffset[i]);
 
      //This register modifies the analog gains of channel n and therefore its input dynamics
      ret |= CAEN_DGTZ_WriteRegister(handle, InputRangeRegister[i], InputRangeValues[Params.InputRange[i]]);

      // printf("TriggerModeRegister %x,TriggerModeValues %x,TriggerModeMask %x \n",TriggerModeRegister[i],TriggerModeValues[TrgMode[i]],TriggerModeMask);
      ret |= WriteRegisterBitmask(handle, TriggerModeRegister[i],TriggerModeValues[Params.TrgMode[i]],TriggerModeMask);
      ret |= WriteRegisterBitmask(handle, TriggerModeRegister[i],Params.SelfTrigger[i],0x1000000);
    }
  }

  /* Set the virtual probes settings
    DPP-PHA can save:
    2 analog waveforms:
        the first and the second can be specified with the VIRTUALPROBE 1 and 2 parameters
        
    4 digital waveforms:
        the first is always the trigger
        the second is always the gate
        the third and fourth can be specified with the DIGITALPROBE 1 and 2 parameters

    CAEN_DGTZ_DPP_VIRTUALPROBE_SINGLE -> Save only the Input Signal waveform
    CAEN_DGTZ_DPP_VIRTUALPROBE_DUAL      -> Save also the waveform specified in VIRTUALPROBE

    Virtual Probes 1 types:
    CAEN_DGTZ_DPP_PHA_VIRTUALPROBE1_Trapezoid
    CAEN_DGTZ_DPP_PHA_VIRTUALPROBE1_Delta
    CAEN_DGTZ_DPP_PHA_VIRTUALPROBE1_Delta2
    CAEN_DGTZ_DPP_PHA_VIRTUALPROBE1_Input
    
    Virtual Probes 2 types:
    CAEN_DGTZ_DPP_PHA_VIRTUALPROBE2_Input
    CAEN_DGTZ_DPP_PHA_VIRTUALPROBE2_S3
    CAEN_DGTZ_DPP_PHA_VIRTUALPROBE2_DigitalCombo
    CAEN_DGTZ_DPP_PHA_VIRTUALPROBE2_trapBaseline
    CAEN_DGTZ_DPP_PHA_VIRTUALPROBE2_None

    Digital Probes types:
    CAEN_DGTZ_DPP_PHA_DIGITAL_PROBE_trgKln
    CAEN_DGTZ_DPP_PHA_DIGITAL_PROBE_Armed
    CAEN_DGTZ_DPP_PHA_DIGITAL_PROBE_PkRun
    CAEN_DGTZ_DPP_PHA_DIGITAL_PROBE_PkAbort
    CAEN_DGTZ_DPP_PHA_DIGITAL_PROBE_Peaking
    CAEN_DGTZ_DPP_PHA_DIGITAL_PROBE_PkHoldOff
    CAEN_DGTZ_DPP_PHA_DIGITAL_PROBE_Flat
    CAEN_DGTZ_DPP_PHA_DIGITAL_PROBE_TRGHoldoff 
  */
  //ret |= CAEN_DGTZ_SetDPP_PHA_VirtualProbe(handle, CAEN_DGTZ_DPP_VIRTUALPROBE_DUAL, CAEN_DGTZ_DPP_PHA_VIRTUALPROBE1_Delta2, CAEN_DGTZ_DPP_PHA_VIRTUALPROBE2_Input, CAEN_DGTZ_DPP_PHA_DIGITAL_PROBE_TRGHoldoff);

  ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_1, CAEN_DGTZ_DPP_VIRTUALPROBE_Trapezoid);
  ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, ANALOG_TRACE_2, CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
  ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, DIGITAL_TRACE_1,CAEN_DGTZ_DPP_DIGITALPROBE_Peaking );
  ret |= CAEN_DGTZ_SetDPP_VirtualProbe(handle, DIGITAL_TRACE_2,CAEN_DGTZ_DPP_DIGITALPROBE_Trigger );

  // Enable propagation of individual trigger   
  ret |= WriteRegisterBitmask(handle, 0x8000, 0x4, 0x4);

  // Enable trigger out AND o OR between channels
  ret |= CAEN_DGTZ_WriteRegister(handle, 0x8110,Params.TriggerOut); 
  ret |= WriteRegisterBitmask(handle, 0x817C,Params.ExtTrigger,0x1);
  //    ret |= WriteRegisterBitmask(handle, 0x817C,0x0,0x1); // enable ext trigger ATTENZIONE: 0->enable 1->disable
  //    ret |= WriteRegisterBitmask(handle, 0x1080,0x1000000,0x1000000); // disable(=1)/enable(=0) acquisition on self trigger
  //    ret |= WriteRegisterBitmask(handle, 0x1080,0x0000000,0x1000000); // disable(=1)/enable(=0) acquisition on self trigger

  // disable(=0)/enable(=1) energy evaluation also for piled-up events
  ret |= WriteRegisterBitmask(handle, 0x1080,0x1000000,0x8000000);


  ret |= WriteRegisterBitmask(handle, 0x1184, 0xFF, 0xFF);// gate (x10ns)
  ret |= WriteRegisterBitmask(handle, 0x1084, 0xFF, 0xFF);// gate (x10ns)
  ret |= WriteRegisterBitmask(handle, 0x8188, 0x4000010C, 0xFFFFFFFF);// 
  ret |= WriteRegisterBitmask(handle, 0x818C, 0xC000010C, 0xFFFFFFFF);// 
  //    ret |= WriteRegisterBitmask(handle, 0x818C, 0x10C, 0xFFFFFFFF);
  //    ret |= WriteRegisterBitmask(handle, 0x818C, 0x10C, 0xFFFFFFFF); // se non setto 818C allora salvo solo il canale 0 (ma solo quando ho la coincidenza tra i due)
 
  ret |= CAEN_DGTZ_CloseDigitizer(handle);

  if(ret) {
    printf("Warning: errors found during the programming of the digitizer.\nSome settings may not be executed\n");
    return ret;
  } 
  else {
    connectionParams[0]=Params.LinkType;
    connectionParams[1]=Params.LinkNum;
    connectionParams[2]=0;
    connectionParams[3]=0;
    return 0;
  }
    
}

