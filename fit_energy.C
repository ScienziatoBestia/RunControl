#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <math.h>
#include <TFile.h>
#include <TROOT.h>
#include <TNtuple.h>
#include <TH1F.h>
#include <TH2F.h>
#include <Riostream.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <TGraph.h>
#include <TROOT.h>
#include <TMath.h>
#include <TCanvas.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stack>
#include <math.h>
#include <TFile.h>
#include <TNtuple.h>
#include <TH1F.h>
#include <TH2F.h>
#include <Riostream.h>
#include <TF1.h>
#include <TROOT.h>
#include <fstream> 
#include <sstream> 
#include <iostream>
#include <vector>
#include <TStyle.h>
#define PI 3.141592653

Double_t fit_func(Double_t *,Double_t *);
Double_t fit_expo(Double_t *,Double_t *);

void plotta(const char *nome=""){
 
  fstream file;
  file.open(nome);
  std::string line;
      
      if(!file.good()){cout << "Non riesco ad aprire il file " << nome<< endl;};
  int Ngr=9;    
 /* for(int i=0;i<5;i++){
        std::getline(file, line);
        std::istringstream iss(line);
        std::string  c;
        std::string roba;
        double inutile;
        iss >> c;
        if (c == "Segments"){
             iss >> inutile >> roba >> Ngr;
             }
              } */
 double x[Ngr],y[Ngr],sigma[Ngr], RE[Ngr],FW[Ngr], FW_err[Ngr];
 double res[Ngr],res_err[Ngr];
 double x_error[Ngr],y_error[Ngr],sigma_err[Ngr],RE_error[Ngr];
 double ysum=0.;
 double R=25.;

 for(int j=0;j<Ngr;j++){
  //file >> x[j] >> x_error[j] >> y[j] >>  y_error[j] >> sigma[j] >> sigma_err[j] >> RE[j] >> RE_error[j] ;
  file >> x[j] >> x_error[j] >> y[j] >>  y_error[j] >> sigma[j] >> sigma_err[j] ;

  //>>  FW[j] >> FW_err[j]
  
  //file >> y[j];
  //file >> x_error[j];
  //file >> y_error[j];
  //x[j] = x[j]*pow(10,9);
  //ysum += y[j];
  cout << x[j] << endl;

  }
 //ysum=ysum*(x[1]-x[0]);
 //cout << "Area " << ysum << endl;
 TCanvas *c1 = new TCanvas("c1","Grafico di interpolazione lineare",200,10,700,500);  
    
  c1->SetGrid();
  c1->Divide(1,2);
  c1->cd(1);
  

  TGraphErrors *gr2 = new TGraphErrors(Ngr,x,y,x_error,y_error);
  gr2->SetTitle("Pulser peak vs Temperature [C]");
  gr2->SetMarkerColor(4);
  gr2->SetMarkerStyle(7);
  gr2->SetMarkerSize(1);
  gr2->GetXaxis()->SetTitle("Temperature [C]");
  gr2->GetYaxis()->SetTitle("Peak[ch]");
  gr2->GetXaxis()->SetTitleSize(0.06);
  gr2->GetYaxis()->SetTitleSize(0.05);
  gr2->GetXaxis()->SetTickLength(0.04);
  gr2->GetYaxis()->SetTickLength(0.04);
  gr2->GetXaxis()->SetLabelSize(0.05);
  gr2->GetYaxis()->SetLabelSize(0.05); 
  gr2->Draw("AP");

  int Stop_lin = 4;
  Double_t start = 2.8;
  Double_t stop = 5.06;
  Double_t start2 = 5.03;
  Double_t stop2 = 9.5;
  
 // cout << "Inizio " << start << " e fine " << stop << endl;
  //printf("dammi intervallo in cui vuoi fittare la risoluzione\n");
  //scanf("%lf %lf",&start,&stop);
  
  
  /*
  TF1 *fun=new TF1("fit_func",fit_func,start,stop,2);//creo una classe che è apposta per fare i Fit si chiama TF1
  gr2->Fit(fun,"EMBRN","");
  fun->SetLineColor(kGreen);
  */

  TF1 *fun_exp = new TF1("fit_exp","pol1",start,stop2);//creo una classe che è apposta per fare i Fit si chiama TF1
  gr2->Fit("pol1");
  //fun_exp->SetLineColor(kGreen);

  Double_t p1_Err,p2_Err;
 // fun->Draw("same");
//  fun_exp->Draw("same");

  for(int i=0;i<Ngr;i++){
      res[i] = y[i] - fun_exp->Eval(x[i]);
      res_err[i] = y_error[i] + fun_exp->GetParError(1) + fun_exp->GetParError(2)*x[i];
    }
  

  TGraphErrors *gr3 = new TGraphErrors(Ngr,x,sigma,x_error, sigma_err);
  c1->cd(2);
  gr3->Draw("AP");
  gr3->Fit("pol1");
  gr3->SetTitle("Sigma vs Temperature");
  gr3->SetMarkerStyle(20);
  gr3->GetXaxis()->SetTitle("Temperature [C]");
  gr3->GetXaxis()->SetTitleSize(0.05);
  gr3->GetYaxis()->SetTitleSize(0.05);
  gr3->GetXaxis()->SetTickLength(0.04);
  gr3->GetYaxis()->SetTickLength(0.04);
  gr3->GetXaxis()->SetLabelSize(0.05);
  gr3->GetYaxis()->SetLabelSize(0.05); 
 
 gStyle->SetOptFit(1111);



TString nome_amp;
nome_amp = nome;
Int_t dot = nome_amp.First('.');// cerca la prima volta che appare il . nella parola
Int_t len = nome_amp.Length();
nome_amp.Remove(dot,len-dot);
nome_amp.Append(".root");
c1->SaveAs(nome_amp);

}

Double_t fit_func(Double_t *x,Double_t *par)//funzione da fittare
{
  Double_t F = 0.129;
Double_t v= par[0] + par[1]*(*x);
  return v;
}

Double_t fit_expo(Double_t *x,Double_t *par)//funzione da fittare
{
 Double_t v= par[0] + par[1]*(*x) + par[2]*(*x)*(*x);
  return v;
}


