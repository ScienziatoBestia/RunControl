#ifndef GCALTree_h
#define GCALTree_h
#include <unistd.h>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TGraph.h>
#include <stdio.h>
#include <iostream>
#include <string.h> 
#include <TH2.h>
#include <TStyle.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TRandom3.h>
#include <TString.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#define nChannels 36

class GCALTree {

public :
   GCALTree(TString rootfile);
   virtual ~GCALTree();
   virtual void CreateTree(TString rootfile);
   virtual void ResetGCAL();
   virtual void StoreGCAL(double data[nChannels][1024], Double_t, UInt_t);
   virtual void FillTree();

 private:
   TFile* f;
   TTree* myTree;
   TH1F* hNoise[nChannels];
   Double_t dataPedSub[nChannels][1024];
   Double_t BinSum[nChannels],BinSum5[nChannels],BinSum10[nChannels],BinSum15[nChannels],BinSum20[nChannels],MaxValue[nChannels],CommonMode[nChannels];
   Int_t MaxX[nChannels], FirstX[nChannels], NEvent[nChannels];
   Double_t gcalTimeStamp;   
   UInt_t gcalStatus;
   int start;

   Double_t GetCommonMode(Double_t data[], Int_t);
   Double_t GetCommonMode(Double_t data[], Int_t, Int_t, Int_t, Int_t);
};

#endif


GCALTree::GCALTree(TString rootfile) {
  CreateTree(rootfile);
  for(int i=0;i<nChannels;i++) NEvent[i]=0;
}


GCALTree::~GCALTree() {
  f->Write();
  f->Close();
}




///////////////////////////////////////////////////////////////////////////////////////////
void GCALTree::StoreGCAL(double data[nChannels][1024], Double_t timeStamp, UInt_t status) {
///////////////////////////////////////////////////////////////////////////////////////////

  gcalTimeStamp=timeStamp;
  gcalStatus=status;

  Int_t NotSignalChans[4]={8,17,26,35};
  Int_t min1=0, max1=200, min2=400, max2=980;
  int TrigChan=8;

  // Loop on digitizer channels 
  for(Int_t i=0; i<nChannels; i++) {  //loop su canali
    NEvent[i]++;

    // Calculate the common mode
    //std::cout << " Channel " << i << std::endl;
    if(i!=TrigChan) 
      CommonMode[i]= GetCommonMode(data[i], min1, max1, min2, max2 );
    else 
      CommonMode[i]= GetCommonMode(data[i], max1);

    //  Loop on samples 
    //  pedestal subtraction, area, max signal, time of max signal
    int mythres=20;
    int first=0;
    for(Int_t j=0; j<1000; j++)  {  //loop sui 1024

      if(i!=TrigChan) dataPedSub[i][j]=-(data[i][j]-CommonMode[i]);
      else dataPedSub[i][j]=data[i][j]-CommonMode[i];

      if((j>min1 && j<max1)||(j>min2 && j<max2)) hNoise[i]->Fill(dataPedSub[i][j]);       

      BinSum[i]+=dataPedSub[i][j];
    
      if(dataPedSub[i][j]>MaxValue[i])  {
	MaxValue[i]=dataPedSub[i][j];
        MaxX[i]=j;
	first=1;
      } 
      else if (first==1 && dataPedSub[i][j]>mythres && FirstX[i]==0) {
	FirstX[i]=j-1;
      }
    } // end loop sui 1024

    start=MaxX[i];
    for(Int_t j=fmax(0,start-20); j<=start+20; j++)  {         
      if(j>=start-5  && j<=start+5)  BinSum5[i] +=dataPedSub[i][j];
      if(j>=start-10 && j<=start+10) BinSum10[i]+=dataPedSub[i][j];
      if(j>=start-15 && j<=start+15) BinSum15[i]+=dataPedSub[i][j];
      if(j>=start-20 && j<=start+20) BinSum20[i]+=dataPedSub[i][j];
    }

  } // end loop su canali



}

///////////////////////////////////////////////////////////////////////
Double_t GCALTree::GetCommonMode(Double_t data[], Int_t MaxChannel ) {
///////////////////////////////////////////////////////////////////////
   Double_t CommonMode=0;
   Double_t SumSamples=0;
   for(Int_t j=0; j<MaxChannel; j++) {
     SumSamples+=data[j];
   }

   CommonMode=SumSamples/MaxChannel;
   //std::cout << " CommonMode " << CommonMode << std::endl;
   return CommonMode;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
Double_t GCALTree::GetCommonMode(Double_t data[], Int_t Min1, Int_t Max1, Int_t Min2, Int_t Max2 ) {
//////////////////////////////////////////////////////////////////////////////////////////////////////
   Double_t CommonMode=0;
   Double_t SumSamples=0;

   for(Int_t j=Min1; j<Max1; j++) {
     SumSamples+=data[j];
   }

   for(Int_t j=Min2; j<Max2; j++) {
     SumSamples+=data[j];
   }

   Int_t NSamples=(Max1-Min1)+(Max2-Min2);
   CommonMode=SumSamples/NSamples;

   //std::cout << " CommonMode (2sides) " << CommonMode << std::endl;

   return CommonMode;
}



///////////////////////////////////////////////////////////////
void  GCALTree::FillTree() {
///////////////////////////////////////////////////////////////
  myTree->Fill();
}



///////////////////////////////////////////////////////////////
//  Reset GCAL variables
///////////////////////////////////////////////////////////////
void  GCALTree::ResetGCAL() {

  for(Int_t i=0; i<nChannels; i++) {  //loop su canali
    CommonMode[i]=0;
    BinSum[i]=0;
    BinSum5[i]=0;
    BinSum10[i]=0;
    BinSum15[i]=0;
    BinSum20[i]=0;
    MaxValue[i]=0;
    MaxX[i]=0;
    FirstX[i]=0;
  }

  gcalTimeStamp=0.;
  gcalStatus=0;
}



///////////////////////////////////////////////////////////////
void  GCALTree::CreateTree(TString rootfile) {
///////////////////////////////////////////////////////////////

  Int_t nChans = nChannels;
  TString s;
  s.Form("[%i]",nChans);
  printf("GCALTree:: Creating new tree\n");
  f=new TFile(rootfile,"recreate");

  TString ichan,hname;
  for(Int_t i=0; i<nChannels; i++) {  //loop su canali
      ichan.Form("%i",i);
      hname="hNoise"+ichan;
      hNoise[i] = new TH1F(hname,"",2000,-1000,1000);
  }

  myTree=new TTree("gcal","gcal");
  myTree->Branch("BinSum",BinSum,"BinSum"+s+"/D");
  myTree->Branch("BinSum5",BinSum5,"BinSum5"+s+"/D");
  myTree->Branch("BinSum10",BinSum10,"BinSum10"+s+"/D");
  myTree->Branch("BinSum15",BinSum15,"BinSum15"+s+"/D");
  myTree->Branch("BinSum20",BinSum20,"BinSum20"+s+"/D");
  myTree->Branch("MaxValue",MaxValue,"MaxValue"+s+"/D");
  myTree->Branch("MaxX",MaxX,"MaxX"+s+"/I");
  myTree->Branch("NEvent",NEvent,"NEvent"+s+"/I");
  myTree->Branch("FirstX",FirstX,"FirstX"+s+"/I");
  myTree->Branch("CommonMode",CommonMode,"CommonMode"+s+"/D");
  myTree->Branch("gcalTimeStamp",&gcalTimeStamp,"gcalTimeStamp/D");
  myTree->Branch("gcalStatus",&gcalStatus,"gcalStatus/i");

/*
    // List of branches
    TBranch        *b_BinSum;   //!
    TBranch        *b_BinSum5;   //!
    TBranch        *b_BinSum10;   //!
    TBranch        *b_BinSum15;   //!
    TBranch        *b_BinSum20;   //!
    TBranch        *b_MaxValue;   //!
    TBranch        *b_MaxX;   //!
    TBranch        *b_NEvent;   //!
    TBranch        *b_FirstX;   //!
    TBranch        *b_CommonMode;   //!
    TBranch        *b_gcalTimeStamp;   //!

    myTree->SetBranchAddress("BinSum", BinSum, &b_BinSum);
    myTree->SetBranchAddress("BinSum5", BinSum5, &b_BinSum5);
    myTree->SetBranchAddress("BinSum10", BinSum10, &b_BinSum10);
    myTree->SetBranchAddress("BinSum15", BinSum15, &b_BinSum15);
    myTree->SetBranchAddress("BinSum20", BinSum20, &b_BinSum20);
    myTree->SetBranchAddress("MaxValue", MaxValue, &b_MaxValue);
    myTree->SetBranchAddress("MaxX", MaxX, &b_MaxX);
    myTree->SetBranchAddress("NEvent", NEvent, &b_NEvent);
    myTree->SetBranchAddress("FirstX", FirstX, &b_FirstX);
    myTree->SetBranchAddress("CommonMode", CommonMode, &b_CommonMode);
    myTree->SetBranchAddress("gcalTimeStamp", &gcalTimeStamp, &b_gcalTimeStamp);
*/

}

