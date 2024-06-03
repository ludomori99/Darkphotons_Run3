


void fits(){

   TCanvas* canvas  = new TCanvas("42","42",800,800);
    setTDRStyle();
    canvas->Divide(1,2);
    canvas->cd(1);
    RooRealVar Mm_mass("Mm_mass", "Mm_mass", 2.6, 3.56);

    RooPlot *frame = Mm_mass.frame(RooFit::Title("Invariant Mass"));

    frame->SetTitle("42");
    gPad->SetLeftMargin(0.15);
    gPad->SetBottomMargin(0.02);

    gPad->SetTopMargin(0.09);
    
    gPad->SetPad(0.01,0.3,0.99,0.99);
    frame->GetYaxis()->SetLabelSize(0.05);
    frame->GetYaxis()->SetTitleSize(0.058);
    frame->GetYaxis()->SetTitleOffset(1.1);
    frame->GetXaxis()->SetLabelSize(0.);
    frame->GetXaxis()->SetTitleSize(0.);

    TLegend leg(0.75, 0.6, 0.85,0.85);
    leg.SetBorderSize(0);
    leg.SetFillStyle(0);
    leg.SetTextSize(0.04);
    leg.SetTextFont(42);

    //compute chisquare
   //  RooAbsCollection *flparams = model->getParameters(Data)->selectByAttrib("Constant", kFALSE);
   //  Int_t nflparams = flparams->getSize();
    Double_t chisquare = -1;
   //  chisquare = frame->chiSquare("Full model", "data", nflparams);

    TPaveText label_2(0.25, 0.63, 0.34, 0.82, "NDC");
    label_2.SetBorderSize(0);
    label_2.SetFillColor(0);
    label_2.SetTextSize(0.041);
    label_2.SetTextFont(42);
    gStyle->SetStripDecimals(kTRUE);
    label_2.SetTextAlign(11);
    TString csquare = to_string(chisquare);
    label_2.AddText("#chi^{2} = " + csquare);

    frame->Draw();
    label_2.Draw();     
    leg.Draw();

    CMS(canvas);

   
    canvas->cd(2);
    RooPlot *frame_pulls = Mm_mass.frame(RooFit::Title("Pull"));
    TLine* line = new TLine(Mm_mass.getMin(), 0, Mm_mass.getMax(), 0);
    line->SetLineColor(kBlue);
    frame_pulls->addObject(line);
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
    frame_pulls->GetXaxis()->SetTitleSize(0.13);
    frame_pulls->GetYaxis()->SetTitleSize(0.1);
    frame_pulls->GetXaxis()->SetLabelSize(0.1);
    frame_pulls->GetYaxis()->SetLabelSize(0.1);
    frame_pulls->GetXaxis()->SetLabelOffset(0.01);
    frame_pulls->GetYaxis()->SetLabelOffset(0.01);
    frame_pulls->GetXaxis()->SetTitleOffset(1.15);
    frame_pulls->GetYaxis()->SetTitleOffset(0.3);
    frame_pulls->GetXaxis()->SetTickLength(0.1);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    frame_pulls->Draw();
   bool save=true;
    if(save)
    {
        canvas->SaveAs("/work/submit/mori25/Darkphotons_ludo/offline_analysis/sandbox.png");
    }
}

void effs(){

   TCanvas* c1 = new TCanvas("Comparison","Comparison",1200,900);
   setTDRStyle();
   c1->SetMargin(0.15, 0.03, 0.14, 0.9);

   gPad->Update();
   double y[6] = {3, 8, 1, 10, 5, 7};
   auto graph = new TGraph(6,y);
   graph->SetMinimum(0.0);
   graph->GetYaxis()->SetTitleOffset(0.85);
   graph->SetMaximum(1.2);
   gPad->Update();

   graph->GetHistogram()->GetXaxis()->SetRangeUser(0.,80.);
   graph->SetMinimum(0.86);
   graph->SetMaximum(1.02);


   //Label
   TPaveText label(0.68, 0.7, 0.94, 0.75, "NDC");
   label.SetBorderSize(0);
   label.SetFillColor(0);
   label.SetTextSize(0.031);
   label.SetTextFont(42);
   gStyle->SetStripDecimals(kTRUE);
   label.SetTextAlign(11);
   label.AddText("Probe");
   label.Draw();     

   CMS_single(c1,string("62.4"),string("13.6"));

   bool save=true;
   if(save)
   {
      c1->SaveAs("/work/submit/mori25/Darkphotons_ludo/offline_analysis/sandbox.png");
   }
}

void scatter(map<string, vector<double>>* data, const char* outfilename) {
   TCanvas canvas("canvas", "CMS Scatter Plot", 800, 600);
   TGraph graph(data->size());
   int i = 0;
   for (const auto& entry : *data) {
      const std::vector<double>& coordinates = entry.second;
      if (coordinates.size() >= 2) {
         graph.SetPoint(i, coordinates[0], coordinates[2]);
      }
      i++;
   }
   graph.SetMarkerStyle(20);
   graph.SetMarkerColor(kBlue);
   graph.Draw("AP");

   TLatex label;
   label.SetTextSize(0.03);
   label.SetTextFont(42);
   for (const auto& entry : *data) {
      const std::vector<double>& coordinates = entry.second;
      if (coordinates.size() >= 2) {
         label.DrawLatex(coordinates[0] + 0.1, coordinates[2] + 0.1, entry.first.c_str());
      }
   }

   graph.GetXaxis()->SetTitle("X");
   graph.GetYaxis()->SetTitle("Y");

   graph.GetXaxis()->SetLabelSize(0.04);
   graph.GetYaxis()->SetLabelSize(0.04);

   graph.GetXaxis()->SetTitleFont(42);
   graph.GetYaxis()->SetTitleFont(42);

   graph.GetXaxis()->SetTitleOffset(1.2);
   graph.GetYaxis()->SetTitleOffset(1.2);

   graph.GetXaxis()->SetLabelFont(42);
   graph.GetYaxis()->SetLabelFont(42);

   graph.GetXaxis()->SetTitleSize(0.05);
   graph.GetYaxis()->SetTitleSize(0.05);

   TLatex cmsLabel;
   cmsLabel.SetTextSize(0.04);
   cmsLabel.SetTextFont(42);
   cmsLabel.DrawLatexNDC(0.15, 0.92, "CMS Preliminary");

   canvas.Update();
   canvas.SaveAs(outfilename);

}

void sandbox(){
   effs();
}