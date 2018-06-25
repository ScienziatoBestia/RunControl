#define V1742WaveForm_cxx
#include "V1742WaveForm.h"
#include <stdio.h>
#include <iostream>
#include <string.h> 
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TRandom3.h>
#include <TString.h>


void V1742WaveForm::Loop()
{
   if (fChain == 0) return;
   Double_t MaxValue,CommonMode;
   int channel=7;
   Long64_t nentries = fChain->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;

      }
}



