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

#include "tdrstyle.C"
#include "TH1.h"
#include "TH1F.h"

using namespace RooFit;
using namespace RooStats;
using namespace std;

void plot_fit( const char* fig_name)
{

    std::unique_ptr<TFile> f{TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/Jpsi_data1M.root", "READ")};
    auto ws = static_cast<RooWorkspace*>(f->FindObjectAny("myWS"));
    ws->Print();

   // setTDRStyle();

   TCanvas *cdata = new TCanvas("fit", "fit demo", 2000, 1500);
   RooAbsPdf *sigModel = ws->pdf("sigModel");
   RooAbsPdf *bkgModel = ws->pdf("bkgModel");
   RooAbsPdf *massModel = ws->pdf("massModel");

   RooRealVar *Mm_mass = ws->var("Mm_mass");
 
   RooRealVar *sigYield = ws->var("sigYield");
   RooRealVar *bkgYield = ws->var("bkgYield");

   auto& data = static_cast<RooDataSet&>(*ws->data("data"));
//    auto& data = ws->data("Mm_mass");
   
//    cdata->Divide(1, 2);

//    cdata->cd(1);
//    RooPlot *frame = Mm_mass->frame(Title("Fit of model to discriminating variable"));
//    data->plotOn(frame,XErrorSize(1), Name("Data"), MarkerSize(0.6), DrawOption("PZ"));
//    massModel->plotOn(frame, Name("FullModel"));
//    massModel->plotOn(frame, Components(*sigModel), Name("SigModel"),DrawOption("F"),FillColor(3), FillStyle(3001), LineColor(0));
//    massModel->plotOn(frame, Components(*bkgModel), LineStyle(7), LineColor(7), Name("BkgModel"));
   
//    //compute chisquare
//    RooAbsCollection *flparams = massModel->getParameters(*data)->selectByAttrib("Constant", kFALSE);
//    Int_t nflparams = flparams->getSize();
//    cout << nflparams << "\n";
//    Double_t chisquare = -1;
//    chisquare = 1; //frame->chiSquare("massModel", "data", nflparams);
 
//    TLegend leg(0.8, 0.6, 0.9,0.85);
//    leg.AddEntry(frame->findObject("FullModel"), "Full model", "l");
//    leg.AddEntry(frame->findObject("SigModel"), "Signal model", "l");
//    leg.AddEntry(frame->findObject("BkgModel"), "Bkg model", "l");
//    leg.SetBorderSize(0);
//    leg.SetFillStyle(0);
//    leg.SetTextSize(0.04);
//    leg.SetTextFont(42);
//    leg.Draw();

//    TPaveText *label_2 = new TPaveText(0.55, 0.23, 0.8, 0.50, "NDC");
//    label_2->SetBorderSize(0);
//    label_2->SetFillColor(0);
//    label_2->SetTextSize(0.041);
//    label_2->SetTextFont(42);
//    gStyle->SetStripDecimals(kTRUE);
//    label_2->SetTextAlign(11);
//    TString sYield = to_string(int(round(sigYield->getValV())));
//    TString bYield = to_string(int(round(bkgYield->getValV())));
//    label_2->AddText("N_{sig} = " + sYield);
//    label_2->AddText("N_{bkg} = " + bYield);
//    label_2->Draw();

//    TPaveText *cms = new TPaveText(0.14, 0.922, 0.3, 0.93, "NDC");
//    cms->AddText("CMS preliminary");
//    cms->SetBorderSize(0);
//    cms->SetFillColor(0);
//    cms->SetTextSize(0.04);
//    cms->SetTextFont(42);
//    cms->Draw();
   
//    gPad->SetLeftMargin(0.15) ;
//    gPad->SetBottomMargin(0.03);
//    gPad->SetPad(0.01,0.2,0.99,0.99);
//    frame->GetYaxis()->SetTitleSize(0.04);
//    frame->GetYaxis()->SetTitleOffset(1.2);
//    frame->GetXaxis()->SetLabelSize(0.);
//    frame->GetXaxis()->SetTitleSize(0.);
//    frame->Draw();

//    //plot pulls
//    cdata->cd(2);
//    // RooHist *hpull = frame->pullHist("data", "massModel");
//    RooPlot *frame2 = Mm_mass->frame(Title("Pull"));
//    // frame2->addPlotable(hpull, "P"); //,"E3");
//    frame2->SetMarkerStyle(2);
//    frame2->SetMarkerSize(0.01);

//    gPad->SetLeftMargin(0.15);
//    gPad->SetPad(0.01, 0.01, 0.99, 0.2);
//    gPad->SetTopMargin(0.1);
//    gPad->SetBottomMargin(0.5); 
//    frame2->GetYaxis()->SetNdivisions(202);
//    frame2->GetYaxis()->SetRangeUser(-4, 4);
//    frame2->GetXaxis()->SetTitle("m_{#mu#mu} [GeV]");
//    frame2->GetYaxis()->SetTitle("Pulls");
//    frame2->GetXaxis()->SetTitleSize(0.2);
//    frame2->GetYaxis()->SetTitleSize(0.2);
//    frame2->GetXaxis()->SetLabelSize(0.15);
//    frame2->GetYaxis()->SetLabelSize(0.15);
//    frame2->GetXaxis()->SetLabelOffset(0.01);
//    frame2->GetYaxis()->SetLabelOffset(0.01);
//    frame2->GetYaxis()->SetTitleOffset(0.2);
//    frame2->GetXaxis()->SetTickLength(0.1);
//    gPad->SetFrameFillColor(0);
//    gPad->SetFrameBorderMode(0);
//    gPad->SetFrameFillColor(0);
//    gPad->SetFrameBorderMode(0);
//    frame2->Draw();

//    cdata->SaveAs(fig_name);
//    cout << "\n\n saving file as " << fig_name << "\n\n";
}