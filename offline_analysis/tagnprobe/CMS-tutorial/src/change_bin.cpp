

void change_bin(int bin_iD, string condition, string MuonId, string quantity, bool DataIsMc, double* init_conditions, const char* path, string hist_file = "Histograms.root")
{
    TFile *_file0 = TFile::Open(hist_file.c_str());
    
    TH1F *hist_all  = (TH1F*)_file0->Get("ALL");
    TH1F *hist_pass = (TH1F*)_file0->Get("PASS");
    
    double * values = new double[4];
    
    values = doFit(path, condition, MuonId, quantity, init_conditions,false,false);
    
    hist_all->SetBinContent(bin_iD, values[0]);
    hist_all->SetBinError(bin_iD, values[2]);
    
    hist_pass->SetBinContent(bin_iD, values[1]);
    hist_pass->SetBinError(bin_iD, values[3]);
    
    TFile* EfficiencyFile = TFile::Open(hist_file.c_str(),"RECREATE");
    hist_all->SetDirectory(gDirectory);
    hist_pass->SetDirectory(gDirectory);
    EfficiencyFile->Write();
    
    get_efficiency(path, hist_all, hist_pass, quantity, MuonId, DataIsMc, false,false);
}
