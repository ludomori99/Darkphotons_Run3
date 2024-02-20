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

void scatter(map<string, vector<double>>*, const char* );

void sandbox(){
    map<string, vector<double>>* data = new map<string, vector<double>>;
    (*data)["hi"]=vector{1.2,2.0,3.};
    cout<<"check 1";
    scatter(data,"hi.png");
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