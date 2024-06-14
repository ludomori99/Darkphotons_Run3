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

// use this order for safety on library loading
using namespace RooFit;
using namespace RooStats;
using namespace std;
 
// see below for implementationbelow for implementatio
void load_config(const char* ,bool, const char*&, const char*&, const char*&, Double_t&, Double_t&);
void AddModelJ_dCB_G(RooWorkspace &, bool, const  char*,  Double_t, Double_t);
void AddModelJ_dG(RooWorkspace &, bool, const  char*,  Double_t, Double_t);
void AddModelJ_VG(RooWorkspace &, bool, const  char*,  Double_t, Double_t);
void AddModelJ_dCB_V(RooWorkspace &, bool, const char*, Double_t , Double_t);
void AddModelY(RooWorkspace &, bool,  Double_t, Double_t);
void AddModelYMC(RooWorkspace &, bool,  Double_t, Double_t);
void AddData(RooWorkspace &, const char*, Double_t, Double_t, int);
void DoFit(RooWorkspace &, const char*, const char*);


void signal_fit(const char* meson, bool isMC, int nEntries = 0)  // "Jpsi" or "Y"; nEntries==0 means take all entries in dataset
{
   // Create a workspace to manage the project.
   const char* file_name;
   const char* model_name; 
   const char* inputfilename;
   Double_t lowRange;
   Double_t highRange;

   RooWorkspace ws_dCB{"ws_dCB_fixS"};
   model_name = "dCB_fixS";
   load_config(meson, isMC, file_name, model_name, inputfilename, lowRange, highRange);
   AddData(ws_dCB, inputfilename, lowRange, highRange, nEntries);
   AddModelJ_dCB_G(ws_dCB,isMC, model_name, lowRange, highRange);
   DoFit(ws_dCB,model_name, file_name);

   // RooWorkspace ws_dG{"ws_dG"};
   // model_name = "dG";
   // load_config(meson, isMC, file_name, model_name, inputfilename, lowRange, highRange);
   // AddData(ws_dG, inputfilename, lowRange, highRange, nEntries);
   // AddModelJ_dG(ws_dG,isMC, model_name, lowRange, highRange);
   // DoFit(ws_dG,model_name, file_name);

   // RooWorkspace ws_VG{"ws_VG"};
   // model_name = "VG";
   // load_config(meson, isMC, file_name, model_name, inputfilename, lowRange, highRange);
   // AddData(ws_VG, inputfilename, lowRange, highRange, nEntries);
   // AddModelJ_VG(ws_VG,isMC, model_name, lowRange, highRange);
   // DoFit(ws_VG,model_name, file_name);

   RooWorkspace ws_dCB_V{"ws_dCB_V_fixS"};
   model_name = "dCB_V_fixS";
   load_config(meson, isMC, file_name, model_name, inputfilename, lowRange, highRange);
   AddData(ws_dCB_V, inputfilename, lowRange, highRange, nEntries);
   AddModelJ_dCB_V(ws_dCB_V,isMC, model_name, lowRange, highRange);
   DoFit(ws_dCB_V,model_name, file_name);


   // RooWorkspace ws_dCB_V_var{"ws_dCB_V_var"};
   // model_name = "dCB_V_var";
   // load_config(meson, isMC, file_name, model_name, inputfilename, lowRange, highRange);
   // AddData(ws_dCB_V_var, inputfilename, lowRange, highRange, nEntries);
   // AddModelJ_dCB_V(ws_dCB_V_var,isMC, model_name, lowRange, highRange);
   // DoFit(ws_dCB_V_var,model_name, file_name);
}


void load_config(const char* meson, bool isMC,
               const char*& file_name,
               const char*& model_name, 
               const char*& inputfilename, 
               Double_t& lowRange,
               Double_t& highRange)
{
   const char** data_or_MC = new const char*;
   if (isMC) *data_or_MC = "_MC_";
   else *data_or_MC = "_data_";

   string s = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/files/") + string(meson) + string(*data_or_MC) + string(model_name) + string(".root");
   string* file_name_str = new string(s);
   file_name = file_name_str->c_str();
   
   // s = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/models/") + string(meson) + string("_fit") + string(*data_or_MC) + string(extra) + string(".dot");
   // string* model_name_str = new string(s); 
   // model_name = model_name_str->c_str();

   if (string(meson) == string("Jpsi")) {
      lowRange = 2.6;
      highRange = 3.56;
      if (isMC) inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Jpsi/merged_A.root"; //JPsi MC
      else inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Jpsi/merged_A.root"; //Jpsi data
   }
   else if(string(meson) == string("Y")){
      lowRange = 8.5;
      highRange = 11.2;
      if (isMC) inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY123_A.root"; //Upsilon MC
      else inputfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Y/merged_A.root"; //Upsilon data
   }
   else {cout<<"type of meson not recognized";}
}
 

void AddModelJ_dCB_G(RooWorkspace &ws, bool isMC, const char* modelName, Double_t lowRange, Double_t highRange)
{
   RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange, "GeV");

   // mass model for single resonance. Gauss + dCB. In this case employed for Jpsi
   RooRealVar mu("mu", "J/Psi Mass", 3.09809, lowRange, highRange);
   RooRealVar sigma("sigma", "Width of Gaussian", 0.0378, 0.001, 10, "GeV");
   RooGaussian Gaussian("Gaussian", "Gaussian", Mm_mass, mu, sigma);

   RooRealVar sigmaL("sigmaL", "Width of left CB", 0.01956, 0.001, 10, "GeV");
   RooRealVar sigmaR("sigmaR", "Width of right CB", 0.01957, 0.001, 11, "GeV");
   RooRealVar nL("nL", "nL CB", 3, 1,15, "");
   RooRealVar alphaL("alphaL", "Alpha left CB", 2.5, 1, 5, "");
   RooRealVar nR("nR", "nR CB", 3, 1,15, "");
   RooRealVar alphaR("alphaR", "Alpha right CB", 1.7, 1, 5, "");


   //choose whether to fix or not the sigmas 
   // RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigmaL, sigmaR, alphaL,nL,alphaR,nR);


   RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigma, sigma, alphaL,nL,alphaR,nR);

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
      sigYield.setVal(400000);
      sigYield.setRange(0,100000000);
      bkgYield.setVal(100000);
      bkgYield.setRange(0,10000000);
   }

   // now make the combined models
   std::cout << "make full model" << std::endl;
   RooAddPdf massModel(modelName, "J/psi + bkg mass model", {sigModel, bkgModel}, {sigYield, bkgYield});
 
   std::cout << "import model" << std::endl;
   ws.import(massModel);
}
void AddModelJ_dG(RooWorkspace &ws, bool isMC, const char* modelName, Double_t lowRange, Double_t highRange) //Double Gaussian
{
   RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange, "GeV");

   // mass model for single resonance. Gauss + Gauss. In this case employed for Jpsi
   RooRealVar mu1("mu1", "J/Psi Mass", 3.09809, lowRange, highRange);
   RooRealVar sigma1("sigma1", "Width of Gaussian", 0.0378, 0.001, 10, "GeV");
   RooGaussian Gaussian1("Gaussian1", "Gaussian1", Mm_mass, mu1, sigma1);

   // RooRealVar mu2("mu2", "J/Psi Mass", 3.09809, lowRange, highRange);
   RooRealVar sigma2("sigma2", "Width of Gaussian", 1, 0.001, 10, "GeV");
   RooGaussian Gaussian2("Gaussian2", "Gaussian2", Mm_mass, mu1, sigma2);

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
   RooAddPdf massModel(modelName, "J/psi + bkg mass model", {sigModel, bkgModel}, {sigYield, bkgYield});
 
   std::cout << "import model" << std::endl;
   ws.import(massModel);
}
void AddModelJ_VG(RooWorkspace &ws, bool isMC, const char* modelName, Double_t lowRange, Double_t highRange) //Gaussian + Voigtian
{
   RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange, "GeV");
   RooRealVar mu1("mu1", "J/Psi Mass", 3.09809, lowRange, highRange);
   RooRealVar sigma1("sigma1", "Width of Gaussian", 0.0378, 0.001, 10, "GeV");
   RooGaussian Gaussian("Gaussian1", "Gaussian1", Mm_mass, mu1, sigma1);

   RooRealVar sigma2("sigma2", "Width of Gaussian", 1, 0.001, 10, "GeV");
   RooRealVar l("l", "Width of BW", 1, 0.001, 10, "GeV");
   RooVoigtian Voigtian("Gaussian2", "Gaussian2", Mm_mass, mu1, l,sigma2);

   RooRealVar GaussFraction("GaussFraction", "Fraction of first Gaussian", 0.5, 0, 1, "");

   // Final model is sigModel
   RooAddPdf sigModel("sigModel", "J/psi mass model", RooArgList(Gaussian, Voigtian), GaussFraction);
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
   RooAddPdf massModel(modelName, "J/psi + bkg mass model", {sigModel, bkgModel}, {sigYield, bkgYield});
 
   std::cout << "import model" << std::endl;
   ws.import(massModel);
}
void AddModelJ_dCB_V(RooWorkspace &ws, bool isMC, const char* modelName, Double_t lowRange, Double_t highRange)
{
   RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange, "GeV");

   // mass model for single resonance. Gauss + dCB. In this case employed for Jpsi
   RooRealVar mu("mu", "J/Psi Mass", 3.095, lowRange, highRange);
   RooRealVar sigma("sigma", "Width of Gaussian", 0.0337, 0.02, 0.1, "GeV");
   RooRealVar l("l", "Width of BW", 0.01, 0.001, 0.1, "GeV");
   RooVoigtian Voigtian("Voigtian", "Voigtian", Mm_mass, mu, l,sigma);

   RooRealVar sigmaL("sigmaL", "Width of left CB", 0.0214613, 0.01, 0.5, "GeV");
   RooRealVar sigmaR("sigmaR", "Width of right CB", 0.0198, 0.01, 0.5, "GeV");
   RooRealVar nL("nL", "nL CB", 1.7, 1,15, "");
   RooRealVar alphaL("alphaL", "Alpha left CB", 1.47, 1, 5, "");
   RooRealVar nR("nR", "nR CB", 6.79, 1,15, "");
   RooRealVar alphaR("alphaR", "Alpha right CB", 1.47, 1, 5, "");

   //choose whether to fix sigma or not 
   // RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigmaL, sigmaR, alphaL,nL,alphaR,nR);

   RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigma, sigma, alphaL,nL,alphaR,nR);

   RooRealVar GaussFraction("GaussFraction", "Fraction of Voigtian", 0.4, 0, 1, "");

   // Final model is sigModel
   RooAddPdf sigModel("sigModel", "J/psi mass model", RooArgList(Voigtian, CB), GaussFraction);
   // --------------------------------------
   // make bkg model
   std::cout << "make bkg model" << std::endl;
   RooRealVar bkgDecayConst("bkgDecayConst", "Decay const for bkg mass spectrum", -1.35, -100, 100, "1/GeV");
   RooExponential bkgModel("bkgModel", "bkg Mass Model", Mm_mass, bkgDecayConst);   

   // --------------------------------------
   //Signal and bkg yields
   RooRealVar sigYield("sigYield", "fitted yield for Signal",0);
   RooRealVar bkgYield("bkgYield", "fitted yield for Background",0);

   if (isMC){
      sigYield.setVal(1000000);
      sigYield.setRange(0., 100000000);
      bkgYield.setVal(50 );
      bkgYield.setRange(0,1000000);
   }

   else{
      sigYield.setVal(373699);
      sigYield.setRange(0,10000000);
      bkgYield.setVal(77600);
      bkgYield.setRange(0,1000000);
   }

   // now make the combined models
   std::cout << "make full model" << std::endl;
   RooAddPdf massModel(modelName, "J/psi + bkg mass model", {sigModel, bkgModel}, {sigYield, bkgYield});
 
   std::cout << "import model" << std::endl;
   ws.import(massModel);
}
void AddModelJ_dCB_V_var(RooWorkspace &ws, bool isMC, const char* modelName, Double_t lowRange, Double_t highRange)
{
   RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange, "GeV");

   // mass model for single resonance. Voigtian + dCB. In this case employed for Jpsi
   RooRealVar mu("mu", "J/Psi Mass", 3.09809, lowRange, highRange);
   RooRealVar sigma("sigma", "Width of Gaussian", 0.0378, 0.01, 0.1, "GeV");
   RooRealVar resl("resl", "resolution of BW", 1,0.2, 2, "GeV");
   RooFormulaVar l("l", "l", "@0*@1", RooArgList(sigma,resl));
   RooVoigtian Voigtian("Voigtian", "Voigtian", Mm_mass, mu,l,sigma);

   RooRealVar sigmaL_res("sigmaL_res", "reso of left CB", 1, 0.2, 2, "GeV");
   RooRealVar sigmaR_res("sigmaR_res", "reso of right CB", 1, 0.2, 2, "GeV");
   RooFormulaVar sigmaL("sigmaL", "sigmaL", "@0*@1", RooArgList(sigma,sigmaL_res));
   RooFormulaVar sigmaR("sigmaR", "sigmaR", "@0*@1", RooArgList(sigma,sigmaR_res));
   RooRealVar nL("nL", "nL CB", 4, 3,15, "");
   RooRealVar alphaL("alphaL", "Alpha left CB", 5, 2, 5, "");
   RooRealVar nR("nR", "nR CB", 4, 3,15, "");
   RooRealVar alphaR("alphaL", "Alpha left CB", 5, 2, 5, "");
   RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigmaL, sigmaR, alphaL,nL,alphaR,nR);

   RooRealVar GaussFraction("GaussFraction", "Fraction of Voigtian", 0.5, 0, 1, "");

   // Final model is sigModel
   RooAddPdf sigModel("sigModel", "J/psi mass model", RooArgList(Voigtian, CB), GaussFraction);
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
      sigYield.setVal(1000000);
      sigYield.setRange(0., 100000000);
      bkgYield.setVal(50 );
      bkgYield.setRange(0,1000000);
   }

   else{
      sigYield.setVal(1500000);
      sigYield.setRange(0,10000000);
      bkgYield.setVal(100000);
      bkgYield.setRange(0,1000000);
   }

   // now make the combined models
   std::cout << "make full model" << std::endl;
   RooAddPdf massModel(modelName, "J/psi + bkg mass model", {sigModel, bkgModel}, {sigYield, bkgYield});
 
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
   RooDataSet* data_full = new RooDataSet("data_full", "data_full", RooArgSet(Mm_mass), Import(*tree));
   RooAbsData* data = data_full->emptyClone("data");

   if(nEvents==0) {
      Int_t numEntries = data_full->numEntries();
      for (int i=0; i < numEntries; i++){
         data->add(*data_full->get(i))   ; 
      }
      ws.import(*data);
      return;
   }

   Int_t numEntries = data_full->numEntries();
   TBits outputBits = TBits(numEntries);
   for(int i=0; i<nEvents; i++){
      Int_t rnd = gRandom->Integer(numEntries);
      outputBits.SetBitNumber(rnd);
   }
   Int_t startBit = outputBits.FirstSetBit(0);
   while (startBit < numEntries){
      data->add(*data_full->get(startBit))   ;                                                              
      startBit = outputBits.FirstSetBit(startBit + 1);
   }

   // RooAbsData* data = data_full.reduce( EventRange(0,100000)); //Cut("Mm_mass<9.8||Mm_mass>10.7"),
   // import data into workspacemassModel
   ws.import(*data);

}

//____________________________________
void DoFit(RooWorkspace &ws, const char* modelname, const char* file_name)
{
   RooAbsPdf *massModel = ws.pdf(modelname);
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


   // if (string(modelname)=="dCB_V"){
      // RooRealVar *l = ws.var("l");
      // RooRealVar *sigma = ws.var("sigma");
      // RooRealVar *nL = ws.var("nL");
      // RooRealVar *nR = ws.var("nR");
      // RooRealVar *alphaR = ws.var("alphaR");
      // RooRealVar *alphaL = ws.var("alphaL");
      // l->setConstant(true);
      // nR->setConstant(true);
      // alphaR->setConstant(true);
      // massModel->fitTo(data_mass, Strategy(2));
      // l->setConstant(false);
      // nR->setConstant(false);
      // alphaR->setConstant(false);
      // massModel->fitTo(data_mass, Strategy(2)); //Minos(RooArgSet(*l,*nL,*nR,*alphaL,*alphaR))
   // }
   massModel->fitTo(data_mass, Strategy(2));
   // massModel->graphVizTree(modelname);

   // std::cout<<massModel->getParameters();
   // massModel->printMultiline(std::cout,10);

   std::cout<<sigYield->getVal()<<"  ;   "<<bkgYield->getVal() << "\n \n \n";
   ws.writeToFile(file_name);
}