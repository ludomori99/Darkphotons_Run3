
#include "CMS.C"

void multi_plot(){
    string wspath =  string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/models/Multi_fit_output.root");
    std::unique_ptr<TFile> f_dCB{TFile::Open(wspath.c_str(), "READ")};
    auto ws_dCB = static_cast<RooWorkspace*>(f_dCB->FindObjectAny("ws_dCB"));  
    ws_dCB->Print();
    
    RooRealVar *Mm_mass = ws->var("Mm_mass");

    plot("Phi", Mm_mass_Phi, Data_Phi, model_Phi, n_signal_Phi, n_bkg_Phi, logFile);


}


void plot(const char* name, RooRealVar Mm_mass, RooDataSet* Data, RooAbsPdf* model, RooRealVar n_signal, RooRealVar n_back, ofstream& logFile, bool save=true){
    TCanvas* canvas  = new TCanvas(name,name,1000,800);
    setTDRStyle();
    CMS(canvas);

    canvas->Divide(1,2);
    canvas->cd(1);
    RooPlot *frame = Mm_mass.frame(RooFit::Title("Invariant Mass"));

    frame->SetTitle(name);
    frame->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");

    Data->plotOn(frame,XErrorSize(1), Name("data"), MarkerSize(1.), DrawOption("PZ"));
    model->plotOn(frame,Name("Full model"));
    model->plotOn(frame,Components(("sigModel_"+string(name)).c_str()),Name("Signal model"),DrawOption("F"),FillColor(3), FillStyle(3001), LineColor(0));
    model->plotOn(frame,Components(("bkgModel_"+string(name)).c_str()),Name("Background model"),LineStyle(7), LineColor(2));

    gPad->SetLeftMargin(0.15);
    gPad->SetBottomMargin(0.02);
    gPad->SetPad(0.01,0.3,0.99,0.99);
    frame->GetYaxis()->SetLabelSize(0.05);
    frame->GetYaxis()->SetTitleSize(0.07);
    frame->GetYaxis()->SetTitleOffset(1.2);
    frame->GetXaxis()->SetLabelSize(0.);
    frame->GetXaxis()->SetTitleSize(0.);

    TLegend leg(0.75, 0.6, 0.85,0.85);
    leg.AddEntry(frame->findObject("data"), "Data", "lep");
    leg.AddEntry(frame->findObject("Full model"), "Full model", "l");
    leg.AddEntry(frame->findObject("Signal model"), "Signal model", "f");
    leg.AddEntry(frame->findObject("Background model"), "Bkg. model", "l");
    leg.SetBorderSize(0);
    leg.SetFillStyle(0);
    leg.SetTextSize(0.04);
    leg.SetTextFont(42);

    //compute chisquare
    RooAbsCollection *flparams = model->getParameters(Data)->selectByAttrib("Constant", kFALSE);
    Int_t nflparams = flparams->getSize();
    Double_t chisquare = -1;
    chisquare = frame->chiSquare("Full model", "data", nflparams);

    TPaveText label_2(0.25, 0.63, 0.34, 0.82, "NDC");
    label_2.SetBorderSize(0);
    label_2.SetFillColor(0);
    label_2.SetTextSize(0.041);
    label_2.SetTextFont(42);
    gStyle->SetStripDecimals(kTRUE);
    label_2.SetTextAlign(11);
    TString sYield = to_string(int(round(n_signal.getValV())));
    TString bYield = to_string(int(round(n_back.getValV())));
    TString csquare = to_string(chisquare);
    label_2.AddText("N_{sig} = " + sYield);
    label_2.AddText("N_{bkg} = " + bYield);
    label_2.AddText("#chi^{2} = " + csquare);
    logFile << "\nchisquare=" << chisquare<<"\n";

    frame->Draw();
    label_2.Draw();     
    leg.Draw();


    canvas->cd(2);
    RooHist *hpull = frame->pullHist("data", "Full model"); //massModel->GetName());
    RooPlot *frame_pulls = Mm_mass.frame(Title("Pull"));
    TLine* line = new TLine(Mm_mass.getMin(), 0, Mm_mass.getMax(), 0);
    line->SetLineColor(kBlue);
    frame_pulls->addObject(line);
    frame_pulls->addPlotable(hpull, "P"); //,"E3")
    frame_pulls->SetMarkerStyle(2);
    frame_pulls->SetMarkerSize(0.01);

    gPad->SetLeftMargin(0.15);
    gPad->SetPad(0.01, 0.01, 0.99, 0.3);
    gPad->SetTopMargin(0.01);
    gPad->SetBottomMargin(0.5); 
    frame_pulls->GetYaxis()->SetNdivisions(202);
    frame_pulls->GetYaxis()->SetRangeUser(-4, 4);
    frame_pulls->GetXaxis()->SetTitle("m_{#mu#mu} [GeV]");
    frame_pulls->GetYaxis()->SetTitle("Pulls");
    frame_pulls->GetXaxis()->SetTitleSize(0.15);
    frame_pulls->GetYaxis()->SetTitleSize(0.15);
    frame_pulls->GetXaxis()->SetLabelSize(0.1);
    frame_pulls->GetYaxis()->SetLabelSize(0.1);
    frame_pulls->GetXaxis()->SetLabelOffset(0.01);
    frame_pulls->GetYaxis()->SetLabelOffset(0.01);
    frame_pulls->GetXaxis()->SetTitleOffset(1.2);
    frame_pulls->GetYaxis()->SetTitleOffset(0.3);
    frame_pulls->GetXaxis()->SetTickLength(0.1);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    frame_pulls->Draw();

    if(save)
    {
        canvas->SaveAs(("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/multi_fit_"+string(name)+"_V_dG.png").c_str());
        // canvas->SaveAs(("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/multi_fit_"+string(name)+"_V_dG.C").c_str());
    }
    // delete canvas;
    // delete frame;
    // delete frame_pulls;
    // delete hpull;
    // delete line;
    // delete flparams;
}