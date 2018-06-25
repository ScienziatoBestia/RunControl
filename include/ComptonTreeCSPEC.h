#ifndef ComptonTree_h
#define ComptonTree_h
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

#define nChannels 18
#define BinSumCut 2000
#define SumGate 630

Double_t GetCommonMode(Double_t data[], Int_t MaxChannel )
{
   Double_t CommonMode=0;
   Double_t SumSamples=0;
   for(Int_t j=0; j<MaxChannel; j++) {
     SumSamples+=data[j];
   }

   CommonMode=SumSamples/MaxChannel;

   return CommonMode;
}


class ComptonTree {

public :
   ComptonTree(TString rootfile, int newTree=1);
   virtual ~ComptonTree();
   virtual void CreateTree(TString rootfile,int newTree);
   virtual void FillTree(double data[nChannels][1024], double HpGeEnergy[2]);

 private:
   TFile* f;
   TTree* myTree;
   Double_t dataPedSub[nChannels][1024];
   Double_t SumTot, SumNotHit, BinSum[nChannels],BinSumCalib[nChannels], BinSumGate[nChannels], BinSumCalibGate[nChannels],BinSumCalibFast[nChannels], BinSumFast[nChannels], MaxValue[nChannels], MaxValueTot, SumSignal, SumSignalCalib, SumSignalGate, SumSignalCalibGate, CommonMode[nChannels], FracPN[nChannels], TriggerTime[nChannels], RiseTime[nChannels], Slope[nChannels];
   Double_t SlowAverage[nChannels];
   Int_t MaxX[nChannels],FirstX[nChannels],ClusterSize,NPos[nChannels],Pos,Neg,Hit[nChannels],start,NEvent[nChannels];
   Int_t MaxChannelPed;   
   Double_t HpGeEnergyCh0, HpGeEnergyCh1;
};

#endif

ComptonTree::ComptonTree(TString rootfile, int newTree)
{
  CreateTree(rootfile, newTree);
  for(int i=0;i<nChannels;i++) {
    NEvent[i]=0;
  }
}

ComptonTree::~ComptonTree()
{
  f->Write();
  f->Close();
}



void ComptonTree::FillTree(double data[nChannels][1024], double HpGeEnergy[2]) {
  Int_t SumCut = BinSumCut;
  Int_t Gate = SumGate;
  Int_t PosChans[2]={40,40};
  Int_t NotSignalChans[4]={8,17,0,35};

  Double_t CalibFactor[18]= {1,0.738651,1.081532,1.145263,1.185543,0.880944,1.581499,1.545455,1,0.956609,1.173398,0.831179,1.214398,1.021140,0.832819,1.166409,0.906950,1};

///////////////////////////////////////////////////////////////
//  Loop on digitizer channels [18]
///////////////////////////////////////////////////////////////
  for(Int_t i=0; i<nChannels; i++) {  //loop su canali
    NEvent[i]++;

    BinSum[i]=0;
    BinSumCalib[i]=0;

    BinSumGate[i]=0;
    BinSumCalibGate[i]=0;
    BinSumCalibFast[i]=0;
    BinSumFast[i]=0;

    MaxValue[i]=0;
    MaxX[i]=0;
    FirstX[i]=0;

    SlowAverage[i]=0;

    if(i==PosChans[0] || i==PosChans[1]) {
      MaxChannelPed=150;
    } else {
      MaxChannelPed=150;
    }

///////////////////////////////////////////////////////////////
//  Common mode
///////////////////////////////////////////////////////////////
    CommonMode[i]= GetCommonMode(data[i], MaxChannelPed);

///////////////////////////////////////////////////////////////
//  Loop on samples 
//  pedestal subtraction, area, max signal, time of max signal
///////////////////////////////////////////////////////////////
    FracPN[i]=0;
    Pos=0; Neg=0;
    int first=0;
    int mythres=20;
    for(Int_t j=0; j<1000; j++)  { 
      if(i!=PosChans[0] && i!= PosChans[1] ) {
	dataPedSub[i][j]=-(data[i][j]-CommonMode[i]);
      } else {
	dataPedSub[i][j]=(data[i][j]-CommonMode[i]);
      }
      if(dataPedSub[i][j]>0) {
	Pos++;
      } else {
	Neg++;
      } 

      BinSum[i]+=dataPedSub[i][j];
    
      if(dataPedSub[i][j]>MaxValue[i])  {
	  MaxValue[i]=dataPedSub[i][j];
	  MaxX[i]=j;
	  first=1;
      } else if (first==1 && dataPedSub[i][j]>mythres && FirstX[i]==0) {
          FirstX[i]=j-1;
      }
    } // end loop sui 1024

    BinSumCalib[i]=BinSum[i]/CalibFactor[i]; //apply calibration factors

    FracPN[i]=(double) Pos/Neg;

///////////////////////////////////////////////////////////////
//  Loop sul gate di 630ns
///////////////////////////////////////////////////////////////
    start=MaxX[i];
    for(Int_t j=fmax(0,start-10); j<fmin(start+Gate,990); j++)  {  //loop sul gate di 630 ns	
      BinSumGate[i]+=dataPedSub[i][j];
      if(j>=start-5 && j<=start+5) BinSumFast[i]+=dataPedSub[i][j];
    } 
    BinSumCalibGate[i]=BinSumGate[i]/CalibFactor[i]; 
    BinSumCalibFast[i]=BinSumFast[i]/CalibFactor[i]; 

    Int_t nSamples=0;
    for(Int_t j=start+30; j<fmin(start+10+Gate,990); j++)  {  //loop sul gate di 630 ns	
      SlowAverage[i]+=dataPedSub[i][j];
      nSamples++;
    } 
    SlowAverage[i] /= nSamples;


    //calcolo trigger time
    Double_t Threshold=0.5*MaxValue[i];
    Double_t Threshold1=0.1*MaxValue[i];
    Double_t Threshold2=0.9*MaxValue[i];
    Double_t xa=0;
    Double_t ya=0;
    Double_t xb=0;
    Double_t yb=0;
    Double_t q=0;
    Slope[i]=0;
    Double_t xa1=0;
    Double_t ya1=0;
    Double_t xb1=0;
    Double_t yb1=0;
    Double_t q1=0;
    Double_t Slope1=0;
    Double_t xa2=0;
    Double_t ya2=0;
    Double_t xb2=0;
    Double_t yb2=0;
    Double_t q2=0;
    Double_t Slope2=0;

    TriggerTime[i]=0;
    RiseTime[i]=0;
    //   for(Int_t j=MaxX[i]; j!=0; j--) {
    for(Int_t j=FirstX[i]; j!=0; j--) {
      if(dataPedSub[i][j]>=Threshold) {
	xa=j;
	ya=dataPedSub[i][j];
      } else {
	xb=j;
	yb=dataPedSub[i][j];
	break;
      }
    }

    for(Int_t j=FirstX[i]; j!=0; j--) {
      if(dataPedSub[i][j]>=Threshold1) {
	xa1=j;
	ya1=dataPedSub[i][j];
      } else {
	xb1=j;
	yb1=dataPedSub[i][j];
	break;
      }
    }

    //   for(Int_t j=MaxX[i]; j!=0; j--) {
    for(Int_t j=FirstX[i]; j!=0; j--) {
      if(dataPedSub[i][j]>=Threshold2) {
	xa2=j;
	ya2=dataPedSub[i][j];
      } else {
	xb2=j;
	yb2=dataPedSub[i][j];
	break;
      }
    }


    if(xa!=xb) {
      Slope[i]=(yb-ya)/(xb-xa);
      q=ya-Slope[i]*xa;
      TriggerTime[i]=(Threshold-q)/Slope[i];	 
    }

    if(xa1!=xb1) {
      Slope1=(yb1-ya1)/(xb1-xa1);
      q1=ya1-Slope1*xa1;	 
    }

    if(xa2!=xb2) {
      Slope2=(yb2-ya2)/(xb2-xa2);
      q2=ya2-Slope2*xa2;
      RiseTime[i]=(Threshold2-q2)/Slope2 - (Threshold1-q1)/Slope1  ;	 
    }
  } // end loop su canali

  //for(Int_t i=0; i<nChannels; i++) {  //loop su canali
  //  std::cout << " *** Event " << NEvent[i] << "  Channel " << i << "   MaxValue "<< MaxValue[i] << "   SlowAverage " << SlowAverage[i] << "   ratio " << MaxValue[i]/Slow[i] << std::endl;
  //} 

  SumTot=0;
  SumNotHit=0;
  MaxValueTot=0;
  
  SumSignal = 0;
  SumSignalCalib = 0;
  
  SumSignalGate = 0;
  SumSignalCalibGate = 0;

  ClusterSize=0;

  for(Int_t i=0; i<nChannels; i++) {  //loop su canali
    NPos[i]=0;
    if(MaxX[i]>10)  {   // conto i campionamenti positivi consecutivi
      for(Int_t j=MaxX[i]; j<1000-MaxX[i]; j++)  {
	if(dataPedSub[i][j-1]>0&&dataPedSub[i][j-2]>0&&dataPedSub[i][j]>0)  {
	  NPos[i]++;
	} else {
	  break;
	} 
      }  
    } 
    if( i!= NotSignalChans[0] && i!=NotSignalChans[1] && i!=NotSignalChans[2] && i!=NotSignalChans[3] ) { //tolgo il canale Tr0 e il dinodo (ch13)
      if(BinSumGate[i]>SumCut) SumTot += BinSumGate[i]; 
      MaxValueTot += MaxValue[i];
    }

    Hit[i]=0;

    if(i!=NotSignalChans[0] && i!=NotSignalChans[1] && i!=NotSignalChans[2] && i!=NotSignalChans[3] && BinSumCalibGate[i]>SumCut) {

      Hit[i]=1;
      SumSignal+= BinSum[i];
      SumSignalCalib+= BinSumCalib[i];
      SumSignalGate+= BinSumGate[i];
      SumSignalCalibGate+= BinSumCalibGate[i];
      ClusterSize++;
    }
  } // end loop su canali

  HpGeEnergyCh0=HpGeEnergy[0];
  HpGeEnergyCh1=HpGeEnergy[1];

  myTree->Fill();
}


void  ComptonTree::CreateTree(TString rootfile, int newTree)
{

  Int_t nChans = nChannels;
  TString s;
  s.Form("[%i]",nChans);
  if(newTree) {
    printf("ComptonTree:: creo nuovo tree\n");
    f=new TFile(rootfile,"recreate");
    myTree=new TTree("Analisi","Analisi");
    myTree->Branch("SumTot",&SumTot,"SumTot/D");
    myTree->Branch("SumNotHit",&SumNotHit,"SumNotHit/D");
    myTree->Branch("BinSum",BinSum,"BinSum"+s+"/D");
    myTree->Branch("BinSumCalib",BinSumCalib,"BinSumCalib"+s+"/D");
    myTree->Branch("BinSumGate",BinSumGate,"BinSumGate"+s+"/D");
    myTree->Branch("BinSumCalibGate",BinSumCalibGate,"BinSumCalibGate"+s+"/D");
    myTree->Branch("BinSumCalibFast",BinSumCalibFast,"BinSumCalibFast"+s+"/D");
    myTree->Branch("BinSumFast",BinSumFast,"BinSumFast"+s+"/D");
    myTree->Branch("MaxValue",MaxValue,"MaxValue"+s+"/D");
    myTree->Branch("SlowAverage",SlowAverage,"SlowAverage"+s+"/D");
    myTree->Branch("MaxValueTot",&MaxValueTot,"MaxValueTot/D");
    myTree->Branch("MaxX",MaxX,"MaxX"+s+"/I");
    myTree->Branch("NEvent",NEvent,"NEvent"+s+"/I");
    myTree->Branch("FirstX",FirstX,"FirstX"+s+"/I");
    myTree->Branch("SumSignal",&SumSignal,"SumSignal/D");
    myTree->Branch("SumSignalCalib",&SumSignalCalib,"SumSignalCalib/D");
    myTree->Branch("SumSignalGate",&SumSignalGate,"SumSignalGate/D");
    myTree->Branch("SumSignalCalibGate",&SumSignalCalibGate,"SumSignalCalibGate/D");
    myTree->Branch("ClusterSize",&ClusterSize,"ClusterSize/I");
    myTree->Branch("NPos",NPos,"NPos"+s+"/I");
    myTree->Branch("Hit",Hit,"Hit"+s+"/I");
    myTree->Branch("FracPN",FracPN,"FracPN"+s+"/D");
    myTree->Branch("TriggerTime",TriggerTime,"TriggerTime"+s+"/D");
    myTree->Branch("RiseTime",RiseTime,"RiseTime"+s+"/D");
    myTree->Branch("Slope",Slope,"Slope"+s+"/D");
    myTree->Branch("CommonMode",CommonMode,"CommonMode"+s+"/D");
    myTree->Branch("HpGeEnergyCh0",&HpGeEnergyCh0,"HpGeEnergyCh0/D");
    myTree->Branch("HpGeEnergyCh1",&HpGeEnergyCh1,"HpGeEnergyCh1/D");
   
  } else {
    printf("appendo tree\n");
    f=new TFile(rootfile,"update");    
    f->GetObject("Analisi",myTree);

    // List of branches
    TBranch        *b_SumTot;   //!
    TBranch        *b_SumNotHit;   //!
    TBranch        *b_BinSum;   //!
    TBranch        *b_BinSumCalib;   //!
    TBranch        *b_BinSumGate;   //!
    TBranch        *b_BinSumCalibGate;   //!
    TBranch        *b_BinSumCalibFast;   //!
    TBranch        *b_BinSumFast;   //!
    TBranch        *b_MaxValue;   //!
    TBranch        *b_SlowAverage;   //!
    TBranch        *b_MaxValueTot;   //!
    TBranch        *b_MaxX;   //!
    TBranch        *b_NEvent;   //!
    TBranch        *b_FirstX;   //!
    TBranch        *b_SumSignal;   //!
    TBranch        *b_SumSignalCalib;   //!
    TBranch        *b_SumSignalGate;   //!
    TBranch        *b_SumSignalCalibGate;   //!
    TBranch        *b_ClusterSize;   //!
    TBranch        *b_NPos;   //!
    TBranch        *b_Hit;   //!
    TBranch        *b_FracPN;   //!
    TBranch        *b_TriggerTime;   //!
    TBranch        *b_RiseTime;   //!
    TBranch        *b_Slope;   //!
    TBranch        *b_CommonMode;   //!
    TBranch        *b_HpGeEnergyCh0;   //!
    TBranch        *b_HpGeEnergyCh1;   //!

   //   myTree->SetMakeClass(0);

    myTree->SetBranchAddress("SumTot", &SumTot, &b_SumTot);
    myTree->SetBranchAddress("SumNotHit", &SumNotHit, &b_SumNotHit);
    myTree->SetBranchAddress("BinSum", BinSum, &b_BinSum);
    myTree->SetBranchAddress("BinSumCalib", BinSumCalib, &b_BinSumCalib);
    myTree->SetBranchAddress("BinSumGate", BinSumGate, &b_BinSumGate);
    myTree->SetBranchAddress("BinSumCalibGate", BinSumCalibGate, &b_BinSumCalibGate);
    myTree->SetBranchAddress("BinSumCalibFast", BinSumCalibFast, &b_BinSumCalibFast);
    myTree->SetBranchAddress("BinSumFast", BinSumFast, &b_BinSumFast);
    myTree->SetBranchAddress("MaxValue", MaxValue, &b_MaxValue);
    myTree->SetBranchAddress("SlowAverage", SlowAverage, &b_SlowAverage);
    myTree->SetBranchAddress("MaxValueTot", &MaxValueTot, &b_MaxValueTot);
    myTree->SetBranchAddress("MaxX", MaxX, &b_MaxX);
    myTree->SetBranchAddress("NEvent", NEvent, &b_NEvent);
    myTree->SetBranchAddress("FirstX", FirstX, &b_FirstX);
    myTree->SetBranchAddress("SumSignal", &SumSignal, &b_SumSignal);
    myTree->SetBranchAddress("SumSignalCalib", &SumSignalCalib, &b_SumSignalCalib);
    myTree->SetBranchAddress("SumSignalGate", &SumSignalGate, &b_SumSignalGate);
    myTree->SetBranchAddress("SumSignalCalibGate", &SumSignalCalibGate, &b_SumSignalCalibGate);
    myTree->SetBranchAddress("ClusterSize", &ClusterSize, &b_ClusterSize);
    myTree->SetBranchAddress("NPos", NPos, &b_NPos);
    myTree->SetBranchAddress("Hit", Hit, &b_Hit);
    myTree->SetBranchAddress("FracPN", FracPN, &b_FracPN);
    myTree->SetBranchAddress("TriggerTime", TriggerTime, &b_TriggerTime);
    myTree->SetBranchAddress("RiseTime", RiseTime, &b_RiseTime);
    myTree->SetBranchAddress("Slope", Slope, &b_Slope);
    myTree->SetBranchAddress("CommonMode", CommonMode, &b_CommonMode);
    myTree->SetBranchAddress("HpGeEnergyCh0", &HpGeEnergyCh0, &b_HpGeEnergyCh0);
    myTree->SetBranchAddress("HpGeEnergyCh1", &HpGeEnergyCh1, &b_HpGeEnergyCh1);
  }
}
