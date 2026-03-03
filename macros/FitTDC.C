void FitTDC(TString inRoot="fe_target-01_TDC.root",
            TString histName="hTDC_A",
            double fitMin= 0.1 ,
            double fitMax=8.0,
            double zoomMax=2.8,
            int rebin=6,
            int doLogY=1)
{
	// open file
	TFile *f = new TFile(inRoot,"READ");
	if (!f || f->IsZombie()) {
		cout << "ERROR: could not open file " << inRoot << endl;
		return;
	}

	// get histogram
	TH1D *h = (TH1D*)f->Get(histName);
	if (!h) {
		cout << "ERROR: could not find histogram " << histName << " in " << inRoot << endl;
		f->ls();
		return;
	}

	// clone so you don’t permanently modify the file object
	TH1D *hc = (TH1D*)h->Clone(Form("%s_clone",histName.Data()));
	hc->SetDirectory(0); // detach from file

	// optional rebin to stabilize fit
	if (rebin > 1) hc->Rebin(rebin);

	// -------- Fit function: A*exp(-x/tau) + B --------
	TF1 *fit = new TF1(Form("fit_%s",histName.Data()), "[0]*exp(-x/[1]) + [2]", fitMin, fitMax);
	fit->SetParNames("A","tau_us","B");

	// good starting guesses
	fit->SetParameters(hc->GetMaximum(), 2.2, 1.0);

	// fit
	hc->Fit(fit,"R");

	double tau = fit->GetParameter(1);
	double tauErr = fit->GetParError(1);

	cout << "\n===== Fit Results for " << histName << " =====" << endl;
	cout << "tau = " << tau << " +/- " << tauErr << " us" << endl;
	cout << "lambda = 1/tau = " << (1.0/tau) << " 1/us" << endl;
	cout << "chi2/ndf = " << fit->GetChisquare() << " / " << fit->GetNDF() << endl;

	// -------- Draw full + zoom --------
	TCanvas *c = new TCanvas(Form("c_%s",histName.Data()),
	                         Form("Fit %s in %s",histName.Data(),inRoot.Data()),
	                         1200, 500);
	c->Divide(2,1);

	// full range
	c->cd(1);
	if (doLogY) gPad->SetLogy();
	hc->GetXaxis()->UnZoom();
	hc->Draw("E");
	fit->Draw("same");

	// zoom
	c->cd(2);
	if (doLogY) gPad->SetLogy();
	hc->GetXaxis()->SetRangeUser(0.0, zoomMax);
	hc->Draw("E");
	fit->Draw("same");

	// keep canvas around
	c->Update();

	// don’t close file until after clone created (we already detached hc)
	f->Close();
}
