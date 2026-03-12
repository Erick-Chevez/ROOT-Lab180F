void FitBGFe()
{
  TString inFile = "../../Outputs/output.root";

  TFile *f = TFile::Open(inFile, "READ");
  if (!f || f->IsZombie()) {
    Printf("ERROR: could not open %s", inFile.Data());
    return;
  }

  gSystem->mkdir("../../Outputs/Fits/ChosenC/Fe", kTRUE);

  auto DoFit = [&](const char* hname, const char* tag, double fitMin, double fitMax, double Cfixed)
  {
    TH1 *h = (TH1*)f->Get(hname);
    if (!h) {
      Printf("Missing hist: %s", hname);
      return;
    }

    h->SetStats(0);

    int bPeak = h->GetMaximumBin();
    double yPeak = h->GetBinContent(bPeak);

    if (yPeak <= 0) {
      Printf("Histogram %s has no entries.", hname);
      return;
    }

    double A0   = yPeak;
    double tau0 = 2.0;

    TF1 *fexpC = new TF1(Form("f_%s", tag), "[0]*exp(-x/[1]) + [2]", fitMin, fitMax);
    fexpC->SetParNames("A", "tau", "C");
    fexpC->SetParameters(A0, tau0, Cfixed);
    fexpC->SetParLimits(0, 0.0, 1e12);
    fexpC->SetParLimits(1, 1e-6, 1e6);
    fexpC->FixParameter(2, Cfixed);

    h->Fit(fexpC, "R0");

    double A    = fexpC->GetParameter(0);
    double eA   = fexpC->GetParError(0);
    double tau  = fexpC->GetParameter(1);
    double etau = fexpC->GetParError(1);
    double C    = fexpC->GetParameter(2);
    double chi2 = fexpC->GetChisquare();
    int ndf     = fexpC->GetNDF();

    Printf("---- %s ----", tag);
    Printf("Fit range: [%.6g, %.6g] us", fitMin, fitMax);
    Printf("A   = %.6g +/- %.6g", A, eA);
    Printf("tau = %.6g +/- %.6g us", tau, etau);
    Printf("C   = %.6g (fixed)", C);
    if (ndf > 0) Printf("chi2/ndf = %.6g / %d = %.6g", chi2, ndf, chi2/ndf);
    else         Printf("chi2/ndf = %.6g / %d", chi2, ndf);

    TCanvas *c = new TCanvas(Form("c_%s", tag), Form("Fit %s", tag), 900, 650);
    h->Draw("HIST");
    fexpC->Draw("SAME");

    TLatex latex;
    latex.SetNDC(kTRUE);
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.15, 0.85, Form("#tau = %.4g #pm %.2g #mus", tau, etau));
    latex.DrawLatex(0.15, 0.80, Form("C = %.4g (fixed)", C));
    latex.DrawLatex(0.15, 0.75, Form("Fit range: [%.3f, %.3f] #mus", fitMin, fitMax));
    latex.DrawLatex(0.15, 0.70, "Model: A e^{-t/#tau} + C");

    c->SaveAs(Form("../../Outputs/Fits/ChosenC/Fe/%s_FixedFe.png", tag));

    delete c;
  };

  DoFit("hTimeA", "TimeA", 1.0, 9.0, 2.28);
  DoFit("hTimeB", "TimeB", 0.2, 9.0, 4.88571);
  DoFit("hTimeC", "TimeC", 0.2, 9.0, 12.6857);

  f->Close();
}