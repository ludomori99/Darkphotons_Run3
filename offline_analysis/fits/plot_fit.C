#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/CMS.C"
#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/setTDRStyle.C"
#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/fit_with_pulls.C"
// #include "RooRealVar.h"
// #include "RooDataSet.h"
// #include "RooRealVar.h"
// #include "RooGaussian.h"
// #include "RooExponential.h"
// #include "RooBernstein.h"
// #include "RooChebychev.h"
// #include "RooAddPdf.h"
// #include "RooProdPdf.h"
// #include "RooAddition.h"
// #include "RooProduct.h"
// #include "RooAbsPdf.h"
// #include "RooFitResult.h"
// #include "RooWorkspace.h"
// #include "RooConstVar.h"
// #include "TCanvas.h"
// #include "TLegend.h"
// #include <iomanip>
// #include "TH1.h"
// #include "TH1F.h"

using namespace RooFit;
using namespace RooStats;
using namespace std;

vector<double> pulls(RooHist*, string);
void plot_model(RooWorkspace*, const char*, const char*, map<const char*,vector<Double_t>>*);
void scatter(map<const char*,vector<Double_t>>*, const char*);

void plot_fit(){ //fit all models in sequence 
   
   map<const char*, vector<Double_t>> *outputs = new map<const char*, vector<Double_t>>; //dictionary with: {model: {Nsig,Nbkg,chi2}}

   std::unique_ptr<TFile> f_dCB{TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/Jpsi_data_dCB.root", "READ")};
   auto ws_dCB = static_cast<RooWorkspace*>(f_dCB->FindObjectAny("ws_dCB"));  
   ws_dCB->Print();
   plot_model(ws_dCB,"dCB","/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/fit_Jpsi_dCB",outputs);

   // std::unique_ptr<TFile> f_dG{TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/Jpsi_data_dG.root", "READ")};
   // auto ws_dG = static_cast<RooWorkspace*>(f_dG->FindObjectAny("ws_dG"));
   // ws_dG->Print();
   // plot_model(ws_dG,"dG","/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/fit_Jpsi_dG",outputs);

   // std::unique_ptr<TFile> f_VG{TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/Jpsi_data_VG.root", "READ")};
   // auto ws_VG = static_cast<RooWorkspace*>(f_VG->FindObjectAny("ws_VG"));
   // ws_VG->Print();
   // plot_model(ws_VG,"VG","/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/fit_Jpsi_VG",outputs);

   // std::unique_ptr<TFile> f_dCB_V{TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/Jpsi_data_dCB_V.root", "READ")};
   // auto ws_dCB_V = static_cast<RooWorkspace*>(f_dCB_V->FindObjectAny("ws_dCB_V"));
   // ws_dCB_V->Print();
   // plot_model(ws_dCB_V,"dCB_V","/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/fit_Jpsi_dCB_V",outputs);

   // std::unique_ptr<TFile> f_dCB_V_var{TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/Jpsi_data_dCB_V_var.root", "READ")};
   // auto ws_dCB_V_var = static_cast<RooWorkspace*>(f_dCB_V_var->FindObjectAny("ws_dCB_V_var"));
   // ws_dCB_V_var->Print();
   // plot_model(ws_dCB_V_var,"dCB_V_var","/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/fit_Jpsi_dCB_V_var",outputs);


   // for (const auto& pair : *outputs) {
   //      std::cout << "Key: " << pair.first << ", Values:";
   //      for (double value : pair.second) {
   //          std::cout << " " << value;
   //      }
   //      std::cout << std::endl;
   //  }

   // scatter(outputs,"/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/fit_Jpsi_results.png" );
}


void plot_model(RooWorkspace* ws, const char* modelName, const char* fig_name, map<const char*,vector<Double_t>>* outputs )
{
   setTDRStyle();

   RooAbsPdf *sigModel = ws->pdf("sigModel");
   RooAbsPdf *bkgModel = ws->pdf("bkgModel");
   RooAbsPdf *massModel = ws->pdf(modelName);

   RooRealVar *Mm_mass = ws->var("Mm_mass");
   RooRealVar *sigYield = ws->var("sigYield");
   RooRealVar *bkgYield = ws->var("bkgYield");
   auto& data = static_cast<RooDataSet&>(*ws->data("data"));

   //Store logs
   string logpath =  "/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/models/" + string(modelName) + "_output.log";
   ofstream logFile((logpath).c_str());
   logFile << "sigYield=" << sigYield->getValV()<<"\n";
   cout << logpath << " sigYield=" << sigYield->getValV()<<"\n";

   // RooRealVar a("mu", "J/Psi Mass", 3.09809, 0, 1);
   // RooRealVar b("sigma", "Width of Gaussian", 0.0378, 0.001, 10, "GeV");
   // RooGaussian *massModel = new RooGaussian("G", "Gaussian", *Mm_mass, a, b);

//    RooAbsPdf massModelNew = RooAbsPdf(massModel, "dCB");

    massModel->Print("v");

   plot("", *Mm_mass, &data, massModel, *sigYield, *bkgYield, logFile,(string(fig_name)+".png").c_str(), true,true);

  
   // string s = string(fig_name) + string("_pulls.png");
   // vector<double> pulls_fit=pulls(hpull,s);

   // cdata->Update();
   // cdata->SaveAs((string(fig_name)+".png").c_str());
   // cout << "\n\n saving file as " << fig_name << ".png\n\n";

   // (*outputs)[modelName]={Double_t(round(sigYield->getValV())),Double_t(round(bkgYield->getValV())), chisquare};

   return;

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
         float displ = 1.2 - (j==3)*3;
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
   graph.GetXaxis()->SetTitleOffset(1);
   graph.GetYaxis()->SetTitleOffset(1);
   graph.GetXaxis()->SetLabelFont(42);
   graph.GetYaxis()->SetLabelFont(42); 

   graph.GetXaxis()->SetTitleSize(0.05);
   graph.GetYaxis()->SetTitleSize(0.05);

   // graph.GetXaxis()->SetTitleSize(0.05);
   graph.GetYaxis()->SetRangeUser(-2,20);

   TLatex cmsLabel;
   cmsLabel.SetTextSize(0.04);
   cmsLabel.SetTextFont(42);
   cmsLabel.DrawLatexNDC(0.07, 0.955, "CMS Preliminary");

   canvas.Update();
   canvas.SaveAs(outfilename);

}