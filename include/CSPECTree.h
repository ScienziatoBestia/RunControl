#ifndef CSPECTree_h
#define CSPECTree_h
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
#include <algorithm>
#include <string>

#define nChannels 18
#define BinSumCut 2000
#define SumGate 630


#define nStrip 1024
#define nChip 8
#define nStripChip 128

using std::vector;
using namespace std;



class CSPECTree {

public :
   CSPECTree(TString rootfile);
   virtual ~CSPECTree();
   virtual void CreateTree(TString rootfile);
   virtual void ResetBaF();
   virtual void StoreBaF(double data[nChannels][1024], Double_t, UInt_t);
   virtual void ResetHPGe();
   virtual void StoreHPGe(double energy[2], Double_t, UInt_t, Double_t, Double_t);
   virtual void ResetSiStrip();
   virtual void StoreSiStrip(uint16_t dataV1495[2][1024], Double_t, UInt_t);
   virtual void InitSiStrip();
   virtual void FillTree();
   void switchOnBranches(TString); 

 private:
   TFile* f;
   TTree* myTree;

   Double_t dataPedSub[nChannels][1024];
   Double_t SumTot, SumNotHit, MaxValue[nChannels], MaxValueTot, SumSignal, SumSignalCalib, SumSignalGate, SumSignalCalibGate;
   Double_t BinSum[nChannels],BinSumCalib[nChannels], BinSumGate[nChannels], BinSumCalibGate[nChannels],BinSumCalibFast[nChannels], BinSumFast[nChannels];
   Double_t SlowAverage[nChannels], CommonMode[nChannels], FracPN[nChannels], TriggerTime[nChannels], RiseTime[nChannels], Slope[nChannels];
   Int_t MaxX[nChannels], FirstX[nChannels], ClusterSize, NPos[nChannels], NEvent[nChannels], Hit[nChannels];
   Int_t Pos, Neg, start, MaxChannelPed;   
   Double_t BaFTimeStamp;
   UInt_t BaFStatus;

   Double_t BaFCommonMode(Double_t data[], Int_t);

   Double_t HPGeEnergyCh0, HPGeEnergyCh1;
   Double_t HPGeTimeStamp, HPGeCommonMode, HPGexMax;
   UInt_t HPGeStatus;

   Double_t SiStripTimeStamp;
   UInt_t SiStripStatus;   
   Float_t xBadChannel[nStrip], yBadChannel[nStrip];
   Float_t xPed[nStrip], yPed[nStrip];
   Float_t xSigma[nStrip], ySigma[nStrip];
   Double_t xCMN[nChip], yCMN[nChip]; 
   int nx[nChip], ny[nChip];
   Double_t SiStripxSignal[nStrip], SiStripySignal[nStrip]; 
   Int_t nxCluster, nyCluster,nSiStripEvent;
   Int_t xClusterSeed[nStrip], yClusterSeed[nStrip];
   Int_t isBadxCluster[nStrip],isBadyCluster[nStrip]; 
   Double_t xClusterSignal[nStrip][11], yClusterSignal[nStrip][11];
   Double_t xCOG[nStrip], etaX[nStrip], xLinEta[nStrip];
   Double_t yCOG[nStrip], etaY[nStrip], yLinEta[nStrip];
   std::vector<double> xSeedSignal;
   std::map<int,double> xSeedMap;
   std::vector<double> ySeedSignal;
   std::map<int,double> ySeedMap;  

   void StripCommonNoise(uint16_t data[2][nStrip]);
   void MakeXStripCluster(int);
   void MakeYStripCluster(int);

   TH1F* Signal;
   void FillSignal(double signal[nStrip]);

   TH1F* xped;
   TH1F* xsigma;
   TH1F* yped;
   TH1F* ysigma;
   TH1F* xbad;
   TH1F* ybad; 

   TObjArray* Hlist;
};

#endif


CSPECTree::CSPECTree(TString rootfile) {
  CreateTree(rootfile);

  for(int i=0;i<nChannels;i++) NEvent[i]=0;

  nSiStripEvent=0;

  myTree->SetBranchStatus("*", 0);  
}


CSPECTree::~CSPECTree() {
  f->cd();
  Hlist->Write();
  myTree->Write();

  f->Close();
}




///////////////////////////////////////////////////////////////////////////////////////////
void CSPECTree::StoreBaF(double data[nChannels][1024], Double_t timeStamp, UInt_t status) {
///////////////////////////////////////////////////////////////////////////////////////////

  BaFTimeStamp=timeStamp;
  BaFStatus=status;

  Int_t SumCut = BinSumCut;
  Int_t Gate = SumGate;
  Int_t PosChans[2]={40,40};
  Int_t NotSignalChans[4]={8,17,0,35};

  Double_t CalibFactor[18]= {1,0.738651,1.081532,1.145263,1.185543,0.880944,1.581499,1.545455,1,0.956609,1.173398,0.831179,1.214398,1.021140,0.832819,1.166409,0.906950,1};

  ///////////////////////////////////////
  //  Loop on digitizer channels [18]
  ///////////////////////////////////////
  for(Int_t i=0; i<nChannels; i++) { 
    NEvent[i]++;

    if(i==PosChans[0] || i==PosChans[1]) {
      MaxChannelPed=150;
    } 
    else {
      MaxChannelPed=150;
    }

    ////////////////////////////////////
    // Calculate the common mode
    ////////////////////////////////////
    CommonMode[i]= BaFCommonMode(data[i], MaxChannelPed);

    ///////////////////////////////////////////////////////////////
    //  Loop on samples 
    //  pedestal subtraction, area, max signal, time of max signal
    ///////////////////////////////////////////////////////////////
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


  for(Int_t i=0; i<nChannels; i++) {  //loop su canali
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

    if(i!=NotSignalChans[0] && i!=NotSignalChans[1] && i!=NotSignalChans[2] && i!=NotSignalChans[3] && BinSumCalibGate[i]>SumCut) {
      Hit[i]=1;
      SumSignal+= BinSum[i];
      SumSignalCalib+= BinSumCalib[i];
      SumSignalGate+= BinSumGate[i];
      SumSignalCalibGate+= BinSumCalibGate[i];
      ClusterSize++;
    }

  } // end loop su canali

}


///////////////////////////////////////////////////////////////
//  Store HPGe variables
///////////////////////////////////////////////////////////////
void  CSPECTree::StoreHPGe(double energy[2], Double_t timeStamp, UInt_t status, Double_t CommonMode, Double_t xMax) {
  HPGeEnergyCh0=energy[0];
  HPGeEnergyCh1=energy[1];
  HPGeTimeStamp=timeStamp;
  HPGeCommonMode=CommonMode;
  HPGexMax=xMax;
  HPGeStatus=status;
}


/////////////////////////////////////////////////////////////////////////////////////////////
void CSPECTree::StoreSiStrip(uint16_t data[2][nStrip], Double_t timeStamp, UInt_t status) {
/////////////////////////////////////////////////////////////////////////////////////////////

  SiStripTimeStamp=timeStamp;
  SiStripStatus=status;

  nSiStripEvent++;
  cout<< "nSiStripEvent " << nSiStripEvent << endl;

  StripCommonNoise(data);  

  int chip=0;
  for(int i=0; i<nStrip; i++) { // Store the strip signal
    chip = (int) i/nStripChip;
    if(xBadChannel[i]==0) SiStripxSignal[i] = ((double) data[0][i]) - xPed[i] -xCMN[chip];
    SiStripxSignal[i] = abs(SiStripxSignal[i]);
    if(yBadChannel[i]==0) SiStripySignal[i] = ((double) data[1][i]) - yPed[i] -yCMN[chip];
    SiStripySignal[i] = abs(SiStripySignal[i]);
    //cout<< "xstrip " << i << "   bad "<< xBadChannel[i] << "   signal " << SiStripxSignal[i] << "   ped "<< xPed[i] << "   sigma " << xSigma[i] << " xCMN " << xCMN[chip] << endl;
    //cout<< "ystrip " << i << "   bad "<< yBadChannel[i] << "   signal " << SiStripySignal[i] << "   ped "<< yPed[i] << "   sigma " << ySigma[i] << " yCMN " << yCMN[chip] << endl;
    //if(nSiStripEvent==2) FillSignal(SiStripySignal);
  }

  nxCluster=0; nyCluster=0;
  double xSeedCut=15, ySeedCut=10;
  xSeedSignal.clear(); xSeedMap.clear();
  ySeedSignal.clear(); ySeedMap.clear();
  for(int i=0; i<nStrip; i++) {  // Search for seeds
    if(xBadChannel[i]==0 && SiStripxSignal[i]/xSigma[i]>xSeedCut) {
       xSeedSignal.push_back(SiStripxSignal[i]);
       xSeedMap.insert(std::make_pair(i,SiStripxSignal[i]));
    }
    if(yBadChannel[i]==0 && SiStripySignal[i]/ySigma[i]>ySeedCut) {
       ySeedSignal.push_back(SiStripySignal[i]);
       ySeedMap.insert(std::make_pair(i,SiStripySignal[i]));
    }
  }

  if(xSeedSignal.size() > 1) { // Make clusters in x view
    std::sort(xSeedSignal.rbegin(), xSeedSignal.rend());
    for(unsigned i=0; i<xSeedSignal.size(); i++){
      cout << "xSeedSignal " << i << "  " << xSeedSignal.at(i) << endl;
      MakeXStripCluster(i);
    }
  }
  else if(xSeedSignal.size()==1) MakeXStripCluster(0);

//////////////////////////////////////////////////////////////////////
// TEST - POSITION RECONSTRUCTION
/////////////////////////////////////////////////////////////////////
  double xPitch=50, yPitch=67;
  int left,right;
  double sumCharge, sumChargePos, xPos, yPos;
  for(int k=0; k<nxCluster; k++) { // Flag as bad the clusters where any of the two strips close to the seed are bad
    if(xClusterSignal[k][3]==0 || xClusterSignal[k][4]==0 || xClusterSignal[k][6]==0 || xClusterSignal[k][7]==0) {
      isBadxCluster[k]=1; 
      continue;
    }

    sumCharge=0.; sumChargePos=0.;
    for(int kk=0; kk<11; kk++) { // Position using the Center of Gravity algorithm
      if(xClusterSignal[k][kk]>0) {
        xPos = xClusterSeed[k]*xPitch + (kk-5)*xPitch ;
        sumChargePos += abs(xClusterSignal[k][kk])*xPos;
        sumCharge += abs(xClusterSignal[k][kk]);
      }
    }
    xCOG[k]=sumChargePos/sumCharge;

    left=4; right=5; xPos=(xClusterSeed[k]-1)*xPitch;  // Position using the Linear eta algorithm
    if(abs(xClusterSignal[k][4]) < abs(xClusterSignal[k][6])) {left=5; right=6; xPos=xClusterSeed[k]*xPitch;}
    etaX[k] = abs(xClusterSignal[k][right])/(abs(xClusterSignal[k][right])+abs(xClusterSignal[k][left]));
    xLinEta[k] = xPos + etaX[k]*xPitch; 
  }

  for(int k=0; k<nxCluster; k++) {
    cout << "xClusterSignal " << k << "    isBadxCluster " << isBadxCluster[k] << endl; 
    for(int kk=0; kk<11; kk++) {
      cout << xClusterSignal[k][kk] << " ";
    }
    cout << endl;
    cout << "xCOG " << xCOG[k] << "   etaX " << etaX[k] << "   xLinEta " << xLinEta[k] << endl; 
  }


// ====================================================== y view here ==========================================
  if(ySeedSignal.size() > 1) { // Make clusters in y view
    std::sort(ySeedSignal.rbegin(), ySeedSignal.rend());
    for(unsigned i=0; i<ySeedSignal.size(); i++){
      cout << "ySeedSignal " << i << "  " << ySeedSignal.at(i) << endl;
      MakeYStripCluster(i);
    }
  }
  else if(ySeedSignal.size()==1) MakeYStripCluster(0);

  for(int k=0; k<nyCluster; k++) { // Flag as bad the clusters where any of the two strips close to the seed are bad
    if(yClusterSignal[k][3]==0 || yClusterSignal[k][4]==0 || yClusterSignal[k][6]==0 || yClusterSignal[k][7]==0) {
      isBadyCluster[k]=1; 
      continue;
    }

    sumCharge=0.; sumChargePos=0.;
    for(int kk=0; kk<11; kk++) { // Position using the Center of Gravity algorithm
      if(yClusterSignal[k][kk]>0) {
        yPos = yClusterSeed[k]*yPitch + (kk-5)*yPitch ;
        sumChargePos += abs(yClusterSignal[k][kk])*yPos;
        sumCharge += abs(yClusterSignal[k][kk]);
      }
    }
    yCOG[k]=sumChargePos/sumCharge;

    left=4; right=5; yPos=(yClusterSeed[k]-1)*yPitch;  // Position using the Linear eta algorithm
    if(abs(yClusterSignal[k][4]) < abs(yClusterSignal[k][6])) {left=5; right=6; yPos=yClusterSeed[k]*yPitch;}
    etaY[k] = abs(yClusterSignal[k][right])/(abs(yClusterSignal[k][right])+abs(yClusterSignal[k][left]));
    yLinEta[k] = yPos + etaY[k]*yPitch; 
  }

  for(int k=0; k<nyCluster; k++) {
    cout << "yClusterSignal " << k << "    isBadyCluster " << isBadyCluster[k] << endl; 
    for(int kk=0; kk<11; kk++) {
      cout << yClusterSignal[k][kk] << " ";
    }
    cout << endl;
    cout << "yCOG " << yCOG[k] << "   etaY " << etaY[k] << "   yLinEta " << yLinEta[k] << endl; 
  }

//////////////////////////////////////////////////////////////////////////////////////


}


///////////////////////////////////////////////////////////////////////////////
void CSPECTree::MakeXStripCluster(int i) {
///////////////////////////////////////////////////////////////////////////////
  int strip=-1;
  double ClusterIncl=3;

  std::map<int,double>::iterator it = xSeedMap.begin();
  while(it != xSeedMap.end()) {  // Retrive from the map the seed strip number
    //std::cout<<it->first<<" :: "<<it->second<<std::endl;
    if(it->second == xSeedSignal.at(i)) strip=it->first;
    it++;
  }

  cout << " MakeXStripCluster " << i << "   strip " << strip << endl;

  int minStrip,maxStrip;
  bool isIncluded=false;
  if(strip>0 && nxCluster) { // Check if the seed has already been used to form a cluster
    for(int j=0; j<nxCluster; j++) {    
      minStrip=max(0,xClusterSeed[j]-5);  maxStrip=min(1023,xClusterSeed[j]+5);
      if(strip<maxStrip && strip>=minStrip) isIncluded=true;
      cout << "min " << minStrip << "   max " << maxStrip << "   isIncluded " << isIncluded << endl;
    }
  }

  if(strip>0 && !isIncluded) { // 
    for(int j=max(0,strip-5); j<=min(1023,strip+5); j++)  {
      //cout << " looping on neighbours: " << j << "  " << SiStripxSignal[j] << "  " << SiStripxSignal[j]/xSigma[j] << endl; 
      if(xBadChannel[j]==1) xClusterSignal[nxCluster][j-strip+5] = 0.;
      else if(SiStripxSignal[j]/xSigma[j]>ClusterIncl) xClusterSignal[nxCluster][j-strip+5] = SiStripxSignal[j];
      else xClusterSignal[nxCluster][j-strip+5] = -SiStripxSignal[j];
      //cout << "xClusterSignal" << nxCluster << " " << j-strip+5 << "  " << xClusterSignal[nxCluster][j-strip+5] << endl;
    }
    xClusterSeed[nxCluster]=strip;
    nxCluster++;
  }
  
}

///////////////////////////////////////////////////////////////////////////////
void CSPECTree::MakeYStripCluster(int i) {
///////////////////////////////////////////////////////////////////////////////
  int strip=-1;
  double ClusterIncl=3;

  std::map<int,double>::iterator it = ySeedMap.begin();
  while(it != ySeedMap.end()) {  // Retrive from the map the seed strip number
    //std::cout<<it->first<<" :: "<<it->second<<std::endl;
    if(it->second == ySeedSignal.at(i)) strip=it->first;
    it++;
  }

  cout << " MakeYStripCluster " << i << "   strip " << strip << endl;

  int minStrip,maxStrip;
  bool isIncluded=false;
  if(strip>0 && nyCluster) { // Check if the seed has already been used to form a cluster
    for(int j=0; j<nyCluster; j++) {    
      minStrip=max(0,yClusterSeed[j]-5);  maxStrip=min(1023,yClusterSeed[j]+5);
      if(strip<maxStrip && strip>=minStrip) isIncluded=true;
      cout << "min " << minStrip << "   max " << maxStrip << "   isIncluded " << isIncluded << endl;
    }
  }

  if(strip>0 && !isIncluded) { // 
    for(int j=max(0,strip-5); j<=min(1023,strip+5); j++)  {
      //cout << " looping on neighbours: " << j << "  " << SiStripySignal[j] << "  " << SiStripySignal[j]/ySigma[j] << endl; 
      if(yBadChannel[j]==1) yClusterSignal[nyCluster][j-strip+5] = 0.;
      else if(SiStripySignal[j]/ySigma[j]>ClusterIncl) yClusterSignal[nyCluster][j-strip+5] = SiStripySignal[j];
      else yClusterSignal[nyCluster][j-strip+5] = -SiStripySignal[j];
      //cout << "yClusterSignal" << nyCluster << " " << j-strip+5 << "  " << yClusterSignal[nyCluster][j-strip+5] << endl;
    }
    yClusterSeed[nyCluster]=strip;
    nyCluster++;
  }
  
}


///////////////////////////////////////////////////////////////////////////////
void CSPECTree::InitSiStrip() {
///////////////////////////////////////////////////////////////////////////////

  char filename[256];
  sprintf(filename,"SiStripPedestal.root");
  cout << "\n******************************"  << endl;
  cout << "*** SiStrip initialization"  << endl;
  cout << "******************************"  << endl;
  cout << "Reading SiStrip pedestal file: " << filename  << endl;
  TFile* siPed = new TFile(filename);

  TH1F* pedestalData = (TH1F*)siPed->Get("pedestalDataFile"); Hlist->Add(pedestalData); 

  TH1F* nloop = (TH1F*)siPed->Get("nLoop"); Hlist->Add(nloop); 
  int npass=0;
  for(int i=0; i<=nloop->GetNbinsX(); i++) {
    if(nloop->GetBinContent(i)) npass=i-1;
    //cout << i << "  BinContent " << nloop->GetBinContent(i) << "  nPass " << npass << endl;
  }
  //cout << "  nPass " << npass << endl;

  char hname[256];        
  sprintf(hname,"xPed%d",npass);
  xped = (TH1F*)siPed->Get(hname);   Hlist->Add(xped);

  sprintf(hname,"xSigma%d",npass);
  xsigma = (TH1F*)siPed->Get(hname); Hlist->Add(xsigma);

  sprintf(hname,"yPed%d",npass);
  yped = (TH1F*)siPed->Get(hname);   Hlist->Add(yped);

  sprintf(hname,"ySigma%d",npass);
  ysigma = (TH1F*)siPed->Get(hname); Hlist->Add(ysigma);

  sprintf(hname,"xBadChannel%d",npass);
  xbad = (TH1F*)siPed->Get(hname);   Hlist->Add(xbad);

  sprintf(hname,"yBadChannel%d",npass);
  ybad = (TH1F*)siPed->Get(hname);   Hlist->Add(ybad);
 
  for(int i=0; i<nStrip; i++) { 
    xPed[i]=xped->GetBinContent(i+1);
    xSigma[i]=xsigma->GetBinContent(i+1);
    xBadChannel[i]=xbad->GetBinContent(i+1);
    //cout << i<< "  xPed " << xPed[i] << "    xSigma " << xSigma[i] << "   badCh " << xBadChannel[i] << endl;

    yPed[i]=yped->GetBinContent(i+1);
    ySigma[i]=ysigma->GetBinContent(i+1); 
    yBadChannel[i]=ybad->GetBinContent(i+1);
    //cout << i<< "  yPed " << yPed[i] << "  ySigma " << ySigma[i] << "   badCh " << yBadChannel[i] << endl;
  }

  cout << "******************************"  << endl;

  //siPed->Close();
}

///////////////////////////////////////////////////////////////////////////////
void CSPECTree::StripCommonNoise(uint16_t data[2][nStrip]) {
///////////////////////////////////////////////////////////////////////////////

 int chip=0;
 for(int c=0; c<nChip; c++) {
   xCMN[c]=0.; yCMN[c]=0.; nx[c]=0; ny[c]=0;
 }

 for(int i=0; i<nStrip; i++) {
   chip = (int) i/nStripChip;
   if(xBadChannel[i]==0) {
     xCMN[chip] += ((double) data[0][i]) - xPed[i];
     nx[chip]++;
   }
   if(yBadChannel[i]==0) {
     yCMN[chip] += ((double) data[1][i]) - yPed[i];
     ny[chip]++;
   }
 }
    
 for(int c=0; c<nChip; c++) {
   xCMN[c] /= nx[c];
   yCMN[c] /= ny[c];
   //printf("Chip %i   nx %i   xCommonNoise %f    ny %i   yCommonNoise %f \n",c,nx[c],xCMN[c],ny[c],yCMN[c]);
 }

 cout << endl;

}



///////////////////////////////////////////////////////////////
void  CSPECTree::FillTree() {
///////////////////////////////////////////////////////////////
  myTree->Fill();
}


///////////////////////////////////////////////////////////////////
Double_t CSPECTree::BaFCommonMode(Double_t data[], Int_t MaxChannel ) {
///////////////////////////////////////////////////////////////////
   Double_t CommonMode=0;
   Double_t SumSamples=0;
   for(Int_t j=0; j<MaxChannel; j++) {
     SumSamples+=data[j];
   }

   CommonMode=SumSamples/MaxChannel;

   return CommonMode;
}



///////////////////////////////////////////////////////////////
//  Reset BaF variables
///////////////////////////////////////////////////////////////
void  CSPECTree::ResetBaF() {

  for(Int_t i=0; i<nChannels; i++) {  //loop su canali
    CommonMode[i]=0;
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
    FracPN[i]=0;
    TriggerTime[i]=0;
    RiseTime[i]=0;
    NPos[i]=0;
    Hit[i]=0;
  }

  BaFTimeStamp=0.;
  BaFStatus=0;

  SumTot=0;
  SumNotHit=0;
  MaxValueTot=0;
  
  SumSignal = 0;
  SumSignalCalib = 0;
  
  SumSignalGate = 0;
  SumSignalCalibGate = 0;

  ClusterSize=0;

}


///////////////////////////////////////////////////////////////
//  Reset HPGe variables
///////////////////////////////////////////////////////////////
void  CSPECTree::ResetHPGe() {
  HPGeEnergyCh0=0.;
  HPGeEnergyCh1=0.;
  HPGeTimeStamp=0.;
  HPGeCommonMode=0.;
  HPGexMax=0.;
  HPGeStatus=0;
}


///////////////////////////////////////////////////////////////
//  Reset SiStrip variables
///////////////////////////////////////////////////////////////
void  CSPECTree::ResetSiStrip() {
  SiStripTimeStamp=0.;
  SiStripStatus=0;

  nxCluster=0;
  nyCluster=0;

  for(int i=0; i<nStrip; i++) {
    SiStripxSignal[i]=0;
    SiStripySignal[i]=0;
    xClusterSeed[i]=0;
    yClusterSeed[i]=0;
    for(int j=0; j<11; j++) { xClusterSignal[i][j]=0.; yClusterSignal[i][j]=0.; }
    isBadxCluster[i]=0;
    xCOG[i]=0.; 
    etaX[i]=0.;
    xLinEta[i]=0.;
    isBadyCluster[i]=0;
    yCOG[i]=0.; 
    etaY[i]=0.;
    yLinEta[i]=0.;
  }

}



///////////////////////////////////////////////////////////////
void  CSPECTree::CreateTree(TString rootfile) {
///////////////////////////////////////////////////////////////
  //cout << "CSPECTree:: Creating new tree" << endl;
  f=new TFile(rootfile,"recreate");

  myTree=new TTree("cspec","cspec");

  Int_t nChans = nChannels;
  TString s;
  s.Form("[%i]",nChans);
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
  myTree->Branch("BaFTimeStamp",&BaFTimeStamp,"BaFTimeStamp/D");
  myTree->Branch("BaFStatus",&BaFStatus,"BaFStatus/i");

  s.Form("[%i]",nChip);
  myTree->Branch("nSiStripEvent",&nSiStripEvent,"nSiStripEvent/I");
  myTree->Branch("xCMN",xCMN,"xCMN"+s+"/D");
  myTree->Branch("yCMN",yCMN,"yCMN"+s+"/D");
  myTree->Branch("SiStripTimeStamp",&SiStripTimeStamp,"SiStripTimeStamp/D");
  myTree->Branch("SiStripStatus",&SiStripStatus,"SiStripStatus/i");
  myTree->Branch("nxCluster",&nxCluster,"nxCluster/I");
  myTree->Branch("xClusterSeed",xClusterSeed,"xClusterSeed[nxCluster]/I");
  myTree->Branch("xClusterSignal",xClusterSignal,"xClusterSignal[nxCluster][11]/D");
  myTree->Branch("nyCluster",&nyCluster,"nyCluster/I");
  myTree->Branch("yClusterSeed",yClusterSeed,"yClusterSeed[nyCluster]/I");
  myTree->Branch("yClusterSignal",yClusterSignal,"yClusterSignal[nyCluster][11]/D");

  myTree->Branch("HPGeEnergyCh0",&HPGeEnergyCh0,"HPGeEnergyCh0/D");
  myTree->Branch("HPGeEnergyCh1",&HPGeEnergyCh1,"HPGeEnergyCh1/D");
  myTree->Branch("HPGeTimeStamp",&HPGeTimeStamp,"HPGeTimeStamp/D");
  myTree->Branch("HPGeStatus",&HPGeStatus,"HPGeStatus/i");
  myTree->Branch("HPGeCommonMode",&HPGeCommonMode,"HPGeCommonMode/D");
  myTree->Branch("HPGexMax",&HPGexMax,"HPGexMax/D ");

  Hlist = new TObjArray();

  Signal = new TH1F("Signal","Signal",nStrip,-0.5,1023.5);
  Signal->Sumw2();  Hlist->Add(Signal);
}


//-----------------------------------------------------------------------------------
void CSPECTree::switchOnBranches(TString detector) {
//-----------------------------------------------------------------------------------
  if(detector=="BaF") {
    myTree->SetBranchStatus("SumTot", 1);
    myTree->SetBranchStatus("SumNotHit", 1);
    myTree->SetBranchStatus("BinSum", 1);
    myTree->SetBranchStatus("BinSumCalib", 1);
    myTree->SetBranchStatus("BinSumGate", 1);
    myTree->SetBranchStatus("BinSumCalibGate", 1);
    myTree->SetBranchStatus("BinSumCalibFast", 1);
    myTree->SetBranchStatus("BinSumFast", 1);
    myTree->SetBranchStatus("MaxValue", 1);
    myTree->SetBranchStatus("SlowAverage", 1);
    myTree->SetBranchStatus("MaxValueTot", 1);
    myTree->SetBranchStatus("MaxX", 1);
    myTree->SetBranchStatus("NEvent", 1);
    myTree->SetBranchStatus("FirstX", 1);
    myTree->SetBranchStatus("SumSignal", 1);
    myTree->SetBranchStatus("SumSignalCalib", 1);
    myTree->SetBranchStatus("SumSignalGate", 1);
    myTree->SetBranchStatus("SumSignalCalibGate", 1);
    myTree->SetBranchStatus("ClusterSize", 1);
    myTree->SetBranchStatus("NPos", 1);
    myTree->SetBranchStatus("Hit", 1);
    myTree->SetBranchStatus("FracPN", 1);
    myTree->SetBranchStatus("TriggerTime", 1);
    myTree->SetBranchStatus("RiseTime", 1);
    myTree->SetBranchStatus("Slope", 1);
    myTree->SetBranchStatus("CommonMode", 1);
    myTree->SetBranchStatus("BaFTimeStamp", 1);
    myTree->SetBranchStatus("BaFStatus", 1);
  }

  if(detector=="SiStrip") {
    myTree->SetBranchStatus("nSiStripEvent", 1);
    myTree->SetBranchStatus("xCMN", 1);
    myTree->SetBranchStatus("yCMN", 1);
    myTree->SetBranchStatus("SiStripTimeStamp", 1);
    myTree->SetBranchStatus("SiStripStatus", 1);
    myTree->SetBranchStatus("nxCluster", 1);
    myTree->SetBranchStatus("xClusterSeed", 1);
    myTree->SetBranchStatus("xClusterSignal", 1);
    myTree->SetBranchStatus("nyCluster", 1);
    myTree->SetBranchStatus("yClusterSeed", 1);
    myTree->SetBranchStatus("yClusterSignal", 1);
  }

  if(detector=="HPGe") {
    myTree->SetBranchStatus("HPGeEnergyCh0", 1);
    myTree->SetBranchStatus("HPGeEnergyCh1", 1);
    myTree->SetBranchStatus("HPGeTimeStamp", 1);
    myTree->SetBranchStatus("HPGeStatus", 1);
    myTree->SetBranchStatus("HPGeCommonMode", 1);
    myTree->SetBranchStatus("HPGexMax", 1);
  }
 

}

//-----------------------------------------------------------------------------------
void CSPECTree::FillSignal(double signal[nStrip]) {
//-----------------------------------------------------------------------------------
  for (Int_t i=0;i<nStrip;i++) {
    Signal->SetBinContent(i+1,signal[i]);
  }
}
