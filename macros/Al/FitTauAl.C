void FitTauAl()
{
  TString inFile = "../../Outputs/outputAl.root";
  Double_t hardMin = 0.2;   // never fit earlier than this (us)
  Double_t fitMax  = 9.0;  // match your histogram range (us)

  Double_t fracOfPeak = 0.30; // tail starts when counts drop below this*peak
                              // smaller => later tail (e.g. 0.15), larger => earlier (e.g. 0.5)

  TFile *f = TFile::Open(inFile, "READ");
  if (!f || f->IsZombie()) { Printf("ERROR: could not open %s", inFile.Data()); return; }

  gSystem->mkdir("../../Outputs/Fits/Raw/Al", kTRUE);

  TF1 *fexp = new TF1("fexp", "[0]*exp(-x/[1])", hardMin, fitMax);
  fexp->SetParNames("A","tau");
  fexp->SetParLimits(1, 1e-6, 1e6); // tau > 0

  auto DoTailFit = [&](const char* hname, const char* tag)
  {
    TH1 *h = (TH1*)f->Get(hname);
    if (!h) { Printf("Missing hist: %s", hname); return; }

    h->SetStats(0);

    // Peak info
    Int_t bPeak = h->GetMaximumBin();
    Double_t yPeak = h->GetBinContent(bPeak);
    Double_t xPeak = h->GetBinCenter(bPeak);

    if (yPeak <= 0) { Printf("Histogram %s has no entries.", hname); return; }

    // Find tail start: first bin AFTER peak where content < fracOfPeak * peak
    Double_t threshold = fracOfPeak * yPeak;

    Int_t bStart = bPeak;
    Int_t bLast  = h->GetNbinsX();

    for (Int_t ib = bPeak; ib <= bLast; ib++) {
      if (h->GetBinContent(ib) <= threshold) { bStart = ib; break; }
    }

    Double_t fitStart = h->GetBinLowEdge(bStart);
    if (fitStart < hardMin) fitStart = hardMin;

    // Optional: if peak occurs after hardMin, ensure start is not before peak
    if (fitStart < xPeak) fitStart = xPeak;

    // If fitStart is too close to fitMax, bail
    if (fitStart >= fitMax - 1e-6) {
      Printf("Fit range invalid for %s (fitStart=%.4g, fitMax=%.4g).", tag, fitStart, fitMax);
      return;
    }

    // Initial guesses
    Double_t A0 = yPeak;
    Double_t tau0 = 2.0;

    //fexp->SetRange(fitStart, fitMax);
    fexp->SetRange(fitStart, fitMax);
    fexp->SetParameters(A0, tau0);

    // Fit tail only
    h->Fit(fexp, "R0");

    Double_t tau  = fexp->GetParameter(1);
    Double_t etau = fexp->GetParError(1);

    Printf("---- %s ----", tag);
    Printf("tail start: %.6g us  (peak at %.6g us, frac=%.2f)", fitStart, xPeak, fracOfPeak);
    Printf("tau = %.6g +/- %.6g us", tau, etau);

    // Draw + save
    TCanvas *c = new TCanvas(Form("c_%s", tag), Form("Tail fit %s", tag), 900, 650);
    h->Draw("HIST");
    fexp->Draw("SAME");

    TLatex latex;
    latex.SetNDC(kTRUE);
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.15, 0.85, Form("#tau = %.4g #pm %.2g #mus", tau, etau));
    latex.DrawLatex(0.15, 0.80, Form("Fit range: [%.3f, %.3f] #mus", fitStart, fitMax));
    latex.DrawLatex(0.15, 0.75, Form("Model: A e^{-t/#tau}   (no background)"));

    c->SaveAs(Form("../../Outputs/Fits/Raw/Al/%s_RawAl.png", tag));
    delete c;
  };

  DoTailFit("hTimeA", "TimeA");
  DoTailFit("hTimeB", "TimeB");
  DoTailFit("hTimeC", "TimeC");

  f->Close();
}