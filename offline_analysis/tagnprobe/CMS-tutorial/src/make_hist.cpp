TH1F* make_hist(string name, double** values, int qnt, int bin_n, Double_t* binning, bool IsDataMc, bool DRAW = false)
{
    //HISTOGRAM NEEDS TO HAVE VARIABLE BINS
   
    TH1F* hist = new TH1F(name.c_str(), name.c_str(), bin_n, binning);

    for (int i = 0; i < bin_n; i++)
    {
        // cout<<"\n\nbinning: "<<binning[i]<<","<<binning[i+1]<<"\ncontent: "<<values[i][qnt];
        hist->SetBinContent(i+1, values[i][qnt]);
        if (IsDataMc == false)
            hist->SetBinError(i+1, values[i][qnt+2]);
    }
    if (DRAW)
    {
        TCanvas* xperiment = new TCanvas;
        xperiment->cd();
        hist->Draw();
        string saveAs = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/Probe_abs_eta/") + to_string(qnt) + ".png";
        xperiment->SaveAs(saveAs.data());
    }
    return hist;
}

