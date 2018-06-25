#include <TH1F.h>
#include <TTree.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TFile.h>
#include <TCanvas.h>
#include <unistd.h>
#include <TAxis.h>
#include <TMath.h>
#include <Calib_RunControl.h>

#define N_ADC 16384 //14 bit
#define N_ADC_corr 16284 //14 bit
#define nPar 10

double CrystalBall(double *x, double *par);

double bkg(Double_t *x, Double_t *par) {
      return par[0] + par[1]*x[0];
   }

double Gaus(double *x, double *par){
	
	double Const = par[0];
	double peak = par[1];
	double sigma = par[2];

	double variance2 = sigma*sigma*2.0;
	double term = x[0] - peak;
	
	return Const*exp(-(term*term)/variance2);

}

double fitFunction(Double_t *x, Double_t *par) {
      return  CrystalBall(x,par) + bkg(x,&par[5]);
   }

double fitFunction_Gaus(Double_t *x, Double_t *par) {
      return  bkg(x,par) + Gaus(x,&par[2]);
   }

double fitFunction_DoubleCryst(Double_t *x, Double_t *par) {
      return  CrystalBall(x,par) + CrystalBall(x,&par[5]);
   }

void Calib_RunControl(){


	TString hname;
	
	hname.Form("h");
	
	
//-------------------------------------------------------------------------------------------------------------------------------------------------//
//Background Subtraction			
// Analizzo i file di segnale e di bacground, guardando le singole entries e trasformando gli spettri, da spettri
// espressi in conteggi ADC a spettri in energia.
//-------------------------------------------------------------------------------------------------------------------------------------------------//

		//File dati	
		TFile* f= new TFile(filename);
		TTree *t1 = (TTree*)f->Get("cspec");
		Double_t Ene;
		t1->SetBranchAddress("HPGeEnergyCh0",&Ene);
		Long64_t nentries = t1->GetEntries();

		//File bkg 600mV
		TFile* fbkg= new TFile(Background_file);
		TTree *t_bkg = (TTree*)fbkg->Get("HPGe");
		Double_t Ene_bkg;
		t_bkg->SetBranchAddress("Energy",&Ene_bkg);
		Long64_t nentries_bkg = t_bkg->GetEntries();
		

		/*File bkg 1400mV
		TFile* fbkg= new TFile(Background_file);
		TTree *t_bkg = (TTree*)fbkg->Get("cspec");
		Double_t Ene_bkg;
		t_bkg->SetBranchAddress("HPGeEnergyCh0",&Ene_bkg);
		Long64_t nentries_bkg = t_bkg->GetEntries();
		*/
		double min = 0;
		double max = 2.5;
		int nbin = 7500;

		TH1F* h_direct=new TH1F("histo_direct","",N_ADC_corr,100,N_ADC);
		TH1F* h_bkg=new TH1F("histo_bkg","",N_ADC_corr,100,N_ADC);
		/*
		TH1F* h_Ene=new TH1F("histo_Ene","",nbin,min,max);
		TH1F* h_Ene_bkg=new TH1F("histo_Ene_bkg","",nbin,min,max);
		TH1F* h_Ene_Nostrip=new TH1F("histo_Ene_Nostrip","",nbin,min,max);
   		*/
   		for (Long64_t i=0;i<nentries;i++) {
     		t1->GetEntry(i);
     		h_direct->Fill(Ene);
     		
     		}

     	for (Long64_t i=0;i<nentries_bkg;i++) {
     		t_bkg->GetEntry(i);
     		h_bkg->Fill(Ene_bkg);
     	}
		
		//Spettro acquisito
		if(plot){
			TCanvas *c_first_spect = new TCanvas("c_first_spect","c_first_spect",3620,3660);
     		c_first_spect->cd();
     		
     		}
     	
     		
	// 	c_first_spect->cd(2);
     	
     	h_bkg->SetLineColor(2);
     	if(plot){
     		h_bkg->Draw("");
     		h_direct->Draw("same");
     		gPad->SetLogy();	
     	}	
     	
     			
		//cout << "Fit segnale " << endl;
		TF1 *fSignal_P = new TF1("fSignal_P","gaus",sign_min_P,sign_max_P);
		TF1 *fSignal_retta = new TF1("fSignal_retta","pol1",sign_min_P,sign_max_P);
		h_direct->Fit(fSignal_retta,"RN");
		h_direct->Fit(fSignal_P,"RN");

		fSignal_retta->SetLineColor(kGreen);
		fSignal_retta->Draw("same");
		fSignal_P->Draw("same");
		
		TH1F* HSpectrum_bkg_traslato = new TH1F("Background_tr", " ",N_ADC_corr,100,N_ADC);
		
		int traslazione = 2;

		for(int j=0;j<N_ADC_corr;j++){
				int bin = j+100-traslazione;
				HSpectrum_bkg_traslato->Fill(bin,h_bkg->GetBinContent(j));
		}
		
		if(plot){
		HSpectrum_bkg_traslato->SetLineColor(kBlack);
		HSpectrum_bkg_traslato->Draw("same");
		}


		cout << "Fit fondo " << endl;
		TF1 *fSignal_bkg_P = new TF1("fSignal_bkg_P","gaus",sign_min_bkg_P,sign_max_bkg_P);
		TF1 *fSignal_retta_bkg = new TF1("fSignal_retta_bkg","pol1",sign_min_bkg_P,sign_max_bkg_P);
		HSpectrum_bkg_traslato->Fit(fSignal_retta_bkg,"RN");
		HSpectrum_bkg_traslato->Fit(fSignal_bkg_P,"RN");
		
		double Ncount_int = fSignal_P->Integral(sign_min_P,sign_max_P) - fSignal_retta->Integral(sign_min_P,sign_max_P);
		double Ncount_bkg =	fSignal_bkg_P->Integral(sign_min_bkg_P,sign_max_bkg_P) - fSignal_retta_bkg->Integral(sign_min_P,sign_max_P);

		cout << "Conteggi segnale " << Ncount_int << " conteggi bkg " << Ncount_bkg << " R= " << Ncount_bkg/Ncount_int <<  endl;
		
		TH1F *hnew = (TH1F*)h_direct->Clone("hnew");
		hnew->Add(HSpectrum_bkg_traslato,-(Ncount_int/Ncount_bkg));

		//hnew->SetLineColor(kBlack);
		
		if(plot){
			TCanvas *c_Energy = new TCanvas("c_Energy","c_Energy",3620,3660);
			c_Energy->cd();
			//hnew->Smooth();
			hnew->Draw("");
		}	
		
//-------------------------------------------------------------------------------------------------------------------------------------------------//
// Fit			
//-------------------------------------------------------------------------------------------------------------------------------------------------//
TH1F *h_fit;

gStyle->SetFitFormat("5.6g");
	


	if(fit){

			h_fit = (TH1F*)hnew->Clone("h_fit");
			//picco
	
		TF1 *fSignal = new TF1("fSignal","gaus",sign_min,sign_max);
		fSignal->SetLineColor(2);		
		TF1* Background = new TF1("Background",bkg,min_fit,max_fit,2);
		Background->SetLineColor(3);
		
		
		
		TCanvas *cfit = new TCanvas("cfit","Fit of electron Energy Spectrum",900,600);
  		gStyle->SetOptFit(0001);
  	  	cfit->cd();
		h_fit->Draw("");

		

		TF1 *fSpectrum = new TF1("fSpectrum",fitFunction_Gaus,min_fit,max_fit,5);
		fSpectrum->SetLineColor(1);
		fSpectrum->SetParNames("Bk0","Bk1","Const","#mu","#sigma");

		h_fit->Fit(fSignal,"RN");
		h_fit->Fit(Background,"RN+");
		
		Double_t param[5];
		Double_t param_read[5];
  		Background->GetParameters(&param[0]);
  		fSignal->GetParameters(&param[2]);
  		fSpectrum->SetParameters(param);
  		h_fit->Fit(fSpectrum,"RM+");

  		fSpectrum->GetParameters(&param_read[0]);
		Background->SetParameters(&param_read[0]);
		fSignal->SetParameters(&param_read[2]);

		fSignal->Draw("same");
		Background->Draw("same");

		//h_fit->GetXaxis()->SetRangeUser(min_fit,max_fit);

		if(onepeak ==false){
  		TF1 *fSignal1 = new TF1("fSignal1","gaus",sign_min_1,sign_max_1);
		fSignal1->SetLineColor(2);		
		TF1* Background1 = new TF1("Background1",bkg,min_fit_1,max_fit_1,2);
		Background1->SetLineColor(3);
		TF1 *fSpectrum1 = new TF1("fSpectrum1",fitFunction_Gaus,min_fit_1,max_fit_1,5);
		fSpectrum1->SetLineColor(1);
		fSpectrum1->SetParNames("Bk0_1","Bk1_1","Const_1","#mu_1","#sigma_1");

		h_fit->Fit(fSignal1,"RN");
		h_fit->Fit(Background1,"RN+");

		Double_t param1[5];
		Double_t param1_read[5];
  		Background1->GetParameters(&param1[0]);
  		fSignal1->GetParameters(&param1[2]);
  		fSpectrum1->SetParameters(param1);
  		h_fit->Fit(fSpectrum1,"RM+");

  		fSpectrum1->GetParameters(&param1_read[0]);
		Background1->SetParameters(&param1_read[0]);
		fSignal1->SetParameters(&param1_read[2]);

		fSignal1->Draw("same");
		Background1->Draw("same");

  		//h_fit->GetXaxis()->SetRangeUser(sign_min_1_pul,sign_max_1_pul);
		//h_fit->GetXaxis()->ZoomOut();

		
		TF1 *fSignal1_pul = new TF1("fSignal1_pul","gaus",sign_min_1_pul,sign_max_1_pul);
		fSignal1_pul->SetLineColor(2);		
		TF1* Background1_pul = new TF1("Background1_pul",bkg,min_fit_1_pul,max_fit_1_pul,2);
		Background1_pul->SetLineColor(3);
		TF1 *fSpectrum1_pul = new TF1("fSpectrum1_pul",fitFunction_Gaus,min_fit_1_pul,max_fit_1_pul,5);
		fSpectrum1_pul->SetLineColor(1);
		fSpectrum1_pul->SetParNames("Bk0_1","Bk1_1","Const_1","#mu_1","#sigma_1");

		h_fit->Fit(fSignal1_pul,"RN");
		h_fit->Fit(Background1_pul,"RN+");

		Double_t param1_pul[5];
		Double_t param1_read_pul[5];
  		Background1_pul->GetParameters(&param1_pul[0]);
  		fSignal1_pul->GetParameters(&param1_pul[2]);
  		fSpectrum1_pul->SetParameters(param1_pul);
  		h_fit->Fit(fSpectrum1_pul,"RM+");

  		fSpectrum1_pul->GetParameters(&param1_read_pul[0]);
		Background1_pul->SetParameters(&param1_read_pul[0]);
		fSignal1_pul->SetParameters(&param1_read_pul[2]);

		fSignal1_pul->Draw("same");
		Background1_pul->Draw("same");
  		
  		}

		double Mean,Mean_err,Sigma,Sigma_err;
		double Mean1,Sigma1,Mean1_err,Sigma1_err;
		double Mean1_pul,Sigma1_pul,Mean1_err_pul,Sigma1_err_pul;

		Mean = param_read[3];
		Sigma = param_read[4];
		Mean_err = fSpectrum->GetParError(3);
		Sigma_err = fSpectrum->GetParError(4);
		double Reso = ((Sigma*2.355)/Mean)*100;
	    double Reso_error = 100.*2.355*sqrt(pow((Mean_err/Mean),2) + pow((((Sigma/Mean)*Sigma_err)/Mean),2));
		cout << "Picco " << Mean << " +- " << Mean_err <<  endl;
	    cout << "Sigma " << Sigma << " +- " << Sigma_err <<  endl;
		cout << "Resolution [%] " << Reso << "+-" << Reso_error <<  endl;
		
		if(onepeak==false){
		Mean1 = param1_read[3];
		Sigma1 = param1_read[4];
		Mean1_err = fSpectrum1->GetParError(3);
		Sigma1_err = fSpectrum1->GetParError(4);
		double Reso1 = ((Sigma1*2.355)/Mean1)*100;
	    double Reso_error1 = 100.*2.355*sqrt(pow((Mean1_err/Mean1),2) + pow(((Sigma1/Mean1)*Sigma1_err/Mean1),2));
		cout << "Picco1 " << Mean1 << " +- " << Mean1_err <<  endl;
	    cout << "Sigma1 " << Sigma1 << " +- " << Sigma1_err <<  endl;
		cout << "Resolution1 [%] " << Reso1 << "+-" << Reso_error1 <<  endl;

		
		Mean1_pul = param1_read_pul[3];
		Sigma1_pul = param1_read_pul[4];
		Mean1_err_pul = fSpectrum1_pul->GetParError(3);
		Sigma1_err_pul = fSpectrum1_pul->GetParError(4);
		double Reso1_pul = ((Sigma1_pul*2.355)/Mean1_pul)*100;
	    double Reso_error1_pul = 100.*2.355*sqrt(pow((Mean1_err_pul/Mean1_pul),2) + pow(((Sigma1_pul/Mean1_pul)*Sigma1_err_pul/Mean1_pul),2));
		cout << "Picco Pulser " << Mean1_pul << " +- " << Mean1_err_pul <<  endl;
	    cout << "Sigma Pulser " << Sigma1_pul << " +- " << Sigma1_err_pul <<  endl;
		cout << "Resolution Pulser [%] " << Reso1_pul << "+-" << Reso_error1_pul <<  endl;
		
		}

		}

		  		
	
}


double CrystalBall(double* x, double* par){
 //http://en.wikipedia.org/wiki/Crystal_Ball_function
 double xcur = x[0];
 double alpha = par[0];
 double n = par[1];
 double mu = par[2];
 double sigma = par[3];
 double N = par[4];
 TF1* exp = new TF1("exp","exp(x)",1e-20,1e20);
 double A; double B;
 if (alpha < 0){
 A = pow((n/(-1*alpha)),n)*exp->Eval((-1)*alpha*alpha/2);
 B = n/(-1*alpha) + alpha;}
 else {
 A = pow((n/alpha),n)*exp->Eval((-1)*alpha*alpha/2);
 B = n/alpha - alpha;}
 double f;
 if ((xcur-mu)/sigma > (-1)*alpha)
 f = N*exp->Eval((-1)*(xcur-mu)*(xcur-mu)/
(2*sigma*sigma));
 else
 f = N*A*pow((B- (xcur-mu)/sigma),(-1*n));
 delete exp;

 return f;
} 
