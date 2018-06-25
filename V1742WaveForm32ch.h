//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Nov 10 08:51:47 2015 by ROOT version 5.34/32
// from TTree test/test
// found on file: testfile.root
//////////////////////////////////////////////////////////

#ifndef V1742WaveForm32ch_h
#define V1742WaveForm32ch_h
#include <unistd.h>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TGraph.h>
#include <TF1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TAxis.h>
#include <TSpectrum.h>
#include <TLine.h>
TGraph *gr1;
TGraph *grMean;
TH1F *hMean;
TH1F *hMaxValue;
TH1F *hArea;
Int_t XStartTreno;
TGraph *grFit;
Double_t dataTemplate[2000000];
TLine * line[32];
TGraph *grDiodo;
// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.


class V1742WaveForm32ch {
public :
  
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Double_t        dataV1742[36][1024];

   // List of branches
   TBranch        *b_dataV1742;   //!
 
   //V1742WaveForm32ch(TTree *tree=0);
   V1742WaveForm32ch(TString);
   virtual ~V1742WaveForm32ch();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();

   virtual Bool_t   Notify();
   virtual void     Show(Int_t channel, Long64_t entry = -1, Int_t freq=1, Option_t *opt="AL");
   virtual void     ShowAll(Long64_t entry = -1);
};

#endif
#ifdef V1742WaveForm32ch_cxx


//V1742WaveForm32ch::V1742WaveForm32ch(TTree *tree) : fChain(0)
V1742WaveForm32ch::V1742WaveForm32ch(TString runID) : fChain(0)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.

   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(runID+"_RawDataBaF.root");
      if (!f || !f->IsOpen()) {
         f = new TFile(runID+"_RawDataBaF.root");
      }
      f->GetObject("rawdata",tree);

   }

   Init(tree);
}

V1742WaveForm32ch::~V1742WaveForm32ch()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t V1742WaveForm32ch::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}

Long64_t V1742WaveForm::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void V1742WaveForm32ch::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

 

   // Set branch addresses and branch pointers

   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("dataV1742", dataV1742, &b_dataV1742);
 
   Notify();
}

Bool_t V1742WaveForm32ch::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void V1742WaveForm32ch::Show(Int_t channel, Long64_t entry, Int_t freq, Option_t *opt)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);

   Int_t n = 1024;
   Double_t x[1024];

   for (Int_t j=0; j<n; j++)
     x[j]=j/(double) freq;

   gr1 = new TGraph (n, x,dataV1742[channel]);
   gr1->Draw(opt);
   TAxis *Xaxis = gr1->GetXaxis();
   Xaxis->SetTitle("t (ns)");
   TAxis *Yaxis = gr1->GetYaxis();
   Yaxis->SetTitle("ADC Counts");
   gPad->Update();

}


void V1742WaveForm32ch::ShowAll(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
   Int_t pos[2]={20,20};
   Int_t n = 1024;
   Double_t xBin[1024];
   Double_t SumData[1024]={0};
   for (Int_t j=0; j<n; j++) {
     xBin[j]=j;
   }
   TCanvas *c1 = new TCanvas("c1","c1",1400,900);
   c1->Divide(8,4);
   Int_t ichan[32]={0,1,2,3,4,5,6,7,9,10,11,12,13,14,15,16,18,19,20,21,22,23,24,25,27,28,29,30,31,32,33,34};
   for(Int_t c=0; c<32; c++)
     {
       char name[10];
       sprintf(name,"Channel %i",ichan[c]);
       c1->cd(c+1);
       TGraph *gr1 = new TGraph (n, xBin,dataV1742[ichan[c]]);
       gr1->Draw("AL");
       gr1->SetTitle(name);
       TAxis *Xaxis = gr1->GetXaxis();
       Xaxis->SetTitle("t (ns)");
       TAxis *Yaxis = gr1->GetYaxis();
       Yaxis->SetTitle("ADC Counts");
       
       //if(ichan[c]==pos[0] || ichan[c]==pos[1]) {
         Yaxis->SetRangeUser(2500,4000);
       //} else {
        // Yaxis->SetRangeUser(3000,4096);
       //}
     }
}

Int_t V1742WaveForm32ch::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

#endif // #ifdef WaveForm32ch_cxx
