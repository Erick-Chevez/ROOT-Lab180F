void MakeTDC(TString folder="fe_target-01_All",
             TString outRoot="fe_target-01_TDC.root",
             double threshold_V=0.8,
             double tMin_us=-2.0,
             double tMax_us=9.0,
             int nBins= 1)
{
	// --------------------------
	// LOOK HERE: THRESHOLD KNOB
	// --------------------------
	// detection threshold in Volts (default 0.4 V)
	// Change by editing threshold_V above, or calling:
	//   MakeTDC("fe_target-01_All","out.root",0.35);
	double VTH = threshold_V;

	// style (optional, like your example)
	gROOT->SetStyle("Plain");
	gStyle->SetTitleBorderSize(0);
	gStyle->SetPalette(1);

	// output ROOT file
	TFile *fout = new TFile(outRoot,"RECREATE");
	assert(!fout->IsZombie());

	// histograms: TDC = time of detections
	TH1D *hTDC_A = new TH1D("hTDC_A","TDC Channel A;Time (#mus);Detections",nBins,tMin_us,tMax_us);
	TH1D *hTDC_B = new TH1D("hTDC_B","TDC Channel B;Time (#mus);Detections",nBins,tMin_us,tMax_us);
	TH1D *hTDC_C = new TH1D("hTDC_C","TDC Channel C;Time (#mus);Detections",nBins,tMin_us,tMax_us);

	// tree + branches
	TTree *tdcTree = new TTree("tdcTree","TDC detections from CSVs (rows passing threshold)");

	double time_us;
	double vA, vB, vC;
	int fileIndex;

	tdcTree->Branch("time_us",&time_us,"time_us/D");
	tdcTree->Branch("vA",&vA,"vA/D");
	tdcTree->Branch("vB",&vB,"vB/D");
	tdcTree->Branch("vC",&vC,"vC/D");
	tdcTree->Branch("fileIndex",&fileIndex,"fileIndex/I");

	// collect csv filenames in folder using ROOT's gSystem (no std containers)
	TList fileList;
	void *dirp = gSystem->OpenDirectory(folder);
	assert(dirp);

	const char *entry;
	while ((entry = gSystem->GetDirEntry(dirp))) {
		TString name = entry;
		if (name=="." || name=="..") continue;
		if (!name.EndsWith(".csv")) continue;

		TString fullPath = folder + "/" + name;
		fileList.Add(new TObjString(fullPath));
	}
	gSystem->FreeDirectory(dirp);

	int nFiles = fileList.GetSize();
	if (nFiles<=0) {
		cout << "ERROR: no .csv files found in folder: " << folder << endl;
		fout->Close();
		return;
	}
	cout << "Found " << nFiles << " CSV files in " << folder << endl;
	cout << "Threshold VTH = " << VTH << endl;

	// loop files
	Long64_t nRowsRead = 0;
	Long64_t nTreeEntries = 0;

	TIter next(&fileList);
	TObjString *os;

	fileIndex = 0;
	while ((os = (TObjString*)next())) {

		if ((fileIndex % 1000)==0) cout << "Processing fileIndex " << fileIndex << " / " << nFiles << endl;

		TString path = os->GetString();

		ifstream fin;
		fin.open(path.Data());
		if (fin.fail()) {
			cout << "WARNING: couldn't open " << path << " (skipping)" << endl;
			fileIndex++;
			continue;
		}

		// skip first two non-empty header lines (like your example has)
		TString line;
		int headerSkipped = 0;

		while (line.ReadLine(fin)) {
			line = line.Strip(TString::kBoth);
			if (line.Length()==0) continue;

			if (headerSkipped < 2) {
				headerSkipped++;
				continue;
			}

			// tokenize CSV: time, A, B, C
			TObjArray *tok = line.Tokenize(",");
			if (!tok || tok->GetEntries() < 4) { if (tok) tok->Delete(); delete tok; continue; }

			time_us = ((TObjString*)tok->At(0))->GetString().Atof();
			vA      = ((TObjString*)tok->At(1))->GetString().Atof();
			vB      = ((TObjString*)tok->At(2))->GetString().Atof();
			vC      = ((TObjString*)tok->At(3))->GetString().Atof();

			tok->Delete(); delete tok;

			nRowsRead++;

			// detection rule
			bool detA, detB, detC;

            detA = (vA <= -VTH);
            detB = (vB <= -VTH);
            detC = (vC <= -VTH);

			// fill TDC histograms
			if (detA) hTDC_A->Fill(time_us);
			if (detB) hTDC_B->Fill(time_us);
			if (detC) hTDC_C->Fill(time_us);

			// store ONLY rows where at least one channel passes threshold
			if (detA || detB || detC) {
				tdcTree->Fill();
				nTreeEntries++;
			}
		}

		fin.close();
		fileIndex++;
	}


	// write outputs
    cout << "Total rows read: " << nRowsRead << endl;
    cout << "Tree entries (rows with any detection): " << nTreeEntries << endl;

    // make sure output file is the current directory
    fout->cd();

    // ----- Canvas A (log scale) -----
    TCanvas *cA = new TCanvas("cTDC_A","TDC Channel A",800,600);
    cA->SetLogy();
    hTDC_A->SetMinimum(0.1);
    hTDC_A->Draw();
    cA->Write();

    // ----- Canvas B (log scale) -----
    TCanvas *cB = new TCanvas("cTDC_B","TDC Channel B",800,600);
    cB->SetLogy();
    hTDC_B->SetMinimum(0.1);
    hTDC_B->Draw();
    cB->Write();

    // ----- Canvas C (log scale) -----
    TCanvas *cC = new TCanvas("cTDC_C","TDC Channel C",800,600);
    cC->SetLogy();
    hTDC_C->SetMinimum(0.1);
    hTDC_C->Draw();
    cC->Write();

    // write histos + tree
    hTDC_A->Write();
    hTDC_B->Write();
    hTDC_C->Write();
    tdcTree->Write();

    fout->Close();

cout << "Wrote ROOT file: " << outRoot << endl;
    
}
