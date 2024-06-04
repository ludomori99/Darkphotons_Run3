TEfficiency* get_efficiency(const char* filepath, TH1F* ALL, TH1F* PASS, string quantity, string MuonId, bool DataIsMc, bool isBarrel, bool isEndcap)
{    
    TFile* pFile = ((isBarrel||isEndcap) ? (new TFile(filepath,"update")) : (new TFile(filepath,"recreate")));
    if (!pFile || pFile->IsZombie()) {
        cout << "Error: Could not open file " << filepath << endl;
    }
    cout<<filepath<<"n";
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
    
    return pEff;
}
