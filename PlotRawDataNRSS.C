void PlotRawDataNRSS(TString rawfile, int n0, int n1){

TFile *f = new TFile(rawfile);

TTree *rawt = (TTree*)f->Get("rawdata");

float DataNRSS[8][1024];

rawt->SetBranchAddress("dataNRSS", DataNRSS);

TH1F *hspec[8];
TCanvas *cev[20];

int nentries = rawt->GetEntries();
cout<<"File has "<<nentries<<" events. "<<n0<<" to "<<n1<< "will be plotted"<<endl;

int count = 0;
for(int i = n0; i<n1; i++){
	rawt->GetEntry(i);
	TString cname;
	cname.Form("Event_%i_spectra",i);
	cev[count] = new TCanvas(cname,cname,600,800);
	cev[count]->Divide(2,4);
	for(int chi = 0; chi<8; chi++){
		TString hname;
		hname.Form("ev_%i_ch_%i",i,chi);
		hspec[chi] = new TH1F(hname,hname,1024,0,1024);
		for(int j = 0; j<1024; j++)
			hspec[chi]->SetBinContent(j+1,DataNRSS[chi][j]);
		cev[count]->cd(chi+1);
		hspec[chi]->Draw();
	}
	count++;
}

return;
}
