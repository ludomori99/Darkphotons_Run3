void compare_plot(TFile *fileMC, TFile *fileOffline, const char* path, string quantity, string MuonId)
{
    TEfficiency* pEffMC = (TEfficiency*)fileMC->Get(path);
    TEfficiency* pEffOffline = (TEfficiency*)fileOffline->Get(path);

    int colorScheme[][2] = {
        {kGreen - 2, kBlue},
        {kBlue,      kRed},
        {kGreen - 2, kRed}
    };

    const char* nameScheme[][2] = {
        {"#Upsilon data", "J/#psi data"},
        {"Real data",     "Simulated data"},
        {"Real data",     "Simulated data"}
    };

    int useScheme = 1;
    //Upsilon vs Jpsi
    //Jpsi    Run vs MC
    //Upsilon Run vs MC

    if (pEffMC == NULL)
    {
        cerr << "Could not read the path in file0\n";
        abort();
    }

    if (pEffOffline == NULL)
    {
        cerr << "Could not read the path in file1\n";
        abort();
    }

    //Create canvas
    TCanvas* c1 = new TCanvas("Comparison","Comparison",1200,800);
    //gStyle->SetOptTitle(0);
    c1->SetMargin(0.10, 0.03, 0.11, 0.07);


    //Plot
    pEffMC->SetMarkerColor(colorScheme[useScheme][0]);
    pEffMC->SetLineColor(colorScheme[useScheme][0]);
    pEffMC->SetMarkerStyle(21);
    pEffMC->Draw("PZ");
    // MarkerSize(0.7)
    gPad->Update();
    
    if (quantity == "Probe_pt")
    {
        pEffMC->SetTitle("Efficiency of Tracker Probe Muon;p_{T} [GeV/c];Efficiency");
    }
    if (quantity == "Probe_eta")
    {
        pEffMC->SetTitle("Efficiency of Tracker Probe Muon;#eta;Efficiency");
    }
    if (quantity == "Probe_abs_eta")
    {
        pEffMC->SetTitle("Efficiency of Tracker Probe Muon;|#eta|;Efficiency");
    }
    if (quantity == "Mm_dR")
    {
        pEffMC->SetTitle("Efficiency of Tracker Probe Muon;dR;Efficiency");
    }

    pEffOffline->SetMarkerColor(colorScheme[useScheme][1]);
    pEffOffline->SetLineColor(colorScheme[useScheme][1]);
    pEffOffline->SetMarkerStyle(21);
    pEffOffline->Draw("same");

    //Set range in y axis
    gPad->Update();
    auto graph = pEffMC->GetPaintedGraph();
    graph->SetMinimum(0.0);
    graph->GetYaxis()->SetTitleOffset(0.85);
    graph->SetMaximum(1.2);
    gPad->Update();

     if (quantity == "Probe_pt")
     {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(0.,80.);
        graph->SetMinimum(0.5);
        graph->SetMaximum(1.2);
     }
    
    if (quantity == "Probe_eta")
    {
        pEffMC->GetPaintedGraph()->GetHistogram()->GetXaxis()->SetRangeUser(-3.,3.);
        graph->SetMinimum(0.8);
        graph->SetMaximum(1.08);
    }
    
   if (quantity == "Probe_abs_eta")
    {
        pEffMC->GetPaintedGraph()->GetHistogram()->GetXaxis()->SetRangeUser(0,3.);
        graph->SetMinimum(0.8);
        graph->SetMaximum(1.08);
    }
    if (quantity == "Mm_dR")
    {
        pEffMC->GetPaintedGraph()->GetHistogram()->GetXaxis()->SetRangeUser(0,1.);
        graph->SetMinimum(0.9);
        graph->SetMaximum(1.05);
    }

    //Legenda
    TLegend* tl = new TLegend(0.68,0.78,0.94,0.88);
    tl->SetTextSize(0.03);
    tl->AddEntry(pEffMC, nameScheme[useScheme][0], "lep");
    tl->AddEntry(pEffOffline, nameScheme[useScheme][1], "lep");
    tl->Draw();

    //Label
    TPaveText label(0.68, 0.63, 0.94, 0.73, "NDC");
    label.SetBorderSize(0);
    label.SetFillColor(0);
    label.SetTextSize(0.031);
    label.SetTextFont(42);
    gStyle->SetStripDecimals(kTRUE);
    label.SetTextAlign(11);
    label.AddText(("Probe: "+MuonId).c_str());
    // label.AddText(string(isBarrel ? "|#eta|<1.43" : "") + string(isEndcap ? "|#eta|>1.43" : "" ).c_str());
    label.Draw();     
    

    //CMS Open Data
    TLatex* txCOD = new TLatex();
    txCOD->SetTextSize(0.04);
    txCOD->SetTextAlign(12);
    txCOD->SetTextFont(42);
    txCOD->SetNDC(kTRUE);
    txCOD->DrawLatex(0.14,0.85,Form("#bf{CMS Preliminary}"));

    //Results stored in...
    string dir = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + quantity + string("/");
    const char* directoryToSave = dir.c_str();

    //Check if dir exists
    if (gSystem->AccessPathName(directoryToSave))
    {
        if (gSystem->mkdir(directoryToSave))
        {
            cerr << "\"" << directoryToSave << "\" directory not found and could not be created ERROR" << endl;
            abort();
        }
        else
        {
            cout << "\"" << directoryToSave << "\" directory created OK" << endl;
        }
    }
    else
    {
        cout << "\"" << directoryToSave << "\" directory OK" << endl;
    }

    //stores file as .png
    string saveAs = string(directoryToSave) + string(pEffMC->GetName()) + ".png";

    c1->SaveAs(saveAs.data());
}

void compare_plot_barrelVsEndcap(TFile *fileMC, TFile *fileOffline, const char* path, string quantity, string MuonId)
{
    TEfficiency* pEffMCBarrel = (TEfficiency*)fileMC->Get((string(path) + string("barrel")).c_str());
    TEfficiency* pEffMCEndcap = (TEfficiency*)fileMC->Get((string(path) + string("endcap")).c_str());
    TEfficiency* pEffOfflineBarrel = (TEfficiency*)fileOffline->Get(path);
    TEfficiency* pEffOfflineEndcap = (TEfficiency*)fileOffline->Get(path);

    int colorScheme[][4] = {
        {kGreen - 2, kBlue, kRed, kOrange},
        {kBlue,      kGreen, kRed, kOrange}
    };

    const char* nameScheme[][4] = {
        {"#Upsilon data", "J/#psi data", " ", " "},
        {"Offline trigger barrel", "Offline trigger endcap",  "Inclusive MinBias barrel", "Inclusive MinBias endcap"}
    };

    int useScheme = 1;

    if ((pEffMCBarrel == NULL) || (pEffMCEndcap == NULL)){
        cerr << "Could not read the path in file0\n";
        abort();
    }
    
    if ((pEffOfflineBarrel == NULL) || (pEffOfflineEndcap == NULL)){
        cerr << "Could not read the path in file1\n";
        abort();
    }

    //Create canvas
    TCanvas* c1 = new TCanvas("Comparison","Comparison",1200,800);
    //gStyle->SetOptTitle(0);
    c1->SetMargin(0.10, 0.03, 0.11, 0.07);

    //Plot
    pEffMCBarrel->SetMarkerColor(colorScheme[useScheme][0]);
    pEffMCBarrel->SetLineColor(colorScheme[useScheme][0]);
    pEffMCBarrel->Draw("PZ");
    // MarkerSize(0.7)
    
    pEffMCEndcap->SetMarkerColor(colorScheme[useScheme][1]);
    pEffMCEndcap->SetLineColor(colorScheme[useScheme][1]);
    pEffMCEndcap->Draw("PZ");
    // MarkerSize(0.7)
    gPad->Update();
    
    if (quantity == "Probe_pt")
    {
        pEffMCBarrel->SetTitle("Efficiency of Tracker Probe Muon;p_{T} [GeV/c];Efficiency");
    }
    if (quantity == "Probe_eta")
    {
        pEffMCBarrel->SetTitle("Efficiency of Tracker Probe Muon;#eta;Efficiency");
    }
    if (quantity == "Probe_abs_eta")
    {
        pEffMCBarrel->SetTitle("Efficiency of Tracker Probe Muon;|#eta|;Efficiency");
    }
    if (quantity == "Mm_dR")
    {
        pEffMCBarrel->SetTitle("Efficiency of Tracker Probe Muon;dR;Efficiency");
    }

    pEffOfflineBarrel->SetMarkerColor(colorScheme[useScheme][2]);
    pEffOfflineBarrel->SetLineColor(colorScheme[useScheme][2]);
    pEffOfflineBarrel->Draw("same");

    pEffOfflineEndcap->SetMarkerColor(colorScheme[useScheme][3]);
    pEffOfflineEndcap->SetLineColor(colorScheme[useScheme][3]);
    pEffOfflineEndcap->Draw("same");

    //Set range in y axis
    gPad->Update();
    auto graph = pEffMCBarrel->GetPaintedGraph();
    graph->SetMinimum(0.0);
    graph->GetYaxis()->SetTitleOffset(0.85);
    graph->SetMaximum(1.2);
    gPad->Update();

     if (quantity == "Probe_pt")
     {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(0.,80.);
        graph->SetMinimum(0.5);
        graph->SetMaximum(1.2);
     }
    
    if (quantity == "Probe_eta")
    {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(-3.,3.);
        graph->SetMinimum(0.8);
        graph->SetMaximum(1.08);
    }
    
   if (quantity == "Probe_abs_eta")
    {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(0,3.);
        graph->SetMinimum(0.8);
        graph->SetMaximum(1.08);
    }
    if (quantity == "Mm_dR")
    {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(0,1.);
        graph->SetMinimum(0.9);
        graph->SetMaximum(1.05);
    }

    //Legenda
    TLegend* tl = new TLegend(0.68,0.78,0.94,0.88);
    tl->SetTextSize(0.03);
    tl->AddEntry(pEffOfflineBarrel, nameScheme[useScheme][0], "lep");
    tl->AddEntry(pEffOfflineEndcap, nameScheme[useScheme][1], "lep");
    tl->AddEntry(pEffMCBarrel, nameScheme[useScheme][2], "lep");
    tl->AddEntry(pEffMCEndcap, nameScheme[useScheme][3], "lep");
    tl->Draw();

    //Label
    TPaveText label(0.25, 0.63, 0.34, 0.78, "NDC");
    label.SetBorderSize(0);
    label.SetFillColor(0);
    label.SetTextSize(0.041);
    label.SetTextFont(42);
    gStyle->SetStripDecimals(kTRUE);
    label.SetTextAlign(11);
    label.AddText(("Probe: "+MuonId).c_str());
    // label.AddText(string(isBarrel ? "|#eta|<1.43" : "") + string(isEndcap ? "|#eta|>1.43" : "" ).c_str());
    label.Draw();     
    

    //CMS Open Data
    TLatex* txCOD = new TLatex();
    txCOD->SetTextSize(0.04);
    txCOD->SetTextAlign(12);
    txCOD->SetTextFont(42);
    txCOD->SetNDC(kTRUE);
    txCOD->DrawLatex(0.14,0.85,Form("#bf{CMS Preliminary}"));

    //Results stored in...
    string dir = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + quantity + string("/");
    const char* directoryToSave = dir.c_str();

    //Check if dir exists
    if (gSystem->AccessPathName(directoryToSave)){
        if (gSystem->mkdir(directoryToSave)){
            cerr << "\"" << directoryToSave << "\" directory not found and could not be created ERROR" << endl;
            abort();
        }
        else cout << "\"" << directoryToSave << "\" directory created OK" << endl;
    }
    else cout << "\"" << directoryToSave << "\" directory OK" << endl;
    //stores file as .png
    string saveAs = string(directoryToSave) + string(pEffMCBarrel->GetName()) + ".png";
    c1->SaveAs(saveAs.data());
}



//Compare efficiency
void compare_efficiency(string quantity, string Run3, string MC, string MuonId, bool barrelVsEndcap = false)
{
    TFile *file0 = TFile::Open(MC.c_str());
    TFile *file1 = TFile::Open(Run3.c_str());

    if (file0 == NULL || file1 == NULL)
    {
        std::cerr << "ABORTING...\n";
        abort();
    }
    if (barrelVsEndcap)  compare_plot(file0, file1, "Efficiency_", quantity, MuonId);
    else compare_plot(file0, file1, "Efficiency", quantity, MuonId);
}
