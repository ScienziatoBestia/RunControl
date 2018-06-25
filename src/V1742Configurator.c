#include "../include/V1742Configurator.h"
#include "../include/UtilsFunctions.h"


/*****************************************************************************************/
void WriteConfigFileV1742(char* filename, V1742Params_t *Params) {
/*****************************************************************************************/
  Params->RecordLength=1024;
  Params->NumEvents=1;

  FILE* file = fopen(filename,"w");
  fprintf(file,"[COMMON] \n\n");
  fprintf(file,"OPEN PCI %i 0 0 \n\n", Params->LinkNum);
  fprintf(file,"DRS4_FREQUENCY %i \n\n", Params->DRS4Frequency);
  fprintf(file,"RECORD_LENGTH %i \n\n",Params->RecordLength);
  //  fprintf(file,"EXTERNAL_TRIGGER %i \n\n",Params->ExtTriggerMode);
  fprintf(file,"EXTERNAL_TRIGGER DISABLED \n\n");
  //  fprintf(file,"FAST_TRIGGER %i \n\n",Params->FastTriggerMode);
  fprintf(file,"FAST_TRIGGER ACQUISITION_ONLY \n\n");
  if(Params->FastTriggerEnabled==1) 
    fprintf(file,"ENABLED_FAST_TRIGGER_DIGITIZING YES \n\n");
  else fprintf(file,"ENABLED_FAST_TRIGGER_DIGITIZING NO \n\n");
  fprintf(file,"MAX_NUM_EVENTS_BLT %i \n\n",Params->NumEvents);
  fprintf(file,"POST_TRIGGER %i \n\n",Params->PostTrigger);
  fprintf(file,"TRIGGER_EDGE ");
  if(Params->TriggerEdge==1) 
    fprintf(file,"FALLING \n\n");
  else fprintf(file,"RISING \n\n");
  //  fprintf(file,"FPIO_LEVEL %i \n\n",Params->FPIOtype);
  fprintf(file,"FPIO_LEVEL NIM \n\n");
  fprintf(file,"GROUP_ENABLE_MASK %X \n\n",Params->GroupEnableMask);
  int nc;
  for(nc=0;nc<MaxV1742NChannels;nc++)
    fprintf(file,"CH %i DC_OFFSET %i \n\n",nc,Params->DCOffset[nc]);
  int ng;
  for(ng=0;ng<MaxV1742NGroups/2;ng++) {
    fprintf(file,"[TR%i] \n\n",ng);
    fprintf(file,"TRIGGER_OFFSET %i \n\n",Params->FastTriggerDCOffset[ng*2]);
    fprintf(file,"TRIGGER_THRESHOLD %i \n\n",Params->FastTriggerThreshold[ng*2]);
  }
  fclose(file);
}



/*****************************************************************************************/
int ParseConfigFileV1742(char* filename, V1742Params_t *V1742Config) {
/*****************************************************************************************/
 char str[1000], str1[1000], *pread;
  int i, group=-1, val, Off=0, tr = -1;
  int ret = 0;

  FILE* f_ini = fopen(filename,"r");

  /* read config file and assign parameters */
  while(!feof(f_ini)) {
    int read;
    char *res;
    // read a word from the file
    read = fscanf(f_ini, "%s", str);
    if( !read || (read == EOF) || !strlen(str))
      continue;
    // skip comments
    if(str[0] == '#') {
      res = fgets(str, 1000, f_ini);
      continue;
    }

    if (strcmp(str, "@ON")==0) {
      Off = 0;
      continue;
    }
    if (strcmp(str, "@OFF")==0)
      Off = 1;
    if (Off)
      continue;

    
    // Section (COMMON or individual group)
    if (str[0] == '[') {
      if (strstr(str, "COMMON")) {
	group = -1;
	continue; 
      }
      if (strstr(str, "TR")) {
	group = -1;
	sscanf(str+1, "TR%d", &val);
	if (val != 0 && val != 1) {
	  printf("%s: Invalid channel number\n", str);
	} else {
	  tr = val;
	}
      } else {
	sscanf(str+1, "%d", &val);
	if (val < 0 || val >= MaxV1742NGroups) {
	  printf("%s: Invalid group number\n", str);
	} else {
	  group = val;
	}
      }
      continue;
    }

    // OPEN: read the details of physical path to the digitizer
    if (strstr(str, "OPEN")!=NULL) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "USB")==0)
	V1742Config->LinkType = CAEN_DGTZ_USB;
      else if (strcmp(str1, "PCI")==0)
	V1742Config->LinkType = CAEN_DGTZ_OpticalLink;
      else {
	printf("%s %s: Invalid connection type\n", str, str1);
	return -1; 
      }
      read = fscanf(f_ini, "%d", &V1742Config->LinkNum);
      if (V1742Config->LinkType == CAEN_DGTZ_USB)
	V1742Config->ConetNode = 0;
      else
	read = fscanf(f_ini, "%d", &V1742Config->ConetNode);
      read = fscanf(f_ini, "%x", &V1742Config->BaseAddress);
      continue;
    }

    // Acquisition Record Length (number of samples)
    if (strstr(str, "RECORD_LENGTH")!=NULL) {
      read = fscanf(f_ini, "%d", &V1742Config->RecordLength);
      continue;
    }

    // Acquisition Record Length (number of samples)
    if (strstr(str, "DRS4_FREQUENCY")!=NULL) {
      int freq;
      read = fscanf(f_ini, "%d", &freq);
      V1742Config->DRS4Frequency = (CAEN_DGTZ_DRS4Frequency_t)freq;
      continue;
    }

    // Trigger Edge
    if (strstr(str, "TRIGGER_EDGE")!=NULL) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "FALLING")==0)
	V1742Config->TriggerEdge = 1;
      else if (strcmp(str1, "RISING")!=0)
	printf("%s: invalid option\n", str);
      continue;
    }

        // External Trigger (DISABLED, ACQUISITION_ONLY, ACQUISITION_AND_TRGOUT)
    if (strstr(str, "EXTERNAL_TRIGGER")!=NULL) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "DISABLED")==0)
	V1742Config->ExtTriggerMode = CAEN_DGTZ_TRGMODE_DISABLED;
      else if (strcmp(str1, "ACQUISITION_ONLY")==0)
	V1742Config->ExtTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
      else if (strcmp(str1, "ACQUISITION_AND_TRGOUT")==0)
	V1742Config->ExtTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
      else
	printf("%s: Invalid Parameter\n", str);
      continue;
    }

    // Max. number of events for a block transfer (0 to 1023)
    if (strstr(str, "MAX_NUM_EVENTS_BLT")!=NULL) {
      read = fscanf(f_ini, "%d", &V1742Config->NumEvents);
      continue;
    }
    

    // Post Trigger (percent of the acquisition window)
    if (strstr(str, "POST_TRIGGER")!=NULL) {
      read = fscanf(f_ini, "%d", &V1742Config->PostTrigger);
      continue;
    }

    if (!strcmp(str, "FAST_TRIGGER")) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "DISABLED")==0)
	V1742Config->FastTriggerMode = CAEN_DGTZ_TRGMODE_DISABLED;
      else if (strcmp(str1, "ACQUISITION_ONLY")==0)
	V1742Config->FastTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
      else
	printf("%s: Invalid Parameter\n", str);
      continue;
    }
		
    if (strstr(str, "ENABLED_FAST_TRIGGER_DIGITIZING")!=NULL) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "YES")==0)
	V1742Config->FastTriggerEnabled= 1;
      else if (strcmp(str1, "NO")!=0)
	printf("%s: invalid option\n", str);
      continue;
    }

    // DC offset (percent of the dynamic range, -50 to 50)
    if (!strcmp(str, "CH")) {
      int ich;
      read = fscanf(f_ini, "%i", &ich);
      read = fscanf(f_ini, "%s", str1);
      if (!strcmp(str1, "DC_OFFSET")) {
	float dc;
	read = fscanf(f_ini, "%f", &dc);
	V1742Config->DCOffset[ich] = dc;
	//	val = (int)((dc+50) * 65535 / 100);      
	//	V1742Config->DCOffset[ich] = val;
	continue;
      }
    }
    // Fast Trigger DC Offset
    if (strstr(str, "TRIGGER_OFFSET")!=NULL) {
      float dc;
      read = fscanf(f_ini, "%f", &dc);
      if (tr != -1) {
	V1742Config->FastTriggerDCOffset[tr*2] = (uint32_t)dc;
	V1742Config->FastTriggerDCOffset[tr*2+1] = (uint32_t)dc;	
	continue;
      }
    }

    // Fast Trigger Threshold
    if (strstr(str, "TRIGGER_THRESHOLD")!=NULL) {
      read = fscanf(f_ini, "%d", &val);
      if (tr != -1) {
	V1742Config->FastTriggerThreshold[tr*2] = val;
	V1742Config->FastTriggerThreshold[tr*2+1] = val;
	continue;
      }
    }
    
    // Front Panel LEMO I/O level (NIM, TTL)
    if (strstr(str, "FPIO_LEVEL")!=NULL) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "TTL")==0)
	V1742Config->FPIOtype = 1;
      else if (strcmp(str1, "NIM")!=0)
	printf("%s: invalid option\n", str);
      continue;
    }
    // Group enable Mask
    if (strstr(str, "GROUP_ENABLE_MASK")!=NULL) {
      read = fscanf(f_ini, "%x", &val);
      V1742Config->GroupEnableMask = val;
      continue;
    }
    printf("%s: invalid setting\n", str);
  }
  fclose(f_ini);
  return ret;

}





/*****************************************************************************************/
int ProgramDigitizerV1742(int connectionParams[4], char* filename, int isCalo) {
/*****************************************************************************************/
  int i, j, val, ret=0;
  static int handle;
  V1742Params_t V1742Config;
  int isVMEDevice= 0;
  CAEN_DGTZ_BoardInfo_t BoardInfo;

  // Open and parse configuration file for V1742
  ParseConfigFileV1742(filename, &V1742Config);

  // Open the digitizer and read the board information
  isVMEDevice = V1742Config.BaseAddress ? 1 : 0; 
  ret = CAEN_DGTZ_OpenDigitizer(V1742Config.LinkType, V1742Config.LinkNum, V1742Config.ConetNode, V1742Config.BaseAddress, &handle);
  printf("LinkType %i LinkNum %i ConetNode %i BaseAddress %i  handle %i\n", V1742Config.LinkType, V1742Config.LinkNum, V1742Config.ConetNode, V1742Config.BaseAddress,handle);
  if(ret!=0 && ret !=-25) {
    printf("***V1742Configurator: error opening digitizer V1742\n");
    return ret;
  }
  if(ret==-25) printf("***V1742Configurator: Digitizer V1742 already open \n");
 
  ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
  if (ret) {
    printf("***V1742Configurator: invalid board type \n");
  }
  else{
    printf("Connected to CAEN Digitizer Model %s\n", BoardInfo.ModelName);
    printf("ROC FPGA Release is %s\n", BoardInfo.ROC_FirmwareRel);
    printf("AMC FPGA Release is %s\n", BoardInfo.AMC_FirmwareRel);
  }


  // Program the digitizer
  //    V1742Config.GroupEnableMask &= (1<<(V1742Config.Nch/8))-1;
    
  // Reset the digitizer 
  ret |= CAEN_DGTZ_Reset(handle);
  if (ret != 0) printf("***V1742Configurator: Unable to reset digitizer.\n");
    
  ret |= CAEN_DGTZ_SetFastTriggerDigitizing(handle,V1742Config.FastTriggerEnabled);
  ret |= CAEN_DGTZ_SetFastTriggerMode(handle,V1742Config.FastTriggerMode);  
  ret |= CAEN_DGTZ_SetRecordLength(handle, V1742Config.RecordLength);
  // ret |= CAEN_DGTZ_GetRecordLength(handle, &V1742Config.RecordLength);
  ret |= CAEN_DGTZ_SetPostTriggerSize(handle, V1742Config.PostTrigger);
  // ret |= CAEN_DGTZ_WriteRegister(handle, 0x1114, 0xF);
  ret |= CAEN_DGTZ_SetIOLevel(handle, V1742Config.FPIOtype);
  CAEN_DGTZ_SetInterruptConfig(handle, CAEN_DGTZ_ENABLE,1,0xAAAA ,1, CAEN_DGTZ_IRQ_MODE_ROAK);

/*
    if( V1742Config.InterruptNumEvents > 0) {
      // Interrupt handling
      
           if( ret |= CAEN_DGTZ_SetInterruptConfig( handle, CAEN_DGTZ_ENABLE, VME_INTERRUPT_LEVEL, VME_INTERRUPT_STATUS_ID, (uint16_t)V1742Config.InterruptNumEvents, INTERRUPT_MODE)!= CAEN_DGTZ_Success) {
      	printf( "\nError configuring interrupts. Interrupts disabled\n\n");
      	V1742Config.InterruptNumEvents = 0;
            }
    }
*/

  ret |= CAEN_DGTZ_SetMaxNumEventsBLT(handle, V1742Config.NumEvents);
  ret |= CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
  ret |= CAEN_DGTZ_SetExtTriggerInputMode(handle, V1742Config.ExtTriggerMode);

  ret |= CAEN_DGTZ_SetGroupEnableMask(handle, V1742Config.GroupEnableMask);
  int index;
  for(i=0; i<MaxV1742NGroups; i++) {
    if(V1742Config.GroupEnableMask & (1<<i)) {
      for(j=0; j<MaxV1742NChannels/MaxV1742NGroups; j++) {
	index=(i*MaxV1742NChannels/MaxV1742NGroups)+j;
        val = (int)((V1742Config.DCOffset[index]+50) * 65535 / 100);
	ret |= CAEN_DGTZ_SetChannelDCOffset(handle,index, val);
      }
    }
  }

  ret |= CAEN_DGTZ_SetDRS4SamplingFrequency(handle, V1742Config.DRS4Frequency);

  // Fast Trigger Settings:
  if(V1742Config.FastTriggerEnabled) {
    for(i=0; i<MaxV1742NGroups; i++) {
      ret |= CAEN_DGTZ_SetGroupFastTriggerDCOffset(handle,i,V1742Config.FastTriggerDCOffset[i]);
      ret |= CAEN_DGTZ_SetGroupFastTriggerThreshold(handle,i,V1742Config.FastTriggerThreshold[i]);
      ret |= CAEN_DGTZ_SetTriggerPolarity(handle, i, V1742Config.TriggerEdge);
    }
  }

  if(isCalo) ret |= CAEN_DGTZ_WriteRegister(handle, 0xEF08, 0x1E);  //setting Board id = 11111
  else ret |= CAEN_DGTZ_WriteRegister(handle, 0xEF08, 0x1F);  //setting Board id = 11111

  ret |= CAEN_DGTZ_CloseDigitizer(handle);
  if(ret) {
    printf("Warning: errors found during the programming of the digitizer.\nSome settings may not be executed\n");
    return ret;
  } 
  else {
    connectionParams[0]=V1742Config.LinkType;
    connectionParams[1]=V1742Config.LinkNum;
    connectionParams[2]=V1742Config.ConetNode;
    connectionParams[3]=V1742Config.BaseAddress;
    return 0;
  }
  
}
