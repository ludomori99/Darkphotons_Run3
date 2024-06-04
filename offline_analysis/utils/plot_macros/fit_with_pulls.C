using namespace RooFit;

vector<double> pulls(RooHist* hpull, string outfilename);

void plot(const char* name, RooRealVar Mm_mass, RooDataSet* Data, RooAbsPdf* model, RooRealVar n_signal, RooRealVar n_back, ofstream& logFile, string path="", bool save=true, bool fit_pulls = false){
    TCanvas* canvas  = new TCanvas(name,name,800,800);
    setTDRStyle();

    canvas->Divide(1,2);
    canvas->cd(1);

    RooPlot *frame = Mm_mass.frame(RooFit::Title("Invariant Mass"));

    frame->SetTitle(name);
    frame->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");

    Data->plotOn(frame,XErrorSize(1), Name("data"), MarkerSize(1.), DrawOption("PZ"));
    model->plotOn(frame,Name("Full model"));
    model->plotOn(frame,Components(("sigModel"+string(name)).c_str()),Name("Signal model"),DrawOption("F"),FillColor(3), FillStyle(3001), LineColor(0));
    model->plotOn(frame,Components(("bkgModel"+string(name)).c_str()),Name("Background model"),LineStyle(7), LineColor(2));

    gPad->SetTopMargin(0.075);
    gPad->SetLeftMargin(0.15);
    gPad->SetBottomMargin(0.02);
    gPad->SetPad(0.01,0.27,0.99,0.99);
    frame->GetYaxis()->SetLabelSize(0.05);
    frame->GetYaxis()->SetTitleSize(0.058);
    frame->GetYaxis()->SetTitleOffset(1.2);
    frame->GetXaxis()->SetLabelSize(0.);
    frame->GetXaxis()->SetTitleSize(0.);

    TLegend leg(0.7, 0.6, 0.85,0.85);
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
    TString csquare = to_string(0.01*float(int(round(100*chisquare)))).substr(0,4); 
    label_2.AddText("N_{sig} = " + sYield);
    label_2.AddText("N_{bkg} = " + bYield);
    label_2.AddText("#chi^{2}_{red} = " + csquare);
    logFile << "\nchisquare=" << chisquare<<"\n";

    frame->Draw();
    label_2.Draw();     
    leg.Draw();

    CMS(canvas,string("62.4"),string("13.6"));

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
    gPad->SetTopMargin(0.04);
    gPad->SetBottomMargin(0.4); 
    gPad->SetPad(0.01, 0.01, 0.99, 0.25);
    frame_pulls->GetYaxis()->SetNdivisions(202);
    frame_pulls->GetYaxis()->SetRangeUser(-4, 4);
    frame_pulls->GetXaxis()->SetTitle("m_{#mu#mu} [GeV]");
    frame_pulls->GetYaxis()->SetTitle("Pulls");
    frame_pulls->GetXaxis()->SetTitleSize(0.16);
    frame_pulls->GetYaxis()->SetTitleSize(0.16);
    frame_pulls->GetXaxis()->SetLabelSize(0.14);
    frame_pulls->GetYaxis()->SetLabelSize(0.14);
    frame_pulls->GetXaxis()->SetLabelOffset(0.02);
    frame_pulls->GetYaxis()->SetLabelOffset(0.02);
    frame_pulls->GetXaxis()->SetTitleOffset(1.1);
    frame_pulls->GetYaxis()->SetTitleOffset(0.25);
    frame_pulls->GetXaxis()->SetTickLength(0.1);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    frame_pulls->Draw();

    if(save)
    {
        canvas->SaveAs(path.c_str());
    }

    if (fit_pulls)
    {
        pulls(hpull, path+"_pulls.png");
    }

    delete canvas;
}

vector<double> pulls(RooHist* hpull, string outfilename){
   //pull distribution  
   TCanvas* canv_pull = new TCanvas("canv_pull", "canv_pull", 800,700);
   TH1D* hist_pull = new TH1D("hist_pull","hist_pull",120,-5,5);

   for(Int_t i = 0; i<hpull->GetN();i++){
            Double_t x, point;
            hpull->GetPoint(i,x,point);
            hist_pull->Fill(point);
   }
   
   hist_pull->Rebin(4);
   //hist_pull->SetMarkerStyle(4);
   //hist_pull->SetMarkerSize(2);
   TAxis* Xaxis = hist_pull->GetXaxis();
   TAxis* Yaxis = hist_pull->GetYaxis();
   Xaxis->SetTitle("Residual value");
   Xaxis->SetTitleSize(0.045);
   Xaxis->SetLabelSize(0.045);
   Xaxis->SetTitleOffset(1.1);
   Yaxis->SetTitle("Frequency");
   Yaxis->SetTitleSize(0.045);
   Yaxis->SetLabelSize(0.045);
   Yaxis->SetTitleOffset(0.8);
   //gStyle->SetOptStat(0);
   hist_pull->Draw();

   //fit it

   TF1* fgauss = new TF1("fgauss","gaus",-5,5);
   fgauss->SetLineColor(4);
   hist_pull->Fit("fgauss");
   fgauss->Draw("same");
   gStyle->SetOptFit(2);

   TLegend* leg3 = new TLegend(0.65, 0.7, 0.9,0.85);
   leg3->AddEntry("hist_pull", "Entries", "l");
   leg3->AddEntry("fgauss", "Gaussian fit", "l");
   leg3->SetTextSize(0.04);
   leg3->SetTextFont(42);
   leg3->Draw();

   canv_pull->SaveAs(outfilename.c_str());
   double redchisq = fgauss->GetChisquare()/fgauss->GetNDF();
   return vector{redchisq,fgauss->GetParameter(1),fgauss->GetParameter(2)};

}