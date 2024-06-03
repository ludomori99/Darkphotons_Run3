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
        {"Offline data",     "Inclusive MinBias"}
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
    TCanvas* c1 = new TCanvas("Comparison","Comparison",1200,900);
    setTDRStyle();
    c1->SetMargin(0.15, 0.03, 0.14, 0.12);

    //Plot
    pEffMC->SetMarkerColor(colorScheme[useScheme][0]);
    pEffMC->SetLineColor(colorScheme[useScheme][0]);
    pEffMC->SetMarkerStyle(21);
    pEffMC->Draw("PZ");
    // MarkerSize(0.7)
    gPad->Update();
    
    if (quantity == "Probe_pt")
    {
        pEffMC->SetTitle(";p_{T} [GeV/c];Efficiency");
    }
    if (quantity == "Probe_eta")
    {
        pEffMC->SetTitle(";#eta;Efficiency");
    }
    if (quantity == "Probe_abs_eta")
    {
        pEffMC->SetTitle(";|#eta|;Efficiency");
    }
    if (quantity == "Mm_dR")
    {
        pEffMC->SetTitle(";#Delta R;Efficiency");
    }


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
        graph->SetMinimum(0.86);
        graph->SetMaximum(1.02);
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

    pEffOffline->SetMarkerColor(colorScheme[useScheme][1]);
    pEffOffline->SetLineColor(colorScheme[useScheme][1]);
    pEffOffline->SetMarkerStyle(21);
    pEffOffline->Draw("same");

    //Legenda
    TLegend* tl = new TLegend(0.68,0.78,0.94,0.88);
    tl->SetTextSize(0.03);
    tl->SetBorderSize(0);
    tl->AddEntry(pEffOffline, nameScheme[useScheme][0], "lep");
    tl->AddEntry(pEffMC, nameScheme[useScheme][1], "lep");
    tl->Draw();

    //Label
    TPaveText label(0.68, 0.7, 0.94, 0.75, "NDC");
    label.SetBorderSize(0);
    label.SetFillColor(0);
    label.SetTextSize(0.031);
    label.SetTextFont(42);
    gStyle->SetStripDecimals(kTRUE);
    label.SetTextAlign(11);
    label.AddText(("Probe: "+MuonId).c_str());
    // label.AddText(string(isBarrel ? "|#eta|<1.43" : "") + string(isEndcap ? "|#eta|>1.43" : "" ).c_str());
    label.Draw();     
    
    CMS_single(c1,string("62.4"),string("13.6"),0.7);

    //Results stored in...
    string dir = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/")  + quantity + string("/");
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
