void CsvToRoot(){
//create root file
TFile *f = new TFile("../Outputs/out.root","RECREATE");

//create 1D hist
TH1D  *h = new TH1D("h","title;x;y",100,0,1);

//Fill 1D histo 
h->Fill(0.3);





f->cd();
h->Write();
f->Close();



}