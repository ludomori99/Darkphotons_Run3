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
#include "CMS_lumi.h"
#include "tdrstyle.C"
#include "TH1.h"
#include "TH1F.h"

using namespace RooFit;
using namespace RooStats;
using namespace std;

vector<double> pulls(RooHist*, const char*);
void plot_model(RooWorkspace*, const char*, const char*, map<const char*,vector<Double_t>>*);
void scatter(map<const char*,vector<Double_t>>*, const char*);

void plot_fit(){ //fit all models in sequence 
   
   map<const char*, vector<Double_t>> *outputs = new map<const char*, vector<Double_t>>; //dictionary with: {model: {Nsig,Nbkg,chi2}}

   // std::unique_ptr<TFile> f_dCB{TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/Jpsi_data_dCB.root", "READ")};
   // auto ws_dCB = static_cast<RooWorkspace*>(f_dCB->FindObjectAny("ws_dCB"));  
   // ws_dCB->Print();
   // plot_model(ws_dCB,"dCB","/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/fit_Jpsi_dCB.png",outputs);

   // std::unique_ptr<TFile> f_dG{TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/Jpsi_data_dG.root", "READ")};
   // auto ws_dG = static_cast<RooWorkspace*>(f_dG->FindObjectAny("ws_dG"));
   // ws_dG->Print();
   // plot_model(ws_dG,"dG","/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/fit_Jpsi_dG.png",outputs);

   // std::unique_ptr<TFile> f_VG{TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/Jpsi_data_VG.root", "READ")};
   // auto ws_VG = static_cast<RooWorkspace*>(f_VG->FindObjectAny("ws_VG"));
   // ws_VG->Print();
   // plot_model(ws_VG,"VG","/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/fit_Jpsi_VG.png",outputs);

   std::unique_ptr<TFile> f_dCB_V{TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/Jpsi_data_dCB_V.root", "READ")};
   auto ws_dCB_V = static_cast<RooWorkspace*>(f_dCB_V->FindObjectAny("ws_dCB_V"));
   ws_dCB_V->Print();
   plot_model(ws_dCB_V,"dCB_V","/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/fit_Jpsi_dCB_V.png",outputs);


   for (const auto& pair : *outputs) {
        std::cout << "Key: " << pair.first << ", Values:";
        for (double value : pair.second) {
            std::cout << " " << value;
        }
        std::cout << std::endl;
    }

   scatter(outputs,"/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/fit_Jpsi_results.png" );
}


void plot_model(RooWorkspace* ws, const char* modelName, const char* fig_name, map<const char*,vector<Double_t>>* outputs )
{
   setTDRStyle();

   TCanvas *cdata = new TCanvas("fit", "fit demo", 2000, 1500);
   RooAbsPdf *sigModel = ws->pdf("sigModel");
   RooAbsPdf *bkgModel = ws->pdf("bkgModel");
   RooAbsPdf *massModel = ws->pdf(modelName);

   RooRealVar *Mm_mass = ws->var("Mm_mass");
   RooRealVar *sigYield = ws->var("sigYield");
   RooRealVar *bkgYield = ws->var("bkgYield");

   // RooRealVar a("mu", "J/Psi Mass", 3.09809, 0, 1);
   // RooRealVar b("sigma", "Width of Gaussian", 0.0378, 0.001, 10, "GeV");
   // RooGaussian *massModel = new RooGaussian("G", "Gaussian", *Mm_mass, a, b);

//    RooAbsPdf massModelNew = RooAbsPdf(massModel, "dCB");

    massModel->Print("v");


   auto& data = static_cast<RooDataSet&>(*ws->data("data"));
   cdata->Divide(1, 2);

   cdata->cd(1);
   RooPlot *frame = Mm_mass->frame(Title("Fit of model to discriminating variable"));
   data.plotOn(frame,XErrorSize(1), Name("data"), MarkerSize(1.3), DrawOption("PZ"));
   massModel->plotOn(frame, Name(modelName));
   massModel->plotOn(frame, Components(*sigModel), Name("SigModel"),DrawOption("F"),FillColor(3), FillStyle(3001), LineColor(0));
   massModel->plotOn(frame, Components(*bkgModel), LineStyle(7), LineColor(2), Name("BkgModel"));

   gPad->SetLeftMargin(0.99) ;
   gPad->SetBottomMargin(0.02);
   gPad->SetPad(0.01,0.3,0.99,0.99);
   frame->GetYaxis()->SetLabelSize(0.05);
   frame->GetYaxis()->SetTitleSize(0.05);
   frame->GetYaxis()->SetTitleOffset(0.8);
   frame->GetXaxis()->SetLabelSize(0.);
   frame->GetXaxis()->SetTitleSize(0.);
   frame->Draw();
   
   //compute chisquare
   RooAbsCollection *flparams = massModel->getParameters(data)->selectByAttrib("Constant", kFALSE);
   Int_t nflparams = flparams->getSize();
   cout << nflparams << "\n";
   Double_t chisquare = -1;
   chisquare = frame->chiSquare(modelName, "data", nflparams);
   // cout<<massModel->GetName()<<"\n"<<chisquare;
 
   TLegend leg(0.8, 0.6, 0.9,0.85);
   leg.AddEntry(frame->findObject("data"), "Data", "E2");
   leg.AddEntry(frame->findObject(modelName), "Full model", "l");
   leg.AddEntry(frame->findObject("SigModel"), "Signal model", "f");
   leg.AddEntry(frame->findObject("BkgModel"), "Bkg model", "l");
   leg.SetBorderSize(0);
   leg.SetFillStyle(0);
   leg.SetTextSize(0.04);
   leg.SetTextFont(42);
   leg.Draw();


   TPaveText *label_2 = new TPaveText(0.25, 0.63, 0.34, 0.78, "NDC");
   label_2->SetBorderSize(0);
   label_2->SetFillColor(0);
   label_2->SetTextSize(0.041);
   label_2->SetTextFont(42);
   gStyle->SetStripDecimals(kTRUE);
   label_2->SetTextAlign(11);
   TString sYield = to_string(int(round(sigYield->getValV())));
   TString bYield = to_string(int(round(bkgYield->getValV())));
   TString csquare = to_string(chisquare);
   label_2->AddText("N_{sig} = " + sYield);
   label_2->AddText("N_{bkg} = " + bYield);
   label_2->AddText("#chi^{2} = " + csquare);
   label_2->Draw();


   TPaveText *cms = new TPaveText(0.14, 0.972, 0.3, 0.98, "NDC");
   cms->AddText("CMS preliminary");
   cms->SetBorderSize(0);
   cms->SetFillColor(0);
   cms->SetTextSize(0.04);
   cms->SetTextFont(42);
   cms->Draw();
   

   //plot pulls
   cdata->cd(2);
   RooHist *hpull = frame->pullHist("data", modelName); //massModel->GetName());
   RooPlot *frame2 = Mm_mass->frame(Title("Pull"));
   TLine* line = new TLine(Mm_mass->getMin(), 0, Mm_mass->getMax(), 0);
   line->SetLineColor(kBlue);
   frame2->addObject(line);
   frame2->addPlotable(hpull, "P"); //,"E3")
   frame2->SetMarkerStyle(2);
   frame2->SetMarkerSize(0.01);

   gPad->SetLeftMargin(0.15);
   gPad->SetPad(0.01, 0.01, 0.99, 0.3);
   gPad->SetTopMargin(0.01);
   gPad->SetBottomMargin(0.5); 
   frame2->GetYaxis()->SetNdivisions(202);
   frame2->GetYaxis()->SetRangeUser(-4, 4);
   frame2->GetXaxis()->SetTitle("m_{#mu#mu} [GeV]");
   frame2->GetYaxis()->SetTitle("Pulls");
   frame2->GetXaxis()->SetTitleSize(0.1);
   frame2->GetYaxis()->SetTitleSize(0.1);
   frame2->GetXaxis()->SetLabelSize(0.08);
   frame2->GetYaxis()->SetLabelSize(0.08);
   frame2->GetXaxis()->SetLabelOffset(0.01);
   frame2->GetYaxis()->SetLabelOffset(0.01);
   frame2->GetYaxis()->SetTitleOffset(0.1);
   frame2->GetXaxis()->SetTickLength(0.1);
   gPad->SetFrameFillColor(0);
   gPad->SetFrameBorderMode(0);
   gPad->SetFrameFillColor(0);
   gPad->SetFrameBorderMode(0);
   frame2->Draw();


   //  size_t length = strlen(fig_name);
   //  size_t newLength = length - 4;
   char* name = new char[strlen(fig_name) -3 ];
   strncpy(name, fig_name, strlen(fig_name) -4);

   cout<<name;

   string s = string(name) + string("_pulls.png");
   string* figure_name_str = new string(s);
   const char* figure_name = figure_name_str->c_str();

   vector<double> pulls_fit=pulls(hpull,figure_name);


   // CMS_lumi(cdata,4,0);

   cdata->Update();
   cdata->SaveAs(fig_name);
   cout << "\n\n saving file as " << fig_name << "\n\n";

   (*outputs)[modelName]={Double_t(round(sigYield->getValV())),Double_t(round(bkgYield->getValV())), chisquare};

   return;

}

vector<double> pulls(RooHist* hpull, const char* outfilename){
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
   // cout<< fgauss->GetChisquare() / fgauss->GetNDF()<<"\n"<<fgauss->GetParameter(1)<<"\n"<<fgauss->GetParameter(2) ;

   TPaveText* cms1 = new TPaveText(0.14,0.972,0.3,0.98, "NDC");
   cms1->AddText("CMS preliminary");
   cms1->SetBorderSize(0);
   cms1->SetFillColor(0);
   cms1->SetTextSize(0.04);
   cms1->SetTextFont(42);
   cms1->SetTextAlign(11);
   //cms->SetTextStyle(2);
   cms1->Draw();

   TLegend* leg3 = new TLegend(0.65, 0.7, 0.9,0.85);
   leg3->AddEntry("hist_pull", "Entries", "l");
   leg3->AddEntry("fgauss", "Gaussian fit", "l");
   leg3->SetTextSize(0.04);
   leg3->SetTextFont(42);
   leg3->Draw();

   canv_pull->SaveAs(outfilename);
   double redchisq = fgauss->GetChisquare()/fgauss->GetNDF();
   return vector{redchisq,fgauss->GetParameter(1),fgauss->GetParameter(2)};

}


void scatter(map<const char*, vector<Double_t>>* data, const char* outfilename) {
   cout<<"check";
   TCanvas canvas("canvas", "CMS Scatter Plot", 800, 600);
   TGraph graph(data->size());
   int i = 0;
   for (const auto& entry : *data) {
      const vector<double>& coordinates = entry.second;
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
   int j=0;
   for (const auto& entry : *data) {
      const vector<double>& coordinates = entry.second;
      if (coordinates.size() >= 2) {
         float displ = 1 - (j==3)*3;
         label.DrawLatex(coordinates[0] + displ, coordinates[2] + displ, entry.first);
      }
      j++;
   }


   gPad->SetLeftMargin(0.08) ;
   gPad->SetBottomMargin(0.1);
   gPad->SetPad(0.01,0.1,0.99,0.99);

   graph.GetXaxis()->SetTitle("Signal yield");
   graph.GetYaxis()->SetTitle("Reduced #chi^{2}");
   graph.GetXaxis()->SetLabelSize(0.05);
   graph.GetYaxis()->SetLabelSize(0.05);
   graph.GetXaxis()->SetTitleFont(42);
   graph.GetYaxis()->SetTitleFont(42);
   graph.GetXaxis()->SetTitleOffset(0.8);
   graph.GetYaxis()->SetTitleOffset(0.8);
   graph.GetXaxis()->SetLabelFont(42);
   graph.GetYaxis()->SetLabelFont(42); 

   graph.GetXaxis()->SetTitleSize(0.05);
   graph.GetYaxis()->SetTitleSize(0.05);

   // graph.GetXaxis()->SetTitleSize(0.05);
   graph.GetYaxis()->SetRangeUser(-2,100);

   TLatex cmsLabel;
   cmsLabel.SetTextSize(0.04);
   cmsLabel.SetTextFont(42);
   cmsLabel.DrawLatexNDC(0.05, 0.97, "CMS Preliminary");

   canvas.Update();
   canvas.SaveAs(outfilename);

}