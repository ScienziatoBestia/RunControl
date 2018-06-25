#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TCanvas.h"
#include "TBrowser.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TObjArray.h"
#include "TText.h"
#include "TMath.h"
#include "TKey.h"
#include "TMatrix.h"
#include "TString.h"
#include "Riostream.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#define nStrip 1024
#define nChip 8

class SiStripAna {
  public:
    SiStripAna(TString);
    virtual ~SiStripAna();

  private:
    virtual void Init(TTree *);
    virtual void switchOnBranches();
    void HistoBook(TFile *);

    double xSigma[nStrip], ySigma[nStrip];

    TTree* T;  
    TObjArray* Hlist;

    TH1F* xCommonNoise[nChip];
    TH1F* yCommonNoise[nChip];

    TH1F* nxcluster;
    TH1F* nBadxCluster;
    TH1F* xSeed;
    TH1F* xClusterEnergy;
    TH1F* xClusterMultiplicity;
    TH1F* xsigma;
    TH1F* xClusterS2N;

    TH1F* nycluster;
    TH1F* nBadyCluster;
    TH1F* ySeed;
    TH1F* yClusterEnergy;
    TH1F* yClusterMultiplicity;
    TH1F* ysigma;
    TH1F* yClusterS2N;

    TH2F* CorrClusterEnergy;

////////////////////////////////////////////////////////
// Tree variables
////////////////////////////////////////////////////////
   Double_t        SumTot;
   Double_t        SumNotHit;
   Double_t        BinSum[18];
   Double_t        BinSumCalib[18];
   Double_t        BinSumGate[18];
   Double_t        BinSumCalibGate[18];
   Double_t        BinSumCalibFast[18];
   Double_t        BinSumFast[18];
   Double_t        MaxValue[18];
   Double_t        SlowAverage[18];
   Double_t        MaxValueTot;
   Int_t           MaxX[18];
   Int_t           NEvent[18];
   Int_t           FirstX[18];
   Double_t        SumSignal;
   Double_t        SumSignalCalib;
   Double_t        SumSignalGate;
   Double_t        SumSignalCalibGate;
   Int_t           ClusterSize;
   Int_t           NPos[18];
   Int_t           Hit[18];
   Double_t        FracPN[18];
   Double_t        TriggerTime[18];
   Double_t        RiseTime[18];
   Double_t        Slope[18]; 
   Double_t        CommonMode[18];
   Double_t        BaFTimeStamp;
   Int_t           nSiStripEvent;  
   Double_t        xCMN[8];
   Double_t        yCMN[8];
   Double_t        SiStripTimeStamp;   
   Int_t           nxCluster;
   Int_t           xClusterSeed[138];   //[nxCluster]
   Double_t        xClusterSignal[138][11];   //[nxCluster]
   Int_t           nyCluster;  
   Int_t           yClusterSeed[144];   //[nyCluster]
   Double_t        yClusterSignal[144][11];   //[nyCluster]
   Double_t        HPGeEnergyCh0;
   Double_t        HPGeEnergyCh1;
   Double_t        HPGeTimeStamp; 
   Double_t        HPGeCommonMode;
   Double_t        HPGexMax;



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
   TBranch        *b_BaFTimeStamp;   //!
   TBranch        *b_nSiStripEvent;   //!
   TBranch        *b_xCMN;   //!
   TBranch        *b_yCMN;   //!
   TBranch        *b_SiStripTimeStamp;   //!
   TBranch        *b_nxCluster;   //!
   TBranch        *b_xClusterSeed;   //!
   TBranch        *b_xClusterSignal;   //!
   TBranch        *b_nyCluster;   //!
   TBranch        *b_yClusterSeed;   //!
   TBranch        *b_yClusterSignal;   //!
   TBranch        *b_HPGeEnergyCh0;   //! 
   TBranch        *b_HPGeEnergyCh1;   //! 
   TBranch        *b_HPGeTimeStamp;   //! 
   TBranch        *b_HPGeCommonMode;   //!
   TBranch        *b_HPGexMax;   //!

};
