

TEfficiency* get_efficiency(TH1F* ALL, TH1F* PASS, string quantity, string MuonId, bool DataIsMc, bool isBarrel, bool isEndcap)
{    
    string* file_name = new string[2];
    file_name[0] = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/")+MuonId+string("/")+quantity+string("/Efficiency_Run3.root");
    file_name[1] = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/")+MuonId+string("/")+quantity+string("/Efficiency_MC.root");
    
    TFile* pFile = ((isBarrel||isEndcap) ? (new TFile(file_name[DataIsMc].c_str(),"update")) : (new TFile(file_name[DataIsMc].c_str(),"recreate")));
    TEfficiency* pEff = new TEfficiency();
    string name = string("Efficiency") + string(isBarrel ? "_barrel" : "") + string(isEndcap ? "_endcap" : "");
    pEff->SetName(name.c_str());
    pEff->SetPassedHistogram(*PASS, "f");
    pEff->SetTotalHistogram (*ALL,"f");
    
    pEff->SetDirectory(gDirectory);
    pFile->Write();
    
    TCanvas* oi = new TCanvas();
    oi->cd();
    pEff->Draw();

    gPad->Update();

    auto graph = pEff->GetPaintedGraph();
    graph->SetMinimum(0.8);
    graph->SetMaximum(1.2);
    gPad->Update();
    
    gSystem->cd("../..");
    delete[] file_name;
    
    return pEff;
}
