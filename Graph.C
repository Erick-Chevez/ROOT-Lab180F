void Graph()
{
    TGraph *graph = new TGraph();

   
    ifstream file("../EXAMPLE2-13/EXAMPLE2-13/EXAMPLE2-13_01.csv");

    if (!file.is_open()) 
    {
        cout << "Could not open EXAMPLE2-13_01.csv" << endl;
        return;
    }
    vector<double> t, v;
    string line;

    while(getline(file, line))
    {
        if (line.size() == 0) continue;

        // skip header lines that contain letters
        bool hasLetter = false;
        for (char c : line) 
        {
            if (isalpha(c)) { hasLetter = true; break; }
        }
        if (hasLetter) continue;

        stringstream ss(line);
        string tStr, vStr;

        if (!getline(ss, tStr, ',')) continue;
        if (!getline(ss, vStr, ',')) continue;

        double time_us = stod(tStr);
        double volt    = stod(vStr);

        t.push_back(time_us);
        v.push_back(volt);

        
    }
    file.close();

    if (t.size() == 0) {
        cout << "No points read.\n";
        return;
        }

    TGraph *gr = new TGraph(t.size(), &t[0], &v[0]);
    gr->SetTitle("Channel A Waveform;Time (us);Voltage (V)");

    TCanvas *c1 = new TCanvas("c1", "Waveform", 900, 600);
    gr->Draw("AL");   // A = axes, L = line
}