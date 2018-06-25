//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Mar 15 10:43:44 2017 by ROOT version 5.34/36
// from TTree rawdata/rawdata
// found on file: RawDataDt5780.root
//////////////////////////////////////////////////////////

#ifndef DT5780WaveForm_h
#define DT5780WaveForm_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class DT5780WaveForm {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Int_t           dataDT5780[8][10000];

   // List of branches
   TBranch        *b_dataDT5780;   //!

   //DT5780WaveForm(TTree *tree=0);
   DT5780WaveForm(TString runID);
   virtual ~DT5780WaveForm();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(int channel, Long64_t entry = -1, int npoints);
};

#endif

#ifdef DT5780WaveForm_cxx
//DT5780WaveForm::DT5780WaveForm(TTree *tree) : fChain(0) 
DT5780WaveForm::DT5780WaveForm(TString runID) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   TTree *tree=0;

   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(runID+"_RawDataHPGe.root");
      if (!f || !f->IsOpen()) {
         f = new TFile(runID+"_RawDataHPGe.root");
      }
      f->GetObject("rawdata",tree);

   }
   Init(tree);
}



DT5780WaveForm::~DT5780WaveForm()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t DT5780WaveForm::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t DT5780WaveForm::LoadTree(Long64_t entry)
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

void DT5780WaveForm::Init(TTree *tree)
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

   fChain->SetBranchAddress("dataDT5780", dataDT5780, &b_dataDT5780);
   Notify();
}

Bool_t DT5780WaveForm::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void DT5780WaveForm::Show( int channel, Long64_t entry, int npoints)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
   Int_t n = npoints;
   Double_t x[10000], Trace1[10000],Trace2[10000],DigitalTrace1[10000],DigitalTrace2[10000];
   FILE* file = fopen("dataShow.txt","w");
   for (Int_t j=0; j<n; j++) {
     x[j]=j*10;
     Trace1[j]=dataDT5780[channel*4][j];
     Trace2[j]=dataDT5780[channel*4+1][j];
     DigitalTrace1[j]=1000*dataDT5780[channel*4+2][j];
     DigitalTrace2[j]=1000*dataDT5780[channel*4+3][j];
     fprintf(file, "%i\n", Trace2[j]);
   }
   printf("Trace2 %i \n",Trace2[600]);
   gr1 = new TGraph (n, x,Trace1);
   gr1->Draw("AL");
   TAxis *Xaxis = gr1->GetXaxis();
   Xaxis->SetTitle("t (ns)");
   TAxis *Yaxis = gr1->GetYaxis();
   Yaxis->SetTitle("ADC Counts");
   //Yaxis->SetRangeUser(-2000,16000);
   Yaxis->SetRangeUser(-500,8000);
   gr2 = new TGraph (n, x,Trace2);
   gr2->SetLineColor(2);
   gr2->Draw("L");
   gr3 = new TGraph (n, x,DigitalTrace1);
   gr3->SetLineColor(3);
   gr3->Draw("L");
   gr4 = new TGraph (n, x,DigitalTrace2);
   gr4->SetLineColor(4);
   gr4->Draw("L");
   gPad->Update();
   fclose(file);
}
Int_t DT5780WaveForm::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef DT5780WaveForm_cxx
