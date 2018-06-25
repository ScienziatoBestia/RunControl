#ifndef SiPedestal_h
#define SiPedestal_h
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
#include <TF1.h>
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


#define nstrip 1024
#define nchip 8
#define nstripChip 128
#define npass 10

using namespace std;


class SiPedestal {

public :
   SiPedestal(TString dataFile, TString rootfile);
   virtual ~SiPedestal();
   virtual void FillRawData(uint16_t data[2048]);
   virtual void FillPedestal(double pedx[nstrip], double pedy[nstrip], int);
   virtual void FillSigma(double sx[nstrip], double sy[nstrip], int);
   virtual void FillBadChannel(int xbc[nstrip], int ybc[nstrip], int);
   virtual void FillCommonNoise(double xcm[nchip], double ycm[nchip], int);
   virtual void FillnLoop(int);
   double GetNoiseCut(int,int,int,double,double);

 private:
   double Pedx[nstrip], Pedy[nstrip];
   double Sigmax[nstrip], Sigmay[nstrip];

   TFile* f;
   TObjArray* Hlist;

   TH1F* nLoop;
   TH1F* pedestalFilename;

   TH1F* xRawData[nstrip];
   TH1F* xPed[npass]; 
   TH1F* xSigma[npass];
   TH1F* xBadChannel[npass]; 
   TH1F* xsigmaChip[npass][nchip];
   TH1F* xcmChip[npass][nchip];

   TH1F* yRawData[nstrip];
   TH1F* yPed[npass];
   TH1F* ySigma[npass];
   TH1F* yBadChannel[npass]; 
   TH1F* ysigmaChip[npass][nchip];
   TH1F* ycmChip[npass][nchip];
};


#endif


//---------------------------------------------------------------------
SiPedestal::SiPedestal(TString dataFile, TString rootfile) {
//---------------------------------------------------------------------

  // Create a new ROOT file
  f = new TFile(rootfile, "RECREATE");
  if( f->IsOpen() ) cout << rootfile << " was opened successfully" << endl;

  Hlist = new TObjArray();

  TH1F* pedestalData = new TH1F("pedestalDataFile",dataFile,1,-0.5,0.5);
  Hlist->Add(pedestalData);

  nLoop = new TH1F("nLoop","N of iteration on data",npass,-0.5,npass-0.5);
  nLoop->Sumw2();  Hlist->Add(nLoop);

  char hname[256];
  char htitle[256];
   
  for (Int_t i=0;i<nstrip;i++) {
    sprintf(hname, "xRawData%d",i);
    sprintf(htitle, "RawData xSide Ch%d",i);
    xRawData[i] = new TH1F(hname,htitle,400,0,4000);
    xRawData[i]->Sumw2();  Hlist->Add(xRawData[i]);

    sprintf(hname, "yRawData%d",i);
    sprintf(htitle, "RawData ySide Ch%d",i);
    yRawData[i] = new TH1F(hname,htitle,400,0,4000);
    yRawData[i]->Sumw2();  Hlist->Add(yRawData[i]);
  }


  for (Int_t np=0;np<npass;np++) {
    sprintf(hname, "xPed%d",np);
    sprintf(htitle, "nPass%d  xSide Pedestal",np);
    xPed[np] = new TH1F(hname,htitle,nstrip,-0.5,1023.5);
    xPed[np]->Sumw2();  Hlist->Add(xPed[np]);

    sprintf(hname, "yPed%d",np);
    sprintf(htitle, "nPass%d  ySide Pedestal",np);
    yPed[np] = new TH1F(hname,htitle,nstrip,-0.5,1023.5);
    yPed[np]->Sumw2();  Hlist->Add(yPed[np]);

    sprintf(hname, "xSigma%d",np);
    sprintf(htitle, "nPass%d  xSide #sigma",np);
    xSigma[np] = new TH1F(hname,htitle,nstrip,-0.5,1023.5);
    xSigma[np]->Sumw2();  Hlist->Add(xSigma[np]);

    sprintf(hname, "ySigma%d",np);
    sprintf(htitle, "nPass%d  ySide #sigma",np);
    ySigma[np] = new TH1F(hname,htitle,nstrip,-0.5,1023.5);
    ySigma[np]->Sumw2();  Hlist->Add(ySigma[np]);

    sprintf(hname, "xBadChannel%d",np);
    sprintf(htitle, "nPass%d  xBadChannel",np);
    xBadChannel[np] = new TH1F(hname,htitle,nstrip,-0.5,1023.5);
    xBadChannel[np]->Sumw2();  Hlist->Add(xBadChannel[np]);

    sprintf(hname, "yBadChannel%d",np);
    sprintf(htitle, "nPass%d  yBadChannel",np);
    yBadChannel[np] = new TH1F(hname,htitle,nstrip,-0.5,1023.5);
    yBadChannel[np]->Sumw2();  Hlist->Add(yBadChannel[np]);

    for (Int_t c=0;c<nchip;c++) {
      sprintf(hname, "xsigma%dChip%d",np,c);
      sprintf(htitle, "nPass%d  #sigma xSide Chip%d",np,c);
      xsigmaChip[np][c] = new TH1F(hname,htitle,240,0,80);
      xsigmaChip[np][c]->Sumw2();  Hlist->Add(xsigmaChip[np][c]);

      sprintf(hname, "ysigma%dChip%d",np,c);
      sprintf(htitle, "nPass%d  #sigma ySide Chip%d",np,c);
      ysigmaChip[np][c] = new TH1F(hname,htitle,240,0,80);
      ysigmaChip[np][c]->Sumw2();  Hlist->Add(ysigmaChip[np][c]);

      sprintf(hname, "xcm%dChip%d",np,c);
      sprintf(htitle, "nPass%d   x CommonMode Chip%d",np,c);
      xcmChip[np][c] = new TH1F(hname,htitle,300,-300,300);
      xcmChip[np][c]->Sumw2();  Hlist->Add(xcmChip[np][c]);

      sprintf(hname, "ycm%dChip%d",np,c);
      sprintf(htitle, "nPass%d   y CommonMode Chip%d",np,c);
      ycmChip[np][c] = new TH1F(hname,htitle,300,-300,300);
      ycmChip[np][c]->Sumw2();  Hlist->Add(ycmChip[np][c]);
    }
  }

}




//---------------------------------------------------------------------
SiPedestal::~SiPedestal() {
//---------------------------------------------------------------------
  f->cd();
  Hlist->Write();

  //f->Write();
  f->Close();
}




//------------------------------------------------------------
void SiPedestal::FillRawData(uint16_t data[2048]) {
//------------------------------------------------------------
  for (Int_t i=0;i<nstrip;i++) {
     xRawData[i]->Fill((float) data[i*2]);
     yRawData[i]->Fill((float) data[i*2+1]);
     if(data[i*2]>4000 || data[i*2+1]>4000) cout << "nstrip " << i << "   x " << data[i*2] << "   y " << data[i*2+1] <<endl;
  }
}


//-----------------------------------------------------------------------------------
void SiPedestal::FillPedestal(double pedx[nstrip], double pedy[nstrip], int pass) {
//-----------------------------------------------------------------------------------
  for (Int_t i=0;i<nstrip;i++) {
    xPed[pass]->SetBinContent(i+1,pedx[i]);
    yPed[pass]->SetBinContent(i+1,pedy[i]);
  }

}


//----------------------------------------------------------------------------
void SiPedestal::FillSigma(double sx[nstrip], double sy[nstrip], int pass) {
//----------------------------------------------------------------------------

  for (Int_t i=0;i<nstrip;i++) {
    xSigma[pass]->SetBinContent(i+1,sx[i]);
    ySigma[pass]->SetBinContent(i+1,sy[i]);
  }

  int start,end;
  for(int j=0; j<nchip; j++) { //Loop on chips
    start = j*nstripChip;
    end = start+nstripChip;
    for(int k=start; k<end; k++) { //Loop on strips
      xsigmaChip[pass][j]->Fill(sx[k]);
      ysigmaChip[pass][j]->Fill(sy[k]);
    }
  }


}


//-----------------------------------------------------------------------------------
void SiPedestal::FillCommonNoise(double xcm[nchip], double ycm[nchip], int pass) {
//-----------------------------------------------------------------------------------

  for (Int_t i=0;i<nchip;i++) {
    xcmChip[pass][i]->Fill((float) xcm[i]);
    ycmChip[pass][i]->Fill((float) ycm[i]);
  }
}


//-------------------------------------------------------------------------
void SiPedestal::FillBadChannel(int xbc[nstrip], int ybc[nstrip],int pass) {
//-------------------------------------------------------------------------

  for (Int_t i=0;i<nstrip;i++) {
    xBadChannel[pass]->SetBinContent(i+1,xbc[i]);
    yBadChannel[pass]->SetBinContent(i+1,ybc[i]);
  }
}


//-------------------------------------------------------------------------
void SiPedestal::FillnLoop(int pass) {
//-------------------------------------------------------------------------
  nLoop->Fill((float)pass);
}


//--------------------------------------------------------------------------------------
double SiPedestal::GetNoiseCut(int xSide, int np, int chip, double mean, double sigma) {
//--------------------------------------------------------------------------------------
   double lowLimit;
   if((mean-2.5*sigma)<0) lowLimit=2.;
   else lowLimit=mean-2.5*sigma;
   //lowLimit=2;
   cout << endl << "pass " << np << "   chip " << chip << "   lowLimit " << lowLimit << "   upperLimit " << mean+2.5*sigma << endl;

   TF1* myFit;
   myFit = new TF1("myFit", "gaus", lowLimit, mean+2.5*sigma);
   myFit->SetParameter(0,50);
   myFit->SetParameter(1,mean);
   myFit->SetParameter(2,sigma);

   char hname[256];
   if(xSide) sprintf(hname, "xsigma%dChip%d",np,chip);
   else sprintf(hname, "ysigma%dChip%d",np,chip);
   TH1F* h = (TH1F*)gDirectory->Get(hname);
   Int_t status=0;
   status = h->Fit("myFit","R");
   //Try to recover in case of fit failure...status doesn't work...
   //if(myFit->GetParameter(0)<0 || myFit->GetParameter(1)<0 || status!=0) {  
   if(myFit->GetParameter(0)<0 || myFit->GetParameter(1)<0 || status!=0 || 
      myFit->GetParameter(2)>h->GetRMS() ) {  
     cout << endl << "*** SiPedestal::GetNoiseCut - Fit failure, trying to recover " << endl <<endl;
     delete myFit;
     myFit = new TF1("myFit", "gaus", lowLimit, mean+2.5*sigma);
     myFit->SetParameter(0,40);
     myFit->SetParLimits(0,0.,50);
     myFit->SetParameter(1,mean);
     myFit->SetParLimits(1,0.9*mean,1.1*mean); 
     myFit->SetParameter(2,sigma);
     status = h->Fit("myFit","R");
   }
   h->SetDirectory(0);

   cout << "pass " << np << "  chip " << chip << "  fit result " << myFit->GetParameter(0)<< "  " << myFit->GetParameter(1) << "   " << myFit->GetParameter(2) << endl;

   // Protect against bad or not converging fits 
   double cut;
   //if(myFit->GetParameter(0)<0 || myFit->GetParameter(1)<0 || status!=0) cut=sigma;
   if(myFit->GetParameter(0)<0 || myFit->GetParameter(1)<0 || status!=0 || myFit->GetParameter(2)>h->GetRMS()) cut=sigma;
   else cut=myFit->GetParameter(2);

   delete myFit;

   cout << "pass " << np << "  chip " << chip << "  cut " << cut << endl;

   return cut;
}
