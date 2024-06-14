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
    TCanvas* canvas = new TCanvas("Comparison","Comparison",1000,1000);
    setTDRStyle();

    canvas->Divide(1,2);
    canvas->cd(1);
    gPad->SetMargin(0.15, 0.03, 0.02, 0.075); //l,r,b,t
    gPad->SetPad(0.01,0.27,0.99,0.99);

    //Plot
    pEffMC->SetMarkerColor(colorScheme[useScheme][0]);
    pEffMC->SetLineColor(colorScheme[useScheme][0]);
    pEffMC->SetMarkerStyle(21);
    // MarkerSize(0.7)
    gPad->Update();
    pEffMC->Draw("E");

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
    graph->GetYaxis()->SetTitleOffset(1.05);
    graph->GetXaxis()->SetTitleSize(0.);
    graph->GetXaxis()->SetLabelSize(0.);
    graph->SetMaximum(1.2);

    gPad->Update();


    pEffOffline->SetMarkerColor(colorScheme[useScheme][1]);
    pEffOffline->SetLineColor(colorScheme[useScheme][1]);
    pEffOffline->SetMarkerStyle(21);


     if (quantity == "Probe_pt")
     {
        gPad->SetLogx();
        graph->GetHistogram()->GetXaxis()->SetLimits(4.,40.);
        graph->SetMinimum(0.86);
        graph->SetMaximum(1.02);

     }
    
    if (quantity == "Probe_eta")
    {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(-1.8,1.8);
        graph->SetMinimum(0.8);
        graph->SetMaximum(1.08);
    }
    
   if (quantity == "Probe_abs_eta")
    {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(0,1.8);
        graph->SetMinimum(0.8);
        graph->SetMaximum(1.08);
    }
    if (quantity == "Mm_dR")
    {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(0,0.8);
        graph->SetMinimum(0.8);
        graph->SetMaximum(1.05);
    }

    
    //Legenda
    TLegend* tl = new TLegend(0.62,0.28,0.88,0.38);
    tl->SetTextSize(0.03);
    tl->SetBorderSize(0);
    tl->AddEntry(pEffOffline, nameScheme[useScheme][0], "lep");
    tl->AddEntry(pEffMC, nameScheme[useScheme][1], "lep");

    //Label
    TPaveText label(0.62, 0.2, 0.94, 0.25, "NDC");
    label.SetBorderSize(0);
    label.SetFillColor(0);
    label.SetTextSize(0.031);
    label.SetTextFont(42);
    gStyle->SetStripDecimals(kTRUE);
    label.SetTextAlign(11);
    label.AddText(("Probe: "+MuonId).c_str());
    
    pEffMC->Draw("E");
    pEffOffline->Draw("Esame");
    tl->Draw();
    label.Draw();     

    CMS(canvas,string("62.4"),string("13.6"));
    // CMS_single(c1,string("62.4"),string("13.6"),0.5);

    canvas->cd(2);
    TH1* effOffHist = pEffOffline->GetCopyPassedHisto();
    effOffHist->Divide(pEffOffline->GetCopyPassedHisto(),pEffOffline->GetCopyTotalHisto(), 1, 1, "B");
    TH1* effMCHist  = pEffMC->GetCopyPassedHisto();
    effMCHist->Divide(pEffMC->GetCopyPassedHisto(),pEffMC->GetCopyTotalHisto(),1,1,"B");

    effOffHist->Sumw2();

    effOffHist->Divide(effMCHist); //massModel->GetName());
    effOffHist->Draw("E");

    TLine* line = new TLine(effOffHist->GetXaxis()->GetXmin(), 0,effOffHist->GetXaxis()->GetXmax(), 0);
    line->SetLineColor(kBlue);
    effOffHist->SetMarkerStyle(20);
    effOffHist->SetMarkerSize(1);

    gPad->SetLeftMargin(0.15);
    gPad->SetRightMargin(0.03);
    gPad->SetTopMargin(0.06);
    gPad->SetBottomMargin(0.4); 
    gPad->SetPad(0.01, 0.01, 0.99, 0.25);


    effOffHist->GetYaxis()->SetNdivisions(202);
    effOffHist->SetMinimum(0.96);
    effOffHist->SetMaximum(1.02);
    effOffHist->GetXaxis()->SetTitle(graph->GetXaxis()->GetTitle());
    effOffHist->GetYaxis()->SetTitle("Data/MC");
    effOffHist->GetXaxis()->SetTitleSize(0.16);
    effOffHist->GetYaxis()->SetTitleSize(0.16);
    effOffHist->GetXaxis()->SetLabelSize(0.14);
    effOffHist->GetYaxis()->SetLabelSize(0.14);
    effOffHist->GetXaxis()->SetLabelOffset(0.02);
    effOffHist->GetYaxis()->SetLabelOffset(0.01);
    effOffHist->GetXaxis()->SetTitleOffset(1.1);
    effOffHist->GetYaxis()->SetTitleOffset(0.4);
    effOffHist->GetXaxis()->SetTickLength(0.1);

    if (quantity == "Probe_pt")
    {
        effOffHist->GetXaxis()->SetLimits(4.,40.);
        gPad->SetLogx();
        effOffHist->GetXaxis()->SetMoreLogLabels();
        effOffHist->GetXaxis()->SetLabelOffset(0.01);
        effOffHist->GetXaxis()->SetTitleOffset(1.2);
    }

    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);

    gPad->Update();

    effOffHist->Draw("E");
    line->Draw();


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
    string saveAs = string(directoryToSave) + string(pEffMC->GetName()) + "_logx.png";

    canvas->SaveAs(saveAs.data());
}


//following script needs some fixes and is currently not used
void compare_plot_BarrelVsEndcap(TFile *fileMC, TFile *fileOffline, const char* path, string quantity, string MuonId)
{
    TEfficiency* pEffMCBarrel = (TEfficiency*)fileMC->Get((string(path) + string("barrel")).c_str());
    TEfficiency* pEffMCEndcap = (TEfficiency*)fileMC->Get((string(path) + string("endcap")).c_str());
    TEfficiency* pEffOfflineBarrel = (TEfficiency*)fileOffline->Get((string(path) + string("barrel")).c_str());
    TEfficiency* pEffOfflineEndcap = (TEfficiency*)fileOffline->Get((string(path) + string("endcap")).c_str());

    int colorScheme[][4] = {
        {kGreen - 2, kBlue, kRed, kOrange},
        {kBlue,      kGreen, kRed, kOrange}
    };

    const char* nameScheme[][4] = {
        {"#Upsilon data", "J/#psi data", " ", " "},
        {"Offline data barrel", "Offline data endcap",  "Inclusive MinBias barrel", "Inclusive MinBias endcap"}
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
    pEffMCBarrel->SetMarkerStyle(21);
    pEffMCBarrel->SetLineColor(colorScheme[useScheme][0]);
    pEffMCBarrel->Draw("PZ");
    // MarkerSize(0.7)
    
    
    if (quantity == "Probe_pt")
    {
        pEffMCBarrel->SetTitle(";p_{T} [GeV/c];Efficiency");
    }
    if (quantity == "Probe_eta")
    {
        pEffMCBarrel->SetTitle(";#eta;Efficiency");
    }
    if (quantity == "Probe_abs_eta")
    {
        pEffMCBarrel->SetTitle(";|#eta|;Efficiency");
    }
    if (quantity == "Mm_dR")
    {
        pEffMCBarrel->SetTitle(";#Delta R;Efficiency");
    }

    //Set range in y axis
    gPad->Update();
    auto graph = pEffMCBarrel->GetPaintedGraph();
    graph->SetMinimum(0.0);
    graph->GetYaxis()->SetTitleOffset(0.85);
    graph->SetMaximum(1.2);
    gPad->Update();

     if (quantity == "Probe_pt")
     {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(0.,40.);
        graph->SetMinimum(0.8);
        graph->SetMaximum(1.05);
     }
    
    if (quantity == "Probe_eta")
    {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(-1.8,1.8);
        graph->SetMinimum(0.8);
        graph->SetMaximum(1.08);
    }
    
   if (quantity == "Probe_abs_eta")
    {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(0,1.8);
        graph->SetMinimum(0.8);
        graph->SetMaximum(1.08);
    }
    if (quantity == "Mm_dR")
    {
        graph->GetHistogram()->GetXaxis()->SetRangeUser(0,0.8);
        graph->SetMinimum(0.8);
        graph->SetMaximum(1.05);
    }


    pEffMCEndcap->SetMarkerColor(colorScheme[useScheme][1]);
    pEffMCEndcap->SetMarkerStyle(21);
    pEffMCEndcap->SetLineColor(colorScheme[useScheme][1]);
    pEffMCEndcap->Draw("same");

    pEffOfflineBarrel->SetMarkerColor(colorScheme[useScheme][2]);
    pEffOfflineBarrel->SetMarkerStyle(21);
    pEffOfflineBarrel->SetLineColor(colorScheme[useScheme][2]);
    pEffOfflineBarrel->Draw("same");

    pEffOfflineEndcap->SetMarkerColor(colorScheme[useScheme][3]);
    pEffOfflineEndcap->SetMarkerStyle(21);
    pEffOfflineEndcap->SetLineColor(colorScheme[useScheme][3]);
    pEffOfflineEndcap->Draw("same");

    //Legenda
    TLegend* tl = new TLegend(0.64,0.78,0.94,0.95);
    tl->SetTextSize(0.03);
    tl->AddEntry(pEffOfflineBarrel, nameScheme[useScheme][0], "lep");
    tl->AddEntry(pEffOfflineEndcap, nameScheme[useScheme][1], "lep");
    tl->AddEntry(pEffMCBarrel, nameScheme[useScheme][2], "lep");
    tl->AddEntry(pEffMCEndcap, nameScheme[useScheme][3], "lep");
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
    

    //CMS Open Data
    TLatex* txCOD = new TLatex();
    txCOD->SetTextSize(0.04);
    txCOD->SetTextAlign(12);
    txCOD->SetTextFont(42);
    txCOD->SetNDC(kTRUE);
    txCOD->DrawLatex(0.14,0.85,Form("#bf{CMS Preliminary}"));

    //Results stored in...
    string dir = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/")  + quantity + string("/");
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
