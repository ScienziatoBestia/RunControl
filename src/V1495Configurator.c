
#include "../include/V1495Configurator.h"
#include <math.h>

#define verbose 0

void WriteConfigFileV1495(char* filename, V1495Params_t *Params) {
  FILE* file = fopen(filename,"w");
  int ch,gr;
  fprintf(file,"LINKNUM %i \n\n",Params->LinkNum);
  fprintf(file,"BASEADDRESS 0x%X \n\n",Params->BaseAddress);
  fclose(file);
}


void ParseConfigFileV1495(char* filename, V1495Params_t *Params) {

  FILE* file = fopen(filename,"r");

  char str[1000], str1[1000];
  int read, value, ret,ch,gr;
  while(!feof(file)) {
    
    read = fscanf(file, "%s", str);

    if( !read || (read == EOF) || !strlen(str)) continue;
    if(str[0] == '#') {fgets(str, 1000, file); continue;}    // skip comments
    if (strstr(str, "LINKNUM")!=NULL) {
      read = fscanf(file, "%i", &Params->LinkNum);
      if(verbose) printf("ParseConfigFileV1495: LinkNumber: %i \n",Params->LinkNum);
      continue;
    } else if (strstr(str, "BASEADDRESS")!=NULL) {
      read = fscanf(file, "%X", &Params->BaseAddress);
      if(verbose) printf("ParseConfigFileV1495: BaseAddress: %X \n",Params->BaseAddress);
      continue;
    } else  printf("%s: invalid setting\n", str);
  }
  fclose(file);
}




int TestConnection(V1495Params_t *Params) {
  int handleV1495;

  int err_code = CAENVME_Init(cvV2718,Params->LinkNum , 0, &handleV1495);

  //if(err_code==cvSuccess) CAENVME_SystemReset(handleV1495);

  if(err_code==cvSuccess) CAENVME_End(handleV1495);

  return err_code;
}
