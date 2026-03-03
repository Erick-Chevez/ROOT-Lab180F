// File: MakeThresholdTimes.C
// Usage in ROOT:
//   root -l
//   .x MakeThresholdTimes.C
//
// What it does:
// - Loops over all *.csv files in ../data/Fe/
// - Each CSV has columns: Time(us), Channel A(V), Channel B(V), Channel C(V)
// - For EACH file, finds the FIRST *downward* crossing of -0.2 V for A/B/C
//   (i.e., previous sample > thr AND current sample <= thr)
// - Fills 3 histograms with the crossing times (one entry per file per channel, if found)
// - Writes everything to ../Outputs/output.root

#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TSystemFile.h"
#include "TList.h"
#include "TString.h"
#include "TFile.h"
#include "TH1D.h"
#include "TObjArray.h"
#include "TObjString.h"

void MakeThresholdTimes()
{
  // ----------------------------
  // User settings
  // ----------------------------
  const Double_t thr = -0.2;                 // threshold in Volts
  const TString inDir  = "../data/Fe";       // folder containing CSVs
  const TString outDir = "../Outputs";       // where output.root will be saved
  const TString outFileName = outDir + "/output.root";

  // Histogram binning (edit if you know your time window better)
  const Int_t    nBins = 500;
  const Double_t tMin  = 0.0;    // us
  const Double_t tMax  = 10.0; // us

  // ----------------------------
  // Make sure output directory exists
  // ----------------------------
  gSystem->mkdir(outDir, kTRUE);

  // ----------------------------
  // Create output ROOT file + histograms
  // ----------------------------
  TFile *fout = new TFile(outFileName, "RECREATE");
  if (!fout || fout->IsZombie()) {
    Error("MakeThresholdTimes", "Could not create output file: %s", outFileName.Data());
    return;
  }

  TH1D *hTimeA = new TH1D("hTimeA", "First threshold crossing time;Time (us);Counts", nBins, tMin, tMax);
  TH1D *hTimeB = new TH1D("hTimeB", "First threshold crossing time;Time (us);Counts", nBins, tMin, tMax);
  TH1D *hTimeC = new TH1D("hTimeC", "First threshold crossing time;Time (us);Counts", nBins, tMin, tMax);

  // ----------------------------
  // List CSV files in input directory using ROOT's directory tools
  // ----------------------------
  TSystemDirectory dir("FeDir", inDir);
  TList *files = dir.GetListOfFiles();
  if (!files) {
    Error("MakeThresholdTimes", "Could not list files in: %s", inDir.Data());
    fout->Close();
    return;
  }

  files->Sort(); // keeps processing order consistent

  // ----------------------------
  // Loop over files in ../data/Fe
  // ----------------------------
  TIter next(files);
  TSystemFile *sysFile = 0;

  Int_t nCSV = 0;
  Int_t nUsed = 0;

  while ((sysFile = (TSystemFile*)next())) {
    TString fname = sysFile->GetName();

    // skip "." ".." subdirs, etc.
    if (sysFile->IsDirectory()) continue;

    // only keep *.csv (case-insensitive)
    TString lower = fname; lower.ToLower();
    if (!lower.EndsWith(".csv")) continue;

    nCSV++;

    const TString fullPath = inDir + "/" + fname;

    // ----------------------------
    // Open CSV using C FILE* (no C++ std:: required)
    // ----------------------------
    FILE *fp = fopen(fullPath.Data(), "r");
    if (!fp) {
      Warning("MakeThresholdTimes", "Could not open: %s (skipping)", fullPath.Data());
      continue;
    }

    // ----------------------------
    // Scan file line-by-line:
    // - Ignore headers / non-numeric lines
    // - Find first DOWNWARD crossing of thr for each channel
    // - Stop reading file once all 3 channels have been found
    // ----------------------------
    Bool_t foundA = kFALSE, foundB = kFALSE, foundC = kFALSE;
    Double_t tA = 0.0, tB = 0.0, tC = 0.0;

    // We use "prev > thr AND current <= thr" to ensure it's the first downward crossing
    Bool_t havePrevA = kFALSE, havePrevB = kFALSE, havePrevC = kFALSE;
    Double_t prevA = 0.0, prevB = 0.0, prevC = 0.0;

    char line[4096];

    while (fgets(line, sizeof(line), fp)) {
      // Convert to TString for light cleanup
      TString s(line);
      s.ReplaceAll("\r", "");
      s.ReplaceAll("\n", "");

      // Skip empty lines
      if (s.Length() == 0) continue;

      // Many CSVs have spaces after commas; remove spaces to simplify parsing
      s.ReplaceAll(" ", "");

      // Try to parse four numbers: time, A, B, C
      // We accept both "t,a,b,c" and "t,a,b,c,..." (extra cols ignored by sscanf)
      Double_t t = 0.0, a = 0.0, b = 0.0, c = 0.0;
      Int_t nRead = sscanf(s.Data(), "%lf,%lf,%lf,%lf", &t, &a, &b, &c);

      // If this line isn't numeric data (headers like "Time,ChannelA,..."), skip it
      if (nRead != 4) continue;

      // --- Channel A threshold detection ---
      if (!foundA) {
        if (havePrevA) {
          if ((prevA > thr) && (a <= thr) && (t >= 0.2)) { foundA = kTRUE; tA = t; }
        }
        prevA = a; havePrevA = kTRUE;
      }

      // --- Channel B threshold detection ---
      if (!foundB) {
        if (havePrevB) {
          if ((prevB > thr) && (b <= thr)) { foundB = kTRUE; tB = t; }
        }
        prevB = b; havePrevB = kTRUE;
      }

      // --- Channel C threshold detection ---
      if (!foundC) {
        if (havePrevC) {
          if ((prevC > thr) && (c <= thr)) { foundC = kTRUE; tC = t; }
        }
        prevC = c; havePrevC = kTRUE;
      }

      // If we've found all three, no need to keep reading this file
      if (foundA && foundB && foundC) break;
    }

    fclose(fp);

    // ----------------------------
    // Fill histograms (one entry per file per channel, if found)
    // ----------------------------
    Bool_t usedThisFile = kFALSE;

    if (foundA) { hTimeA->Fill(tA); usedThisFile = kTRUE; }
    if (foundB && tB > 0.2) { hTimeB->Fill(tB); usedThisFile = kTRUE; }
    if (foundC && tC > 0.2) { hTimeC->Fill(tC); usedThisFile = kTRUE; }

    if (usedThisFile){ 
      nUsed++;
    }
    else {
      // 🔥 DELETE FILE because no valid crossing after 0.2 us
    gSystem->Unlink(fullPath);
    Printf("Deleted (no peak after 0.2 us): %s", fullPath.Data());
    }
  }
  

  // ----------------------------
  // Write output
  // ----------------------------
  fout->cd();
  hTimeA->Write();
  hTimeB->Write();
  hTimeC->Write();

  // Optional: store a little run summary as TNamed strings (no std:: needed)
  {
    TString s1; s1.Form("CSV files found: %d", nCSV);
    TString s2; s2.Form("CSV files used (>=1 channel found): %d", nUsed);
    TNamed("Summary1", s1.Data()).Write();
    TNamed("Summary2", s2.Data()).Write();
  }

  fout->Close();

  Printf("Done.");
  Printf("  Input folder:  %s", inDir.Data());
  Printf("  CSV files:     %d", nCSV);
  Printf("  Used files:    %d", nUsed);
  Printf("  Output file:   %s", outFileName.Data());
}

// If you execute with `.x MakeThresholdTimes.C`, ROOT will look for a function
// matching the file name sometimes depending on your setup.
// To be safe, we provide a wrapper with the same base name.
void MakeThresholdTimes_C() { MakeThresholdTimes(); }