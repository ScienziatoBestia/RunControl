#include "SiStripAna.h"

SiStripAna::SiStripAna(TString name) {

  gROOT->Reset();

  TString filename = "./"+name;
  cout << "\n Reading: " << filename << endl;

  TFile *f = new TFile(filename);
  f->GetObject("cspec",T);
   
  Init(T);
  T->SetBranchStatus("*", 0);
  switchOnBranches();
   
  Hlist = new TObjArray();
  HistoBook(f);

  Int_t DEBUG=0;

////////////////////////////////////////////////////////////////////////////////////////////////////////// 
// Main loop over all the events 
//////////////////////////////////////////////////////////////////////////////////////////////////////////
  Int_t nentries = (Int_t)T->GetEntries();
  cout << " The Tree contains " << nentries << " entries" << endl;

  for (Int_t n=0; n<nentries; n++) {
  //for (Int_t n=0; n<100; n++) {
   T->GetEntry(n);
   if(n%100 == 0) cout << " >>> Event " << n << endl;
   cout << " >>> Event " << n << endl;

   if(DEBUG) T->Show(n);

// -----  Common noise -----
   for (Int_t c=0; c<nChip; c++) {
     xCommonNoise[c]->Fill(xCMN[c]);
     yCommonNoise[c]->Fill(yCMN[c]);
   }

// -----  x view -----
   int nxBad=0, cluMult;
   double xcluEne, S2N;
   nxcluster->Fill(nxCluster);
   if(nxCluster>30) continue; //Protection against patological events
   for (Int_t i=0; i<nxCluster; i++) {
     if(xClusterSignal[i][3]==0 || xClusterSignal[i][4]==0 || xClusterSignal[i][6]==0 || xClusterSignal[i][7]==0) {
        nxBad++;
        continue;
     }
     xSeed->Fill(xClusterSeed[i]);
     xcluEne=0.; cluMult=0; S2N=0.;
     for (Int_t j=0; j<11; j++) {
       if(xClusterSignal[i][j]>0) {
         xcluEne+=xClusterSignal[i][j];
         S2N += xClusterSignal[i][j]/xSigma[xClusterSeed[i]+j-5];
         cluMult++; 
       }
     }
     xClusterEnergy->Fill(xcluEne);
     xClusterMultiplicity->Fill( (Float_t) cluMult);
     xClusterS2N->Fill(S2N);
   }
   nBadxCluster->Fill((Float_t) nxBad);

// -----  y view -----
   int nyBad=0;
   double ycluEne;
   nycluster->Fill(nyCluster);
   if(nyCluster>30) continue; //Protection against patological events
   for (Int_t i=0; i<nyCluster; i++) {
     if(yClusterSignal[i][3]==0 || yClusterSignal[i][4]==0 || yClusterSignal[i][6]==0 || yClusterSignal[i][7]==0) {
        nyBad++;
        continue;
     }
     ySeed->Fill(yClusterSeed[i]);
     ycluEne=0.; cluMult=0; S2N=0.;
     for (Int_t j=0; j<11; j++) {
       if(yClusterSignal[i][j]>0) {
         ycluEne+=yClusterSignal[i][j];
         S2N += yClusterSignal[i][j]/ySigma[yClusterSeed[i]+j-5];
         cluMult++; 
       }
     }
     yClusterEnergy->Fill(ycluEne);
     yClusterMultiplicity->Fill( (Float_t) cluMult);
     yClusterS2N->Fill(S2N);
   }
   nBadyCluster->Fill((Float_t) nyBad);

   if(nxCluster==1 && nxBad==0 && nyCluster==1 && nyBad==0) CorrClusterEnergy->Fill(xcluEne,ycluEne);


  } //end loop over entries 


  // Open a file and write the histo array to the file   
  TString Hfilename = "./Histo"+name;
  TFile* Hf = new TFile(Hfilename, "RECREATE");
  Hlist->Write();
  Hf->Close();
  cout << " Histos saved to: " << Hfilename << endl;

}


SiStripAna::~SiStripAna()  {
 // delete T;
}

void SiStripAna::HistoBook(TFile* treeFile) {
  char hname[100];
  char htitle[100];

  TH1F* pedestalData = (TH1F*)treeFile->Get("pedestalDataFile"); Hlist->Add(pedestalData);

  TH1F* nloop = (TH1F*)treeFile->Get("nLoop"); Hlist->Add(nloop);
  int npass=0;
  for(int i=0; i<=nloop->GetNbinsX(); i++) {
    if(nloop->GetBinContent(i)) npass=i-1;
  }

  sprintf(hname,"xSigma%d",npass);
  xsigma = (TH1F*)treeFile->Get(hname); Hlist->Add(xsigma);
  sprintf(hname,"ySigma%d",npass);
  ysigma = (TH1F*)treeFile->Get(hname); Hlist->Add(ysigma);
  for(int i=0; i<nStrip; i++) {
    //xPed[i]=xped->GetBinContent(i+1);
    xSigma[i]=xsigma->GetBinContent(i+1);
    //xBadChannel[i]=xbad->GetBinContent(i+1);  
    //yPed[i]=yped->GetBinContent(i+1);
    ySigma[i]=ysigma->GetBinContent(i+1);
    //yBadChannel[i]=ybad->GetBinContent(i+1);
  }
  

  for (Int_t c=0;c<nChip; c++) {
    sprintf(hname, "xCommonNoise%d",c);
    sprintf(htitle, "x CommonMode Chip%d",c);
    xCommonNoise[c] = new TH1F(hname,htitle,150,-300,300);
    xCommonNoise[c]->Sumw2();  Hlist->Add(xCommonNoise[c]);

    sprintf(hname, "yCommonNoise%d",c);
    sprintf(htitle, "y CommonMode Chip%d",c);
    yCommonNoise[c] = new TH1F(hname,htitle,150,-300,300);
    yCommonNoise[c]->Sumw2();  Hlist->Add(yCommonNoise[c]);
  }

  nxcluster = new TH1F("nxcluster","Number of cluster - x view",31,-0.5,30.5);
  nxcluster->Sumw2();  Hlist->Add(nxcluster);

  nBadxCluster = new TH1F("nBadxCluster","Number of BAD cluster - x view",15,-0.5,14.5);
  nBadxCluster->Sumw2();  Hlist->Add(nBadxCluster);
    
  xSeed = new TH1F("xSeed","Seed Strip - x view",nStrip,-0.5,1023.5);
  xSeed->Sumw2();  Hlist->Add(xSeed);

  xClusterEnergy = new TH1F("xClusterEnergy","Cluster Energy - x view",500,0.,1000.);
  xClusterEnergy->Sumw2();  Hlist->Add(xClusterEnergy);

  xClusterMultiplicity = new TH1F("xClusterMultiplicity","Cluster Multiplicity - x view",15,0.,15.);
  xClusterMultiplicity->Sumw2();  Hlist->Add(xClusterMultiplicity);

  xClusterS2N = new TH1F("xClusterS2N","Cluster Signal to Noise ratio - x view",100,0.,100.);
  xClusterS2N->Sumw2();  Hlist->Add(xClusterS2N);

  nycluster = new TH1F("nycluster","Number of cluster - y view",31,-0.5,30.5);
  nycluster->Sumw2();  Hlist->Add(nycluster);

  nBadyCluster = new TH1F("nBadyCluster","Number of BAD cluster - y view",15,-0.5,14.5);
  nBadyCluster->Sumw2();  Hlist->Add(nBadyCluster);
    
  ySeed = new TH1F("ySeed","Seed Strip - y view",nStrip,-0.5,1023.5);
  ySeed->Sumw2();  Hlist->Add(ySeed);

  yClusterEnergy = new TH1F("yClusterEnergy","Cluster Energy - y view",500,0.,1000.);
  yClusterEnergy->Sumw2();  Hlist->Add(yClusterEnergy);

  yClusterMultiplicity = new TH1F("yClusterMultiplicity","Cluster Multiplicity - y view",15,0.,15.);
  yClusterMultiplicity->Sumw2();  Hlist->Add(yClusterMultiplicity);

  yClusterS2N = new TH1F("yClusterS2N","Cluster Signal to Noise ratio - y view",100,0.,100.);
  yClusterS2N->Sumw2();  Hlist->Add(yClusterS2N);

  CorrClusterEnergy = new TH2F("CorrClusterEnergy","xy Cluster energy correlation ",250,0.,1000.,250,0.,1000.);
  CorrClusterEnergy->Sumw2();  Hlist->Add(CorrClusterEnergy);
}



void SiStripAna::Init(TTree *fChain) {

   if (!fChain) return;

   fChain->SetBranchAddress("SumTot", &SumTot, &b_SumTot);
   fChain->SetBranchAddress("SumNotHit", &SumNotHit, &b_SumNotHit);
   fChain->SetBranchAddress("BinSum", BinSum, &b_BinSum);
   fChain->SetBranchAddress("BinSumCalib", BinSumCalib, &b_BinSumCalib);
   fChain->SetBranchAddress("BinSumGate", BinSumGate, &b_BinSumGate);   
   fChain->SetBranchAddress("BinSumCalibGate", BinSumCalibGate, &b_BinSumCalibGate);
   fChain->SetBranchAddress("BinSumCalibFast", BinSumCalibFast, &b_BinSumCalibFast);
   fChain->SetBranchAddress("BinSumFast", BinSumFast, &b_BinSumFast);
   fChain->SetBranchAddress("MaxValue", MaxValue, &b_MaxValue);
   fChain->SetBranchAddress("SlowAverage", SlowAverage, &b_SlowAverage);
   fChain->SetBranchAddress("MaxValueTot", &MaxValueTot, &b_MaxValueTot);
   fChain->SetBranchAddress("MaxX", MaxX, &b_MaxX);
   fChain->SetBranchAddress("NEvent", NEvent, &b_NEvent);
   fChain->SetBranchAddress("FirstX", FirstX, &b_FirstX);
   fChain->SetBranchAddress("SumSignal", &SumSignal, &b_SumSignal);
   fChain->SetBranchAddress("SumSignalCalib", &SumSignalCalib, &b_SumSignalCalib);
   fChain->SetBranchAddress("SumSignalGate", &SumSignalGate, &b_SumSignalGate);   
   fChain->SetBranchAddress("SumSignalCalibGate", &SumSignalCalibGate, &b_SumSignalCalibGate);
   fChain->SetBranchAddress("ClusterSize", &ClusterSize, &b_ClusterSize);
   fChain->SetBranchAddress("NPos", NPos, &b_NPos);
   fChain->SetBranchAddress("Hit", Hit, &b_Hit);   
   fChain->SetBranchAddress("FracPN", FracPN, &b_FracPN);
   fChain->SetBranchAddress("TriggerTime", TriggerTime, &b_TriggerTime);
   fChain->SetBranchAddress("RiseTime", RiseTime, &b_RiseTime);
   fChain->SetBranchAddress("Slope", Slope, &b_Slope);
   fChain->SetBranchAddress("CommonMode", CommonMode, &b_CommonMode);
   fChain->SetBranchAddress("BaFTimeStamp", &BaFTimeStamp, &b_BaFTimeStamp);

   fChain->SetBranchAddress("nSiStripEvent", &nSiStripEvent, &b_nSiStripEvent);
   fChain->SetBranchAddress("xCMN", xCMN, &b_xCMN);
   fChain->SetBranchAddress("yCMN", yCMN, &b_yCMN);
   fChain->SetBranchAddress("SiStripTimeStamp", &SiStripTimeStamp, &b_SiStripTimeStamp);
   fChain->SetBranchAddress("nxCluster", &nxCluster, &b_nxCluster);
   fChain->SetBranchAddress("xClusterSeed", xClusterSeed, &b_xClusterSeed);
   fChain->SetBranchAddress("xClusterSignal", xClusterSignal, &b_xClusterSignal);
   fChain->SetBranchAddress("nyCluster", &nyCluster, &b_nyCluster);
   fChain->SetBranchAddress("yClusterSeed", yClusterSeed, &b_yClusterSeed);
   fChain->SetBranchAddress("yClusterSignal", yClusterSignal, &b_yClusterSignal);

   fChain->SetBranchAddress("HPGeEnergyCh0", &HPGeEnergyCh0, &b_HPGeEnergyCh0);  
   fChain->SetBranchAddress("HPGeEnergyCh1", &HPGeEnergyCh1, &b_HPGeEnergyCh1);  
   fChain->SetBranchAddress("HPGeTimeStamp", &HPGeTimeStamp, &b_HPGeTimeStamp);  
   fChain->SetBranchAddress("HPGeCommonMode", &HPGeCommonMode, &b_HPGeCommonMode);
   fChain->SetBranchAddress("HPGexMax", &HPGexMax, &b_HPGexMax);

}



void SiStripAna::switchOnBranches() {
   T->SetBranchStatus("nSiStripEvent",1);
   T->SetBranchStatus("xCMN",1);
   T->SetBranchStatus("yCMN",1);
   T->SetBranchStatus("SiStripTimeStamp",1);
   T->SetBranchStatus("nxCluster",1);  
   T->SetBranchStatus("xClusterSeed",1);
   T->SetBranchStatus("xClusterSignal",1);
   T->SetBranchStatus("nyCluster",1);
   T->SetBranchStatus("yClusterSeed",1);
   T->SetBranchStatus("yClusterSignal",1);
}
