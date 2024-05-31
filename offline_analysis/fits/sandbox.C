#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooExponential.h"
#include "RooBernstein.h"
#include "RooChebychev.h"
#include "RooAddPdf.h"
#include "RooProdPdf.h"
#include "RooAddition.h"
#include "RooProduct.h"
#include "RooAbsPdf.h"
#include "RooFitResult.h"
#include "RooWorkspace.h"
#include "RooConstVar.h"
#include "TCanvas.h"
#include "TLegend.h"
#include <iomanip>
#include "CMS.C"
#include "TH1.h"
#include "TH1F.h"

using namespace RooFit;
using namespace RooStats;
using namespace std;

void plot(bool save=true);
void scatter(map<string, vector<double>>*, const char* );

void sandbox(){

   plot();
   //  map<string, vector<double>>* data = new map<string, vector<double>>;
   //  (*data)["hi"]=vector{1.2,2.0,3.};
   //  scatter(data,"hi.png");

   // vector<double> bins; 
   //  int bin_n;

   //  if (true){
   //      double bins_tmp[] =  {2, 4, 4.2, 4.4, 4.7, 5.0, 5.1, 5.2, 5.4, 5.5, 5.6, 5.7, 5.8, 5.9, 6.2, 6.4, 6.6, 6.8, 7.3, 9.5, 13.0, 17.0, 40}; // pt
   //      bin_n = 22;
   //      bins.assign(bins_tmp, bins_tmp + bin_n + 1);
   //  }

   //  cout<<bins[12]<<"\n";
   //  cout<<bins[22];
}
void plot(bool save=true){
   TCanvas* canvas  = new TCanvas("42","42",800,800);
    setTDRStyle();

    canvas->Divide(1,2);
    canvas->cd(1);
    RooRealVar Mm_mass("Mm_mass", "Mm_mass", 2.6, 3.56);

    RooPlot *frame = Mm_mass.frame(RooFit::Title("Invariant Mass"));

    frame->SetTitle("42");
   //  frame->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");

   //  Data->plotOn(frame,XErrorSize(1), Name("data"), MarkerSize(1.), DrawOption("PZ"));


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
    RooPlot *frame_pulls = Mm_mass.frame(Title("Pull"));
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

    if(save)
    {
        canvas->SaveAs("/work/submit/mori25/Darkphotons_ludo/offline_analysis/sandbox.png");
        // canvas->SaveAs(("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/multi_fit_"+string(name)+"_V_dG.C").c_str());
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