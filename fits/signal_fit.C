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

// use this order for safety on library loading
using namespace RooFit;
using namespace RooStats;
using namespace std;
 
// see below for implementationbelow for implementatio
void load_config(const char* ,bool, const char*&, const char*&, const char*&, const char*&, Double_t&, Double_t&, const char*);
void AddModelJ_dCB_G(RooWorkspace &, bool,  Double_t, Double_t);
void AddModelJ_dG(RooWorkspace &, bool,  Double_t, Double_t);
void AddModelY(RooWorkspace &, bool,  Double_t, Double_t);
void AddModelYMC(RooWorkspace &, bool,  Double_t, Double_t);
void AddData(RooWorkspace &, const char*, Double_t, Double_t, int);
void DoFit(RooWorkspace &, const char*, const char*);
void SaveData(RooWorkspace &, const char*);
void MakePlots(RooWorkspace &, const char*);



void signal_fit(const char* meson, bool isMC, int nEntries = 1000000)  // "Jpsi" or "Y"
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

   AddModelJ_dCB_G(wspace,isMC, lowRange, highRange);
   // AddModelJ_dG(wspace,isMC, lowRange, highRange);

   AddData(wspace, inputfilename, lowRange, highRange, nEntries);
   DoFit(wspace,model_name, file_name);
   // SaveData(wspace, file_name);
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

   string s = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/") + string(meson) + string(*data_or_MC) + string(extra) + string(".root");
   string* file_name_str = new string(s);
   file_name = file_name_str->c_str();

   s = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/") + string(meson) + string(*data_or_MC) + string(extra) + string(".gif");
   string* fig_name_str = new string(s);
   fig_name = fig_name_str->c_str();
   
   s = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/models/") + string(meson) + string("_fit") + string(*data_or_MC) + string(extra) + string(".dot");
   string* model_name_str = new string(s); 
   model_name = model_name_str->c_str();

   if (string(meson) == string("Jpsi")) {
      lowRange = 2.6;
      highRange = 3.56;
      if (isMC) inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/Jpsi/merged_A.root"; //JPsi MC
      else inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Jpsi/merged_A_bkp.root"; //Jpsi data
   }
   else if(string(meson) == string("Y")){
      lowRange = 8.5;
      highRange = 11.2;
      if (isMC) inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/Y/merged_A.root"; //Upsilon MC
      else inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Y/merged_A.root"; //Upsilon data
   }
   else {cout<<"type of meson not recognized";}
}
 
void AddModelJ_dCB_G(RooWorkspace &ws, bool isMC,  Double_t lowRange, Double_t highRange) //double CB + Gaussian
{
   RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange, "GeV");

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

   // RooRealVar c0("c0","c0",1,-100,100,"");
   // RooRealVar c1("c1","c1",1,-100,100,"");
   // RooRealVar c2("c2","c2",1,-100,100,"");
   // RooRealVar c3("c3","c3",1,-100,100,"");
   // RooBernstein bkgModel("bkgModel", "bkg Mass Model", Mm_mass,RooArgList(c0,c1,c2,c3));
 
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
void AddModelJ_dG(RooWorkspace &ws, bool isMC,  Double_t lowRange, Double_t highRange) //Double Gaussian
{
   RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange, "GeV");

   // mass model for single resonance. Gauss + Gauss. In this case employed for Jpsi
   RooRealVar mu1("mu1", "J/Psi Mass", 3.09809, lowRange, highRange);
   RooRealVar sigma1("sigma1", "Width of Gaussian", 0.0378, 0.001, 10, "GeV");
   RooGaussian Gaussian1("Gaussian1", "Gaussian1", Mm_mass, mu1, sigma1);

   RooRealVar mu2("mu2", "J/Psi Mass", 3.09809, lowRange, highRange);
   RooRealVar sigma2("sigma2", "Width of Gaussian", 1, 0.001, 10, "GeV");
   RooGaussian Gaussian2("Gaussian2", "Gaussian2", Mm_mass, mu2, sigma2);

   RooRealVar GaussFraction("GaussFraction", "Fraction of first Gaussian", 0.5, 0, 1, "");

   // Final model is sigModel
   RooAddPdf sigModel("sigModel", "J/psi mass model", RooArgList(Gaussian1, Gaussian2), GaussFraction);
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
   
   // make bkg model
   // RooRealVar bkgDecayConst("bkgDecayConst", "Decay const for bkg mass spectrum", -0.353, -100, 100, "1/GeV");
   // RooExponential bkgModel("bkgModel", "bkg Mass Model", Mm_mass, bkgDecayConst);
   
   RooRealVar c0("c0","c0",-10,10,1,"");
   RooRealVar c1("c1","c1",-10,10,1,"");
   RooRealVar c2("c2","c2",-10,10,1,"");
   RooRealVar c3("c3","c3",-10,10,1,"");
   RooBernstein bkgModel("bkgModel", "bkg Mass Model", Mm_mass,RooArgList(c0,c1,c2,c3));

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

   RooDataSet data_full("data_full", "data_full", RooArgSet(Mm_mass, Muon_softMva1,Muon_softMva2,Mm_kin_lxy,Mm_kin_eta,Mm_kin_l3d,Mm_kin_sl3d,Mm_kin_vtx_chi2dof,
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
void DoFit(RooWorkspace &ws, const char* modelname, const char* file_name)
{
   RooAbsPdf *massModel = ws.pdf("massModel");
   RooRealVar *sigYield = ws.var("sigYield");
   RooRealVar *bkgYield = ws.var("bkgYield");
   RooRealVar *Mm_mass = ws.var("Mm_mass");
   RooDataSet& data = static_cast<RooDataSet&>(*ws.data("data"));

   RooDataSet data_mass{data.GetName(), data.GetTitle(), &data, RooArgSet(*Mm_mass), nullptr};

   RooMsgService::instance().setSilentMode(true);
   // RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
   data.Print();
 
   sigYield->setConstant(false);
   bkgYield->setConstant(false);
   massModel->fitTo(data_mass);
   massModel->graphVizTree(modelname);

   // std::cout<<massModel->getParameters();
   // massModel->printMultiline(std::cout,10);

   std::cout<<sigYield->getVal()<<"  ;   "<<bkgYield->getVal() << "\n \n \n";
   ws.writeToFile(file_name);
}

void SaveData(RooWorkspace &ws, const char* file_name)
{  
   RooDataSet& data = static_cast<RooDataSet&>(*ws.data("data"));
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
   std::cout << "make plots" << std::endl;
   // setTDRStyle();

   TCanvas *cdata = new TCanvas("fit", "fit demo", 2000, 1500);
   RooAbsPdf *sigModel = ws.pdf("sigModel");
   RooAbsPdf *bkgModel = ws.pdf("bkgModel");
   RooAbsPdf *massModel = ws.pdf("massModel");

   RooRealVar *Mm_mass = ws.var("Mm_mass");
 
   RooRealVar *sigYield = ws.var("sigYield");
   RooRealVar *bkgYield = ws.var("bkgYield");
   auto& data = static_cast<RooDataSet&>(*ws.data("data"));
   RooDataSet data_mass{data.GetName(), data.GetTitle(), &data, RooArgSet(*Mm_mass), nullptr};

   cdata->Divide(1, 2);

   cdata->cd(1);
   RooPlot *frame = Mm_mass->frame(Title("Fit of model to discriminating variable"));
   data_mass.plotOn(frame,XErrorSize(1), Name("Data"), MarkerSize(0.6), DrawOption("PZ"));
   massModel->plotOn(frame, Name("FullModel"));
   massModel->plotOn(frame, Components(*sigModel), Name("SigModel"),DrawOption("F"),FillColor(3), FillStyle(3001), LineColor(0));
   massModel->plotOn(frame, Components(*bkgModel), LineStyle(7), LineColor(7), Name("BkgModel"));
   
   //compute chisquare
   RooAbsCollection *flparams = massModel->getParameters(data_mass)->selectByAttrib("Constant", kFALSE);
   Int_t nflparams = flparams->getSize();
   cout << nflparams << "\n";
   Double_t chisquare = -1;
   chisquare = 1; //frame->chiSquare("massModel", "data", nflparams);
 
   TLegend leg(0.8, 0.6, 0.9,0.85);
   leg.AddEntry(frame->findObject("FullModel"), "Full model", "l");
   leg.AddEntry(frame->findObject("SigModel"), "Signal model", "l");
   leg.AddEntry(frame->findObject("BkgModel"), "Bkg model", "l");
   leg.SetBorderSize(0);
   leg.SetFillStyle(0);
   leg.SetTextSize(0.04);
   leg.SetTextFont(42);
   leg.Draw();

   TPaveText *label_2 = new TPaveText(0.55, 0.23, 0.8, 0.50, "NDC");
   label_2->SetBorderSize(0);
   label_2->SetFillColor(0);
   label_2->SetTextSize(0.041);
   label_2->SetTextFont(42);
   gStyle->SetStripDecimals(kTRUE);
   label_2->SetTextAlign(11);
   TString sYield = to_string(int(round(sigYield->getValV())));
   TString bYield = to_string(int(round(bkgYield->getValV())));
   label_2->AddText("N_{sig} = " + sYield);
   label_2->AddText("N_{bkg} = " + bYield);
   label_2->Draw();

   TPaveText *cms = new TPaveText(0.14, 0.922, 0.3, 0.93, "NDC");
   cms->AddText("CMS preliminary");
   cms->SetBorderSize(0);
   cms->SetFillColor(0);
   cms->SetTextSize(0.04);
   cms->SetTextFont(42);
   cms->Draw();
   
   gPad->SetLeftMargin(0.15) ;
   gPad->SetBottomMargin(0.03);
   gPad->SetPad(0.01,0.2,0.99,0.99);
   frame->GetYaxis()->SetTitleSize(0.04);
   frame->GetYaxis()->SetTitleOffset(1.2);
   frame->GetXaxis()->SetLabelSize(0.);
   frame->GetXaxis()->SetTitleSize(0.);
   frame->Draw();

   //plot pulls
   cdata->cd(2);
   // RooHist *hpull = frame->pullHist("data", "massModel");
   RooPlot *frame2 = Mm_mass->frame(Title("Pull"));
   // frame2->addPlotable(hpull, "P"); //,"E3");
   frame2->SetMarkerStyle(2);
   frame2->SetMarkerSize(0.01);

   gPad->SetLeftMargin(0.15);
   gPad->SetPad(0.01, 0.01, 0.99, 0.2);
   gPad->SetTopMargin(0.1);
   gPad->SetBottomMargin(0.5);
   frame2->GetYaxis()->SetNdivisions(202);
   frame2->GetYaxis()->SetRangeUser(-4, 4);
   frame2->GetXaxis()->SetTitle("m_{#mu#mu} [GeV]");
   frame2->GetYaxis()->SetTitle("Pulls");
   frame2->GetXaxis()->SetTitleSize(0.2);
   frame2->GetYaxis()->SetTitleSize(0.2);
   frame2->GetXaxis()->SetLabelSize(0.15);
   frame2->GetYaxis()->SetLabelSize(0.15);
   frame2->GetXaxis()->SetLabelOffset(0.01);
   frame2->GetYaxis()->SetLabelOffset(0.01);
   frame2->GetYaxis()->SetTitleOffset(0.2);
   frame2->GetXaxis()->SetTickLength(0.1);
   gPad->SetFrameFillColor(0);
   gPad->SetFrameBorderMode(0);
   gPad->SetFrameFillColor(0);
   gPad->SetFrameBorderMode(0);
   frame2->Draw();

   cdata->SaveAs(fig_name);
   cout << "\n\n saving file as " << fig_name << "\n\n";
}