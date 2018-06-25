
#include "../include/V812Configurator.h"
#include <math.h>

#define verbose 0

void WriteConfigFileV812(char* filename, V812Params_t *Params) {
  FILE* file = fopen(filename,"w");
  int ch,gr;
  fprintf(file,"LINKNUM %i \n\n",Params->LinkNum);
  fprintf(file,"BASEADDRESS 0x%X \n\n",Params->BaseAddress);

  for(ch=0;ch<MaxV812NChannels;ch++) {
    fprintf(file,"THRESHOLD CH%i %X \n",ch,Params->Threshold[ch]);
  }
  fprintf(file,"\n");
  for(gr=0;gr<MaxV812NGroups;gr++) {
    fprintf(file,"OUTPUTWIDTH GR%i %X \n",gr,Params->OutputWidth[gr]);
  }
  fprintf(file,"\n");
  for(gr=0;gr<MaxV812NGroups;gr++) {
    fprintf(file,"DEADTIME GR%i %X \n",gr,Params->DeadTime[gr]);
  }
  fprintf(file,"\n");
  fprintf(file,"MAJORITY %i \n\n",Params->Majority);
  fprintf(file,"INHIBITPATTERN %X \n",Params->InhibitPattern);
  fclose(file);
}

void ParseConfigFileV812(char* filename, V812Params_t *Params) {
  FILE* file = fopen(filename,"r");

  char str[1000], str1[1000];
  int read, value, ret,ch,gr;
  while(!feof(file)) {
    
    read = fscanf(file, "%s", str);
    if( !read || (read == EOF) || !strlen(str)) continue;
    if(str[0] == '#') {fgets(str, 1000, file); continue;}    // skip comments
    if (strstr(str, "LINKNUM")!=NULL) {
      read = fscanf(file, "%i", &Params->LinkNum);
      if(verbose) printf("ParseConfigFileV812: LinkNumber: %i \n",Params->LinkNum);
      continue;
    } else if (strstr(str, "BASEADDRESS")!=NULL) {
      read = fscanf(file, "%X", &Params->BaseAddress);
      if(verbose) printf("ParseConfigFileV812: BaseAddress: %X \n",Params->BaseAddress);
      continue;

    } else if (strstr(str, "THRESHOLD")!=NULL) {
      read = fscanf(file, "%s", str);
      if (strstr(str, "CH")) {
	sscanf(str, "CH%i", &value);
	ch=value;
	read = fscanf(file, "%X", &Params->Threshold[ch]);
	 if(verbose) printf("ParseConfigFileV812: Ch%i Threshold: %X \n",ch,Params->Threshold[ch]);
      } else {
	printf("%s: invalid setting for Threshold\n", str);
      }
      continue;
    } else if (strstr(str, "OUTPUTWIDTH")!=NULL) {
      read = fscanf(file, "%s", str);
      if (strstr(str, "GR")) {
	sscanf(str, "GR%d", &value);
	gr=value;
	read = fscanf(file, "%X", &Params->OutputWidth[gr]);
	 if(verbose) printf("ParseConfigFileV812: Gr%i OutputWidth: %X \n",gr,Params->OutputWidth[gr]);
      } else {
	printf("%s: invalid setting for OutputWidth\n", str);
      }
      continue;
    } else if (strstr(str, "DEADTIME")!=NULL) {
      read = fscanf(file, "%s", str);
      if (strstr(str, "GR")) {
	sscanf(str, "GR%d", &value);
	gr=value;
	read = fscanf(file, "%X", &Params->DeadTime[gr]);
	 if(verbose) printf("ParseConfigFileV812: Gr%i DeadTime: %X \n",gr,Params->DeadTime[gr]);
      } else {
	printf("%s: invalid setting for DeadTime\n", str);
      }
      continue;
    } else if (strstr(str, "MAJORITY")!=NULL) {
      read = fscanf(file, "%i", &Params->Majority);
       if(verbose) printf("ParseConfigFileV812: Majority: %i \n",Params->Majority);
      continue;
    } else if (strstr(str, "INHIBITPATTERN")!=NULL) {
      read = fscanf(file, "%X", &Params->InhibitPattern);
       if(verbose) printf("ParseConfigFileV812: Inhibit Pattern: %X \n",Params->InhibitPattern);
      continue;
    } else  printf("%s: invalid setting\n", str);
  }
  fclose(file);
}

int ProgramV812(char* filename) {

  CVBoardTypes VMEBoard = cvV2718;
  int32_t       V812Handle;

  CVAddressModifier AM = cvA32_U_DATA ;
  CVDataWidth DW=cvD16 ;
  int err_code;
  int ch, gr;
  V812Params_t Params;

  memset(&Params, 0, sizeof(V812Params_t));
  ParseConfigFileV812(filename, &Params);
  /* open connection */

  err_code = CAENVME_Init(VMEBoard, Params.LinkNum, 0, &V812Handle);

  if(err_code==0) printf("\nConnected to CAEN Discriminator V812 \n");
  else printf("Error Opening connection to CAEN Discriminator V812: %s  \n",CAENVME_DecodeError(err_code));
 
  if(err_code !=0) return err_code;

  uint32_t baseAddr = Params.BaseAddress<<16;

  /* Read Serial Number */
  uint32_t	data ; 

  err_code =   CAENVME_ReadCycle(V812Handle, baseAddr| REG_V812_VERSION, &data,AM, DW);
 
  if(err_code == cvSuccess)  printf("Serial Number: %i  \n",data & 0xffff);
  else  printf("Error Reading Serial Number: %s \n", CAENVME_DecodeError(err_code));

  for(ch=0;ch<MaxV812NChannels;ch++) {
    err_code |=   CAENVME_WriteCycle(V812Handle, (baseAddr| REG_V812_THRESHOLD_CH0)+ch*V812_THRESHOLD_STEP, &Params.Threshold[ch],AM, DW);
  }
  if(verbose) printf("Setting Threshold: %s \n", CAENVME_DecodeError(err_code));
  err_code=0;
  err_code |=   CAENVME_WriteCycle(V812Handle, baseAddr| REG_V812_OUTWIDTH_0_7, &Params.OutputWidth[0],AM, DW);
  err_code |=   CAENVME_WriteCycle(V812Handle, baseAddr| REG_V812_OUTWIDTH_8_15, &Params.OutputWidth[1],AM, DW);
  if(verbose) printf("Setting OutputWidth: %s \n", CAENVME_DecodeError(err_code));
  err_code=0;
  err_code |=   CAENVME_WriteCycle(V812Handle, baseAddr| REG_V812_DEADTIME_0_7, &Params.DeadTime[0],AM, DW);
  err_code |=   CAENVME_WriteCycle(V812Handle, baseAddr| REG_V812_DEADTIME_8_15, &Params.DeadTime[1],AM, DW);
  if(verbose) printf("Setting DeadTime: %s \n", CAENVME_DecodeError(err_code));

  int MajThr = round((((Params.Majority)*50.-25.)/4.));
  err_code =   CAENVME_WriteCycle(V812Handle, baseAddr| REG_V812_MAJORITY, &MajThr,AM, DW);
  if(verbose) printf("Setting majority threshold: %s \n", CAENVME_DecodeError(err_code));

  err_code =   CAENVME_WriteCycle(V812Handle, baseAddr| REG_V812_INHIBITPATTERN, &Params.InhibitPattern,AM, DW);
  if(verbose) printf("Setting Inhibit Pattern: %s \n", CAENVME_DecodeError(err_code));
  CAENVME_End(V812Handle);
  return err_code;
}
