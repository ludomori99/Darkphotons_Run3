#include "RooRealVar.h"
#include "RooStats/SPlot.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooExponential.h"
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
 
// use this order for safety on library loading
using namespace RooFit;
using namespace RooStats;
using namespace std;
 
// see below for implementationbelow for implementatio
void load_config(const char* ,bool, const char*&, const char*&, const char*&, const char*&, Double_t&, Double_t&, const char*);
void AddModelJ(RooWorkspace &, bool,  Double_t, Double_t);
void AddModelY(RooWorkspace &, bool,  Double_t, Double_t);
void AddModelYMC(RooWorkspace &, bool,  Double_t, Double_t);
void AddData(RooWorkspace &, const char*, Double_t, Double_t, int);
void DoSPlot(RooWorkspace &, const char*);
void SaveData(RooWorkspace &, const char*);
void MakePlots(RooWorkspace &, const char*);



void sPlot(const char* meson, bool isMC, int nEntries = 1000000)  // "Jpsi" or "Y"
{

   // Create a workspace to manage the project.
   RooWorkspace wspace{"myWS"};
  
   const char* file_name;
   const char* fig_name;
   const char* model_name; 
   const char* inputfilename;
   Double_t lowRange;
   Double_t highRange;

   load_config(meson, isMC, file_name, fig_name, model_name, inputfilename, lowRange, highRange, "1M");

   if (string(meson) == "Jpsi") AddModelJ(wspace,isMC, lowRange, highRange);
   else if (string(meson) == "Y"){
      if (isMC) AddModelYMC(wspace,isMC, lowRange, highRange);
      else AddModelY(wspace,isMC, lowRange, highRange);
   }

   AddData(wspace, inputfilename, lowRange, highRange, nEntries);
 
   // inspect the workspace if you wish
   // wspace.Print();

   // This will make a new dataset with sWeights added for every event.
   DoSPlot(wspace,model_name);
   SaveData(wspace, file_name);
   MakePlots(wspace, fig_name);
}



void load_config(const char* meson, bool isMC,
               const char*& file_name,
               const char*& fig_name,
               const char*& model_name, 
               const char*& inputfilename, 
               Double_t& lowRange,
               Double_t& highRange, 
               const char* extra = "")
{
   const char** data_or_MC = new const char*;
   if (isMC) *data_or_MC = "_MC";
   else *data_or_MC = "_data";

   string s = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/sPlot/files/data_withSWeight_") + string(meson) + string(*data_or_MC) + string(extra) + string(".root");
   string* file_name_str = new string(s);
   file_name = file_name_str->c_str();

   s = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/sPlot/figures/SPlot") + string(meson) + string(*data_or_MC) + string(extra) + string(".gif");
   string* fig_name_str = new string(s);
   fig_name = fig_name_str->c_str();
   
   s = (string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/sPlot/models/SPlot") + string(meson) + string("_fit") + string(*data_or_MC) + string(extra) + string(".dot")).c_str();
   string* model_name_str = new string(s); 
   model_name = model_name_str->c_str();

   if (string(meson) == string("Jpsi")) {
      lowRange = 2.6;
      highRange = 3.56;
      if (isMC) inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC/Jpsi/merged.root"; //JPsi MC
      else inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Jpsi/merged_A.root"; //Jpsi data
   }
   else if(string(meson) == string("Y")){
      lowRange = 8.5;
      highRange = 11.2;
      if (isMC) inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC/Y/merged.root"; //Upsilon MC
      else inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Y/merged_A.root"; //Upsilon data
   }
   else {cout<<"type of meson not recognized";}

}
 
//____________________________________
void AddModelJ(RooWorkspace &ws, bool isMC,  Double_t lowRange, Double_t highRange)
{
   RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange, "GeV");
   RooRealVar forest_standard_Y_mva("forest_standard_Y_mva", "forest_standard_Y_mva", 0., 1., "");

   // mass model for single resonance. Gauss + dCB. In this case employed for Jpsi
   RooRealVar mu("mu", "J/Psi Mass", 3.09809, lowRange, highRange);
   RooRealVar sigma("sigma", "Width of Gaussian", 0.0378, 0.001, 10, "GeV");
   RooGaussian Gaussian("Gaussian", "Gaussian", Mm_mass, mu, sigma);

   RooRealVar sigmaL("sigmaL", "Width of left CB", 0.01956, 0.001, 10, "GeV");
   RooRealVar sigmaR("sigmaR", "Width of right CB", 0.01957, 0.001, 11, "GeV");
   RooRealVar nL("nL", "nL CB", 0.8, 0.1,15, "");
   RooRealVar alphaL("alphaR", "Alpha right CB", 2.5, 0.001, 5, "");
   RooRealVar nR("nR", "nR CB", 0.3, 0.1,15, "");
   RooRealVar alphaR("alphaL", "Alpha left CB", 1.7, 0.001, 5, "");
   RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigmaL, sigmaR, alphaL,nL,alphaR,nR);

   RooRealVar GaussFraction("GaussFraction", "Fraction of Gaussian", 0.5, 0, 1, "");

   // Final model is sigModel
   RooAddPdf sigModel("sigModel", "J/psi mass model", RooArgList(Gaussian, CB), GaussFraction);
   // --------------------------------------
   // make bkg model
 
   std::cout << "make bkg model" << std::endl;
   RooRealVar bkgDecayConst("bkgDecayConst", "Decay const for bkg mass spectrum", -2., -100, 100, "1/GeV");
   RooExponential bkgModel("bkgModel", "bkg Mass Model", Mm_mass, bkgDecayConst);
 
   // --------------------------------------
   //Signal and bkg yields
   RooRealVar sigYield("sigYield", "fitted yield for Signal",0);
   RooRealVar bkgYield("bkgYield", "fitted yield for Background",0);

   if (isMC){
      sigYield.setVal(1000);
      sigYield.setRange(0., 100000000);
      bkgYield.setVal(50 );
      bkgYield.setRange(0,1000000);
   }

   else{
      sigYield.setVal(90000);
      sigYield.setRange(0,1000000);
      bkgYield.setVal(10000);
      bkgYield.setRange(0,1000000);
   }

   // now make the combined models
   std::cout << "make full model" << std::endl;
   RooAddPdf massModel("massModel", "J/psi + bkg mass model", {sigModel, bkgModel}, {sigYield, bkgYield});
 
   std::cout << "import model" << std::endl;
   ws.import(massModel);
}


void AddModelY(RooWorkspace &ws, bool isMC,  Double_t lowRange, Double_t highRange)
{
   RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange, "GeV");
   RooRealVar forest_standard_Y_mva("forest_standard_Y_mva", "forest_standard_Y_mva", 0., 1., "");

   // mass model for multiple resonances. Gauss + dCB. In this case employed for Y
   RooRealVar mu1("mu1", "Y Mass",9.44796, lowRange, highRange);
   RooRealVar sigma1("sigma1", "Width of Gaussian", 0.2, 0.01, 0.2, "GeV");
   RooGaussian Gaussian1("Gaussian1", "Gaussian1", Mm_mass, mu1, sigma1);
   RooRealVar sigmaL("sigmaL", "Width of left CB", 0.1, 0.01, 0.1, "GeV");
   RooRealVar sigmaR("sigmaR", "Width of right CB", 0.1, 0.01, 0.1, "GeV");
   RooRealVar n1("n1", "n CB", 5, 1.5,10, "");
   RooRealVar alpha("alpha", "Alpha  CB", 0.376, 0.001, 3, "");
   RooCrystalBall CB1("CB1", "CB", Mm_mass, mu1, sigmaL, sigmaR, alpha,n1,alpha,n1);
   RooRealVar GaussFraction("GaussFraction", "Fraction of Gaussian", 0.459, 0, 1, "");
   RooAddPdf sig1("sig1", "Y mass model", RooArgList(Gaussian1, CB1), GaussFraction);

   RooRealVar mu2("mu2", "Y Mass", 10.01066, lowRange, highRange);
   RooRealVar sigma2("sigma2", "Width of Gaussian", 0.1, 0.01, 0.2, "GeV");
   RooGaussian Gaussian2("Gaussian2", "Gaussian2", Mm_mass, mu2, sigma2);
   RooRealVar n2("n2", "nL CB", 2, 1.5,10, "");
   RooCrystalBall CB2("CB2", "CB", Mm_mass, mu2, sigmaL, sigmaR, alpha,n2,alpha,n2);
   RooAddPdf sig2("sig2", "Y mass model", RooArgList(Gaussian2, CB2), GaussFraction);

   RooRealVar mu3("mu3", "Y Mass", 10.339, lowRange, highRange);
   RooRealVar sigma3("sigma3", "Width of Gaussian", 0.0888, 0.01, 0.2, "GeV");
   RooGaussian Gaussian3("Gaussian3", "Gaussian3", Mm_mass, mu3, sigma3);
   RooRealVar n3("n3", "nL CB", 10, 2,10, "");
   RooCrystalBall CB3("CB3", "CB", Mm_mass, mu3, sigmaL, sigmaR, alpha,n3,alpha,n3);
   RooAddPdf sig3("sig3", "Y mass model", RooArgList(Gaussian3, CB3), GaussFraction);

   RooRealVar fracY2("fracY2","fracY2",0.25, 0, 0.5, "" );
   RooRealVar fracY3("fracY3","fracY3",0.25, 0, 0.5, "" );
   RooAddPdf sigModel("sigModel", "Y mass model", RooArgList(sig2, sig3, sig1), RooArgList(fracY2,fracY3), false);
   
   // make bkg model
   std::cout << "make bkg model" << std::endl;
   RooRealVar bkgDecayConst("bkgDecayConst", "Decay const for bkg mass spectrum", -0.353, -10, 10, "1/GeV");
   RooExponential bkgModel("bkgModel", "bkg Mass Model", Mm_mass, bkgDecayConst);
 
   // --------------------------------------
   //Signal and bkg yields
   RooRealVar sigYield("sigYield", "fitted yield for Signal",10);
   RooRealVar bkgYield("bkgYield", "fitted yield for Background",10);

   if (isMC){
      sigYield.setVal(1000);
      sigYield.setRange(0., 100000000);
      bkgYield.setVal(50 );
      bkgYield.setRange(0,1000000);
   }

   else{
      sigYield.setVal(400000);
      sigYield.setRange(0,1000000);
      bkgYield.setVal(500000);
      bkgYield.setRange(0,1000000);
   }

   // now make the combined models
   std::cout << "make full model" << std::endl;
   RooAddPdf massModel("massModel", "J/psi + bkg mass model", {sigModel, bkgModel}, {sigYield, bkgYield});
 
   std::cout << "import model" << std::endl;
   ws.import(massModel);
}


void AddModelYMC(RooWorkspace &ws, bool isMC,  Double_t lowRange, Double_t highRange)
{
   RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange, "GeV");
   RooRealVar forest_standard_Y_mva("forest_standard_Y_mva", "forest_standard_Y_mva", 0., 1., "");

   // mass model for single resonance. Gauss + dCB. In this case employed for Jpsi
   RooRealVar mu("mu", "J/Psi Mass", 9, lowRange, highRange);
   RooRealVar sigma("sigma", "Width of Gaussian", 0.1, 0.001, 10, "GeV");
   RooGaussian Gaussian("Gaussian", "Gaussian", Mm_mass, mu, sigma);

   RooRealVar sigmaL("sigmaL", "Width of left CB", 0.01956, 0.001, 10, "GeV");
   RooRealVar sigmaR("sigmaR", "Width of right CB", 0.01957, 0.001, 10, "GeV");
   RooRealVar nL("nL", "nL CB", 10, 0.1,15, "");
   RooRealVar alphaL("alphaR", "Alpha right CB", 3, 0.001, 5, "");
   RooRealVar nR("nR", "nR CB", 10, 0.1,15, "");
   RooRealVar alphaR("alphaL", "Alpha left CB", 3, 0.001, 5, "");
   RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigmaL, sigmaR, alphaL,nL,alphaR,nR);

   RooRealVar GaussFraction("GaussFraction", "Fraction of Gaussian", 0.5, 0, 1, "");

   // Final model is sigModel
   RooAddPdf sigModel("sigModel", "J/psi mass model", RooArgList(Gaussian, CB), GaussFraction);
   // --------------------------------------
   // make bkg model
 
   std::cout << "make bkg model" << std::endl;
   // mass model for QCD.
   // the exponential is of the form exp(c*x).  If we want
   // the mass to decay an e-fold every R GeV, we use
   // c = -1/R.
   // We can leave this parameter free during the fit.
   RooRealVar bkgDecayConst("bkgDecayConst", "Decay const for bkg mass spectrum", -0.353, -100, 100, "1/GeV");
   RooExponential bkgModel("bkgModel", "bkg Mass Model", Mm_mass, bkgDecayConst);
 
   // --------------------------------------
   //Signal and bkg yields
   RooRealVar sigYield("sigYield", "fitted yield for Signal",0);
   RooRealVar bkgYield("bkgYield", "fitted yield for Background",0);

   if (isMC){
      sigYield.setVal(1000);
      sigYield.setRange(0., 100000000);
      bkgYield.setVal(50 );
      bkgYield.setRange(0,1000000);
   }

   else{
      sigYield.setVal(40000000);
      sigYield.setRange(0,100000000);
      bkgYield.setVal(5000000);
      bkgYield.setRange(0,10000000);
   }

   // now make the combined models
   std::cout << "make full model" << std::endl;
   RooAddPdf massModel("massModel", "Y + bkg mass model", {sigModel, bkgModel}, {sigYield, bkgYield});
 
   std::cout << "import model" << std::endl;
   ws.import(massModel);
}


void AddData(RooWorkspace &ws, const char* inputfilename, Double_t lowRange, Double_t highRange, int nEvents)
{
   // make the toy data
   std::cout << "make data set and import to workspace" << std::endl;

   TFile *inputfile = TFile::Open(inputfilename);
   TTree *tree = inputfile->Get<TTree>("tree");
    
   RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange); 
   RooRealVar forest_standard_Y_mva("forest_standard_Y_mva", "forest_standard_Y_mva", 0, 1);
   // RooRealVar vtx_BDT_Y_forest("vtx_BDT_Y_forest", "vtx_BDT_Y_forest", 0, 1);

   RooRealVar Muon_softMva1("Muon_softMva1", "Muon_softMva1", -1, 1);
   RooRealVar Muon_softMva2("Muon_softMva2", "Muon_softMva2", -1, 1);
   RooRealVar Mm_kin_lxy("Mm_kin_lxy", "Mm_kin_lxy", 0, 100);
   RooRealVar Mm_kin_eta("Mm_kin_eta", "Mm_kin_eta", -10, 10);

   //training vars
   RooRealVar Mm_kin_l3d("Mm_kin_l3d", "Mm_kin_l3d", -1000000, 1000000);
   RooRealVar Mm_kin_sl3d("Mm_kin_sl3d", "Mm_kin_sl3d", -1000000, 1000000);
   RooRealVar Mm_kin_vtx_chi2dof("Mm_kin_vtx_chi2dof", "Mm_kin_vtx_chi2dof", -1000000, 1000000);
   RooRealVar Mm_kin_vtx_prob("Mm_kin_vtx_prob", "Mm_kin_vtx_prob", -1000000, 1000000);
   RooRealVar Mm_kin_alpha("Mm_kin_alpha", "Mm_kin_alpha", -1000000, 1000000);
   RooRealVar Mm_kin_alphaBS("Mm_kin_alphaBS", "Mm_kin_alphaBS", -1000000, 1000000);
   RooRealVar Mm_closetrk("Mm_closetrk", "Mm_closetrk", -1000000, 1000000);
   RooRealVar Mm_closetrks1("Mm_closetrks1", "Mm_closetrks1", -1000000, 1000000);
   RooRealVar Mm_closetrks2("Mm_closetrks2", "Mm_closetrks2", -1000000, 1000000);
   RooRealVar Mm_kin_pvip("Mm_kin_pvip", "Mm_kin_pvip", -1000000, 1000000);
   RooRealVar Mm_kin_spvip("Mm_kin_spvip", "Mm_kin_spvip", -1000000, 1000000);
   RooRealVar Mm_kin_pvlip("Mm_kin_pvlip", "Mm_kin_pvlip", -1000000, 1000000);
   RooRealVar Mm_kin_slxy("Mm_kin_slxy", "Mm_kin_slxy", -1000000, 1000000);
   RooRealVar Mm_iso("Mm_iso", "Mm_iso", -1000000, 1000000);
   RooRealVar Mm_otherVtxMaxProb("Mm_otherVtxMaxProb", "Mm_otherVtxMaxProb", -1000000, 1000000);
   RooRealVar Mm_otherVtxMaxProb1("Mm_otherVtxMaxProb1", "Mm_otherVtxMaxProb1", -1000000, 1000000);
   RooRealVar Mm_otherVtxMaxProb2("Mm_otherVtxMaxProb2", "Mm_otherVtxMaxProb2", -1000000, 1000000);

   RooDataSet data_full("data_full", "data_full", RooArgSet(Mm_mass, forest_standard_Y_mva,Muon_softMva1,Muon_softMva2,Mm_kin_lxy,Mm_kin_eta,Mm_kin_l3d,Mm_kin_sl3d,Mm_kin_vtx_chi2dof,
      Mm_kin_vtx_prob,Mm_kin_alpha,Mm_kin_alphaBS,Mm_closetrk,Mm_closetrks1,Mm_closetrks2,Mm_kin_pvip,Mm_kin_spvip,Mm_kin_pvlip,Mm_kin_slxy,Mm_iso,Mm_otherVtxMaxProb,
      Mm_otherVtxMaxProb1,Mm_otherVtxMaxProb2), Import(*tree));

   Int_t numEntries = data_full.numEntries();
   TBits outputBits = TBits(numEntries);
   for(int i=0; i<nEvents; i++){
      Int_t rnd = gRandom->Integer(numEntries);
      outputBits.SetBitNumber(rnd);
   }
   RooAbsData* data = data_full.emptyClone("data");
   Int_t startBit = outputBits.FirstSetBit(0);
   while (startBit < numEntries){
      data->add(*data_full.get(startBit))   ;                                                              
      startBit = outputBits.FirstSetBit(startBit + 1);
   }

   // RooAbsData* data = data_full.reduce( EventRange(0,100000)); //Cut("Mm_mass<9.8||Mm_mass>10.7"),
 
   // import data into workspace
   ws.import(*data);

}

//____________________________________
void DoSPlot(RooWorkspace &ws, const char* modelname)
{
   std::cout << "Calculate sWeights" << std::endl;
 
   // get what we need out of the workspace to do the fit
   RooAbsPdf *massModel = ws.pdf("massModel");
   RooRealVar *sigYield = ws.var("sigYield");
   RooRealVar *bkgYield = ws.var("bkgYield");
   RooRealVar *Mm_mass = ws.var("Mm_mass");
   RooDataSet& data = static_cast<RooDataSet&>(*ws.data("data"));

   RooDataSet data_mass{data.GetName(), data.GetTitle(), &data, RooArgSet(*Mm_mass), nullptr};

   ws.Print();
 
   std::cout << "\n\n------------------------------------------\nThe dataset before creating sWeights:\n";
   data.Print();


   sigYield->setConstant(false);
   bkgYield->setConstant(false);
   massModel->fitTo(data_mass);
   massModel->graphVizTree(modelname);

   // std::cout<<massModel->getParameters();
   // massModel->printMultiline(std::cout,10);


   std::cout<<sigYield->getVal()<<"  ;   "<<bkgYield->getVal() << "\n \n \n";


   RooStats::SPlot sData{"sData", "An SPlot", data, massModel, RooArgList(*sigYield, *bkgYield)};


   // interesting for debugging and visualizing the model
 
   std::cout << "\n\nThe dataset after creating sWeights:\n";
   data.Print();
 
   // Check that our weights have the desired properties
 
   std::cout << "\n\n------------------------------------------\n\nCheck SWeights:" << std::endl;
 
   std::cout << std::endl
             << "Yield of signal is\t" << sigYield->getVal() << ".  From sWeights it is "
             << sData.GetYieldFromSWeight("sigYield") << std::endl;
 
   std::cout << "Yield of bkg is\t" << bkgYield->getVal() << ".  From sWeights it is "
             << sData.GetYieldFromSWeight("bkgYield") << std::endl
             << std::endl;
 
   for (Int_t i = 0; i < 10; i++) {
      std::cout << "signal Weight for event " << i << std::right << std::setw(12) << sData.GetSWeight(i, "sigYield") << "  bkg Weight"
                << std::setw(12) << sData.GetSWeight(i, "bkgYield") << "  Total Weight" << std::setw(12) << sData.GetSumOfEventSWeight(i)
                << std::endl;
   }
 
   std::cout << std::endl;

 
   // import this new dataset with sWeights
   std::cout << "import new dataset with sWeights" << std::endl;
   ws.import(data, Rename("dataWithSWeights"));

}

void SaveData(RooWorkspace &ws, const char* file_name)
{  
   RooDataSet& data = static_cast<RooDataSet&>(*ws.data("dataWithSWeights"));
   TFile* out = new TFile(file_name, "RECREATE");
   out->cd();
   RooDataSet::setDefaultStorageType(RooAbsData::Tree);

   RooDataSet mydataw (data.GetName(),data.GetTitle(), &data,*data.get());
   const TTree* tree8 = mydataw.GetClonedTree();
   cout << "tree entries: " << tree8->GetEntries() << endl;
   tree8->Write();
   out->Close();

   cout<<"\n\n saving file as "<<file_name<<"\n\n";

}
 
void MakePlots(RooWorkspace &ws, const char* fig_name)
{
 
   // Here we make plots of the discriminating variable (invMass) after the fit
   // and of the control variable (isolation) after unfolding with sPlot.
   std::cout << "make plots" << std::endl;

 
   // make our canvas
   TCanvas *cdata = new TCanvas("sPlot", "sPlot demo", 2000, 3000);
   cdata->Divide(1, 3);
 
   // get what we need out of the workspace
   RooAbsPdf *sigModel = ws.pdf("sigModel");
   RooAbsPdf *bkgModel = ws.pdf("bkgModel");
   RooAbsPdf *massModel = ws.pdf("massModel");

 
   RooRealVar *forest_standard_Y_mva = ws.var("forest_standard_Y_mva");
   RooRealVar *Mm_mass = ws.var("Mm_mass");
 
   // note, we get the dataset with sWeights
   auto& data = static_cast<RooDataSet&>(*ws.data("dataWithSWeights"));
 
   // create weighted data sets
   
   RooDataSet data_mass{data.GetName(), data.GetTitle(), &data, RooArgSet(*Mm_mass), nullptr};
   RooDataSet dataw_sig{data.GetName(), data.GetTitle(), &data, *data.get(), nullptr, "sigYield_sw"};
   RooDataSet dataw_bkg{data.GetName(), data.GetTitle(), &data, *data.get(), nullptr, "bkgYield_sw"};

   std::cout << "Print data";
   data.Print();

   cdata->cd(1);
   RooPlot *frame = Mm_mass->frame(Title("Fit of model to discriminating variable"));
   data_mass.plotOn(frame);
   massModel->plotOn(frame, Name("FullModel"));
   massModel->plotOn(frame, Components(*sigModel), LineStyle(kDashed), LineColor(kRed), Name("SigModel"));
   massModel->plotOn(frame, Components(*bkgModel), LineStyle(kDashed), LineColor(kGreen), Name("BkgModel"));
   massModel->paramOn(frame);
 
   TLegend leg(0.11, 0.5, 0.5, 0.8);
   leg.AddEntry(frame->findObject("FullModel"), "Full model", "L");
   leg.AddEntry(frame->findObject("SigModel"), "Signal model", "L");
   leg.AddEntry(frame->findObject("BkgModel"), "Bkg model", "L");
   leg.SetBorderSize(0);
   leg.SetFillStyle(0);

 
   frame->Draw();
   leg.DrawClone();
 
   // Plot isolation for Z component.
   // Do this by plotting all events weighted by the sWeight for the Z component.
   // The SPlot class adds a new variable that has the name of the corresponding
   // yield + "_sw".
   cdata->cd(2);
 
   RooPlot *frame2 = forest_standard_Y_mva->frame(Title("BDT distribution with s weights to project out signal"));
   // Since the data are weighted, we use SumW2 to compute the errors.
   dataw_sig.plotOn(frame2, DataError(RooAbsData::SumW2));
   // sigModel->plotOn(frame2, LineStyle(kDashed), LineColor(kRed));
 
   frame2->Draw();
 
   // Plot isolation for QCD component.
   // Eg. plot all events weighted by the sWeight for the QCD component.
   // The SPlot class adds a new variable that has the name of the corresponding
   // yield + "_sw".
   cdata->cd(3);
   RooPlot *frame3 = forest_standard_Y_mva->frame(Title("BDT distribution with s weights to project out bkg"));
   dataw_bkg.plotOn(frame3, DataError(RooAbsData::SumW2));
   // bkgModel->plotOn(frame3, LineStyle(kDashed), LineColor(kGreen));
 
   frame3->Draw();
 
   cdata->SaveAs(fig_name);
   cout<<"\n\n saving file as "<<fig_name<<"\n\n";
}