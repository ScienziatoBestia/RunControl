double FuncGausPois(double *x, double *par) // sum of gaussians with integer step between mean values folded to a poissonian
{
  double result = 0;
  double anorm = par[0]; // normalization
  double amean;
  double lambda = par[1]; // poissonian mean value
  int ngaus = (int) par[2]; // number of gaussians
  double xshift = par[3];
  double scale = par[4];
  double asigm;  // gaussian sigma
  double t=x[0]-xshift; // the variable

  for(int i=0;i<ngaus;i++) {
    //    amean = par[4]+(i+1)*par[1];
    //    amean=par[5+ngaus+i];
    amean = (i+1)*scale;
    asigm = par[5+i];
    result += anorm*TMath::Gaus(t,amean,asigm)*TMath::Poisson(t/scale,lambda);
  }


  //  result = ROOT::Math::gaussian_pdf(t,asigm,amean);
  return result;
}

void FitNGaus(int nChannel, int nGauss, int firstX, int maxX) {
  int isMaxValue = 1; // 0 to obtain the energy spectrum from the 5ns area 
  const int npeak=50;
  double sigma,SingleProton, shift;
  int nbin=maxX;
  //int nbin=250;

  TH1F* pippo= new TH1F;
  pippo->Fit("gaus");
  gStyle->SetStatY(0.9);
  gStyle->SetStatX(0.9);

  if(isMaxValue) {
    sigma=5;
    SingleProton=20.5; // canali ADC
    shift=10;
  } else {
    sigma=50;
    SingleProton=200; // canali ADC
    shift=10;
    nbin=nbin/10.;
  }
  firstX=firstX-SingleProton;
  printf("firstX %i \n", firstX);
  TString hname;
  TString Param[3];
  TString ichan;
  Double_t Sigma[npeak],eSigma[npeak],Sigma2[npeak],eSigma2[npeak], Mean[npeak],eMean[npeak],  Mean2[npeak],eMean2[npeak],AreaGaus[npeak], Diff[npeak], xBin[npeak],energy[npeak], ex[npeak], ey[npeak], p0,p1,ep0, ep1,minFit,maxFit,Residui[npeak],eResidui[npeak];
  TAxis *Xaxis;
  TAxis *Yaxis; 
  ichan.Form("[%i]",nChannel);
  string GaussString;
  TString test;
  char GaussFormula[1000];
  Option_t* option;


  ///////////////////////////////////////  
  gStyle->SetOptFit(00001);
  gStyle->SetOptStat("imr");

  TCanvas *c1 = new TCanvas("c1","c1",600,600);
  TH1F* h[npeak];
  TCanvas *c8 = new TCanvas("c8","c8",750,500);
  TH1F * adc = new TH1F("adc","adc",300,20,23);
  //  c1->Divide(2,1);
  c1->cd();
  for(int i=0; i<nGauss;i++) {
    hname.Form("h%i",i);
    if(isMaxValue) h[i] = new TH1F(hname,"",nbin,0,maxX);
    else h[i] = new TH1F(hname,"",nbin,0,maxX);
    gaus->SetParameter(1,SingleProton*(i+1));
    gaus->SetParameter(2,sigma);
    gaus->SetParameter(0,500);

    if(i==0) {
      // m.v. 
      //if(isMaxValue) gcal->Draw("MaxValue"+ichan+">>"+hname,"abs(MaxX[8]-MaxX"+ichan+")-515<8 && abs(MaxX[8]-MaxX"+ichan+")-515>-8");
      if(isMaxValue) gcal->Draw("MaxValue"+ichan+">>"+hname,"abs(MaxX[8]-MaxX"+ichan+")-515<8 && abs(MaxX[8]-MaxX"+ichan+")-515>-8 && MaxValue"+ichan+"<25+BinSum5"+ichan+"*0.094");
      //if(isMaxValue) gcal->Draw("MaxValue"+ichan+">>"+hname,"abs(MaxX[8]-MaxX"+ichan+")-539<10 && abs(MaxX[8]-MaxX"+ichan+")-539>-10 && MaxValue"+ichan+"<25+BinSum5"+ichan+"*0.094");
      else gcal->Draw("BinSum5"+ichan+">>"+hname,"abs(MaxX[8]-MaxX"+ichan+")-515<8 && abs(MaxX[8]-MaxX"+ichan+")-515>-8");
      option = "";
    } else {
      h[i]->Add(h[0],1);
      option="same";
    }
      minFit=6;
      maxFit=6;
    if(!isMaxValue)  {
      minFit=1.5*sigma;
      maxFit=1.5*sigma;
    }
    //    h[i]->Fit("gaus","R",option,shift+(i+1)*SingleProton-minFit,shift+(i+1)*SingleProton+maxFit);
    h[i]->Fit("gaus","R",option,firstX+SingleProton-minFit,firstX+SingleProton+maxFit);

    printf("i %i mean fit %i \n",i,firstX+SingleProton);
    Mean[i]=gaus->GetParameter(1);
    //    SingleProton=Mean[i];
    firstX=Mean[i];
    eMean[i]=gaus->GetParError(1);
    Mean2[i]=(Mean[i])*(Mean[i]);;
    eMean2[i]=2*eMean[i]*Mean[i];
    Sigma[i]=gaus->GetParameter(2);
    eSigma[i]=gaus->GetParError(2);
    Sigma2[i]=(Sigma[i])*(Sigma[i]);
    eSigma2[i]=2*eSigma[i]*Sigma[i];
    //    AreaGaus[i]=Sigma[i]*2.5066*gaus->GetParameter(0);
    //    if(i!=0) sigma= Sigma[i];
    //    shift=Mean[i];

    if(i!=0) {
      Diff[i-1]=Mean[i]-Mean[i-1];
      adc->Fill(Diff[i-1]);
      //     printf("Diff %f \n",Diff[i-1]);
    }
    xBin[i]=i;
    energy[i]=(i+1)*3;
    //energy[i]=(i+1)*2.973;
    ex[i]=0;
  }	
  for(int i=0; i<nGauss;i++) {
    printf("i %i Mean %f Err %f ",i, Mean[i],eMean[i]);
    printf("i %i Sigma %f Sigma2 %f \n",i, Sigma[i],Sigma2[i]);
  }
  Xaxis = h0->GetXaxis();
  Xaxis->SetTitle("ADC counts");
  //  gStyle->SetOptStat("i");
  gStyle->SetStatH(0.2);
  gStyle->SetStatW(0.2);

  //  gStyle->SetOptStat(1000000);	
  gPad->Update();
  
  TCanvas *c2 = new TCanvas("c2","c2",600,600);
  gStyle->SetOptFit(00001);
  TGraphErrors *gr = new TGraphErrors (nGauss,Mean,Sigma,eMean,eSigma);
  gr->SetMarkerStyle(20);
  gr->SetMarkerSize(0.7);

  gr->Draw("AP");
  pol2->SetParameter(1,5);
  pol2->SetParLimits(1,0.,10.);
  gr->Fit("pol1","R","",30,maxX);
  Xaxis = gr->GetXaxis();
  Xaxis->SetTitle("Mean (ADC counts)");
  
  Yaxis = gr->GetYaxis();
  Yaxis->SetTitle("Sigma (ADC counts)");
  
  gr->SetTitle("");
  

  ////////////////////////////////////////
  //  c1->cd(2);
  //////
  c8->cd();
  adc->Draw();

  
  TCanvas *c4 = new TCanvas("c4","c4",600,600);

  TF1* FuncGausPois =new TF1("FuncGausPois",FuncGausPois,0,1000,5+nGauss);
  //  TF1* FuncGausPois =new TF1("FuncGausPois",FuncGausPois,0,1000,5+2*nGauss);

  FuncGausPois->SetParameter(0,5000); //norm
  FuncGausPois->SetParName(0,"Norm"); //norm

  //  FuncGausPois->SetParameter(1,20); //mean
  //  FuncGausPois->SetParName(1,"Mean"); //mean

  FuncGausPois->SetParameter(1,4);  //lambda
  FuncGausPois->SetParName(1,"Lambda"); //lambda

  FuncGausPois->FixParameter(2,nGauss);  //ngaus
  FuncGausPois->SetParName(2,"Ngaus"); //ngaus


  FuncGausPois->SetParameter(3,0);  //xshift
  FuncGausPois->SetParName(3,"XShift"); //xshift

  FuncGausPois->SetParameter(4,SingleProton);  //scale
  FuncGausPois->SetParName(4,"Scale"); //scale

  for(int i=0; i<nGauss; i++) {
    FuncGausPois->SetParameter(5+i,sigma);
    FuncGausPois->SetParLimits(5+i,0,15);
    FuncGausPois->SetParName(5+i,"Sigma");

    //    FuncGausPois->SetParameter(5+nGauss+i,(i+1)*SingleProton);
    //    FuncGausPois->SetParName(5+nGauss+i,"Mean");

  }

  FuncGausPois->SetNpx(10000);

  h0->Draw();
  h0->Fit("FuncGausPois","R","",10,SingleProton*nGauss+2*sigma+10);
  for(int i=0; i<nGauss;i++) {
    //    Mean[i]=FuncGausPois->GetParameter(3)+FuncGausPois->GetParameter(5+nGauss+i);
    //    Mean[i]=FuncGausPois->GetParameter(5+nGauss+i);
    //    eMean[i]=FuncGausPois->GetParError(5+nGauss+i);
    Mean2[i]=Mean[i]*Mean[i];
    eMean2[i]=2*Mean[i]*eMean[i];
    Sigma[i]=FuncGausPois->GetParameter(5+i);
    Sigma2[i]=(Sigma[i])*(Sigma[i]);
    eSigma2[i]=2*FuncGausPois->GetParError(5+i)*Sigma[i];
    eSigma[i]=FuncGausPois->GetParError(5+i);
  }
  for(int i=0; i<nGauss;i++) {
    printf("i %i Mean fit %f \n",i, Mean[i]);
  }
  ////////////////////////////////////////////////
  

  TCanvas *c3 = new TCanvas("c3","c3",1800,600);
  TH1F * adc = new TH1F("adc","adc",400,18,26);
  TH1F * HistoResidui = new TH1F("HistoResidui","HistoResidui",400,-5,5);
  c3->Divide(3,1);
  c3->cd(1);

  //  adc->Draw();
  TGraphErrors *gr2 = new TGraphErrors (nGauss,energy,Mean,ex,eMean);
  //  TGraph *gr2 = new TGraph (nGauss,energy,Mean);

  gr2->SetMarkerStyle(20);
  gr2->SetMarkerSize(0.7);
  gStyle->SetStatX(0.46);

  gr2->Draw("AP");
  gr2->Fit("pol1","R","",1,nGauss*3+1);
  
  Xaxis = gr2->GetXaxis();
  //  Xaxis->SetTitle("N peak");
  Xaxis->SetTitle("Energy (MeV)");
  Yaxis = gr2->GetYaxis();
  Yaxis->SetTitle("Mean (ADC counts)");
  
  gr2->SetTitle("Linearity");
  
  p0 = pol1->GetParameter(0);
  p1 = pol1->GetParameter(1);
  ep0= pol1->GetParError(0);
  ep1= pol1->GetParError(1);
  double maxres,minres;
  for(int i=0; i<nGauss;i++) {
    Residui[i]=p0+p1*energy[i]-Mean[i];
    maxres=(p0+ep0)+(p1+ep1)*energy[i]-(Mean[i]-eMean[i]);
    minres=(p0-ep0)+(p1-ep1)*energy[i]-(Mean[i]+eMean[i]);
    //    eResidui[i]=(maxres-minres)/2.0;
    eResidui[i]=sqrt(ep0*ep0+ep1*energy[i]*ep1*energy[i]+eMean[i]*eMean[i]);
    HistoResidui->Fill(Residui[i]);
    printf("i %i Residui %f %f \n", i, Residui[i],eResidui[i]);
    //    printf("Residui %i %f %f %f \n", i,p0+p1*i,Mean[i], p0+p1*i-Mean[i]);
  }
  c3->cd(3);
  HistoResidui->Draw();
  //  gStyle->SetStatX(0.9);

  Xaxis = HistoResidui->GetXaxis();
  Xaxis->SetTitle("ADC counts");
  c3->cd(2);
  //  TGraph *gr3 = new TGraph (nGauss,energy,Residui);
  //  TGraphErrors *gr3 = new TGraphErrors(nGauss,energy,Residui,ex,eResidui);
  //  TGraphErrors *gr3 = new TGraphErrors(nGauss,Mean,Residui,eMean,eResidui);
TGraphErrors *gr3 = new TGraphErrors(nGauss,Mean,Residui,eMean,ex);
  gr3->SetMarkerStyle(20);
  gr3->SetMarkerSize(0.7);

  gr3->Draw("AP");
 Xaxis = gr3->GetXaxis();
 //  Xaxis->SetTitle("Energy (MeV)");
    Xaxis->SetTitle("Mean (ADC counts)");

  Yaxis = gr3->GetYaxis();
  Yaxis->SetTitle("Residuals (ADC counts)");
  
  gr3->SetTitle("Residuals");
 
  ////////////////////////////////////////
  TCanvas *c6 = new TCanvas("c6","c6",750,500);
  gr2->Draw("AP");
  TCanvas *c7 = new TCanvas("c7","c7",750,500);
  gr3->Draw("AP");
  ////////////////////////////////////////

  /*
  c1->cd(2);

  TH1F* hPois = new TH1F("hPois","Poisson distribution",100,0,100);  
  TH1F* hNewPois = new TH1F("hNewPois","Poisson distribution",1000,0,1000);  
  TH1F* hPoisGaus = new TH1F("hPoisGaus","Poisson distribution",300,0,300);  

  TF1* pois=new TF1("pois",pois,0,100,2);

  pois->SetParameter(0,100);
  pois->SetParameter(1,nPois);
  hPois->FillRandom("pois",h0->GetEntries());

  Double_t NewBins[100];
  for(int i=0;i<100;i++) {
    NewBins[i]=hPois->GetBinContent(i+1);
  }
  for(int i=1;i<20;i++) {
    hNewPois->SetBinContent(i*p1,NewBins[i]);
    gaus->SetParameter(0,NewBins[i]);
    gaus->SetParameter(1,p0+(i-1)*p1);
    //    printf("i %i Sigma %f \n",i,Sigma[i-1]);
    if(i<nGauss) gaus->SetParameter(2,Sigma[i-1]);
    else gaus->SetParameter(2,Sigma[nGauss-1]);
    //    else gaus->SetParameter(2,5);
    if(NewBins[i]!=0) hPoisGaus->FillRandom("gaus",NewBins[i]);
  }
  h0->SetLineWidth(2);
  h0->Draw();
  Xaxis = h0->GetXaxis();
  Xaxis->SetTitle("ADC counts");
  hPoisGaus->SetLineWidth(2);
  hPoisGaus->SetLineColor(kRed);
  hPoisGaus->Draw("same");

  */
}
