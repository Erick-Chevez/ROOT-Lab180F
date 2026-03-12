void FitFloatAl()
{
  TString inFile = "../../Outputs/outputAl.root";

  TFile *f = TFile::Open(inFile, "READ");
  if (!f || f->IsZombie()) {
    Printf("ERROR: could not open %s", inFile.Data());
    return;
  }

  gSystem->mkdir("../../Outputs/Fits/FloatC/Al", kTRUE);

  auto DoFit = [&](const char* hname, const char* tag, Double_t fitMin, Double_t fitMax)
  {
    TH1 *h = (TH1*)f->Get(hname);
    if (!h) {
      Printf("Missing hist: %s", hname);
      return;
    }

    h->SetStats(0);

    Int_t bPeak = h->GetMaximumBin();
    Double_t yPeak = h->GetBinContent(bPeak);

    if (yPeak <= 0) {
      Printf("Histogram %s has no entries.", hname);
      return;
    }

    // Initial guess for C from late-time average
    Int_t b1 = h->FindBin(2.0);
    Int_t b2 = h->FindBin(8.999999);

    if (b1 < 1) b1 = 1;
    if (b2 > h->GetNbinsX()) b2 = h->GetNbinsX();

    Double_t C0 = 0.0;
    if (b2 >= b1) C0 = h->Integral(b1, b2) / (b2 - b1 + 1);

    Double_t A0   = yPeak;
    Double_t tau0 = 2.0;

    TF1 *fexpC = new TF1(Form("f_%s", tag), "[0]*exp(-x/[1]) + [2]", fitMin, fitMax);
    fexpC->SetParNames("A", "tau", "C");
    fexpC->SetParameters(A0, tau0, C0);
    fexpC->SetParLimits(0, 0.0, 1e12);
    fexpC->SetParLimits(1, 1e-6, 1e6);
    fexpC->SetParLimits(2, 0.0, 1e12);

    h->Fit(fexpC, "R0");

    Double_t A    = fexpC->GetParameter(0);
    Double_t eA   = fexpC->GetParError(0);
    Double_t tau  = fexpC->GetParameter(1);
    Double_t etau = fexpC->GetParError(1);
    Double_t C    = fexpC->GetParameter(2);
    Double_t eC   = fexpC->GetParError(2);
    Double_t chi2 = fexpC->GetChisquare();
    Int_t    ndf  = fexpC->GetNDF();

    Printf("---- %s ----", tag);
    Printf("Fit range: [%.6g, %.6g] us", fitMin, fitMax);
    Printf("A   = %.6g +/- %.6g", A, eA);
    Printf("tau = %.6g +/- %.6g us", tau, etau);
    Printf("C   = %.6g +/- %.6g counts/bin", C, eC);
    if (ndf > 0) Printf("chi2/ndf = %.6g / %d = %.6g", chi2, ndf, chi2/ndf);
    else         Printf("chi2/ndf = %.6g / %d", chi2, ndf);

    TCanvas *c = new TCanvas(Form("c_%s", tag), Form("Fit %s", tag), 900, 650);
    h->Draw("HIST");
    fexpC->Draw("SAME");

    TLatex latex;
    latex.SetNDC(kTRUE);
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.15, 0.85, Form("#tau = %.4g #pm %.2g #mus", tau, etau));
    latex.DrawLatex(0.15, 0.80, Form("C = %.4g #pm %.2g counts/bin", C, eC));
    latex.DrawLatex(0.15, 0.75, Form("Fit range: [%.3f, %.3f] #mus", fitMin, fitMax));
    latex.DrawLatex(0.15, 0.70, "Model: A e^{-t/#tau} + C");

    c->SaveAs(Form("../../Outputs/Fits/FloatC/Al/%s_FloatAl.png", tag));

    delete c;
  };

  DoFit("hTimeA", "TimeA", 0.9, 8.99);
  DoFit("hTimeB", "TimeB", 0.2, 8.99);
  DoFit("hTimeC", "TimeC", 1.0, 8.99);

  f->Close();
}