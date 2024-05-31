#include "multi_plot.C"
using namespace RooFit;

// void plot(const char* name, RooRealVar Mm_mass, RooDataSet* Data, RooAbsPdf* model, RooRealVar n_signal, RooRealVar n_back, ofstream& logFile, bool save=true);

//Attempt to make plotting independent from fitting. did not take off fast enough


void multi_fit(int event_frac = 15)
{
    RooWorkspace ws{"ws"};

    TFile *File_Phi    = TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Phi/merged_A_var.root");
    TTree *DataTree_Phi = (TTree*)File_Phi->Get(("tree"));
    TFile *File_Jpsi    = TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Jpsi/merged_A.root");
    TTree *DataTree_Jpsi = (TTree*)File_Jpsi->Get(("tree"));
    TFile *File_Psi2    = TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Psi2/merged_A_var.root");
    TTree *DataTree_Psi2 = (TTree*)File_Psi2->Get(("tree"));


    // Very annoying, as per root forum need to rename mass variables in ntuples if want to do simfit. 

    RooRealVar Mm_mass_Phi("Mm_mass_Phi", "Mm_mass_Phi", 0.85, 1.17);
    RooRealVar Mm_mass_Jpsi("Mm_mass", "Mm_mass", 2.6, 3.56);
    RooRealVar Mm_mass_Psi2("Mm_mass_Psi2", "Mm_mass_Psi2", 3.25,4.15);
    RooRealVar trigger("HLT_DoubleMu4_3_LowMass", "HLT_DoubleMu4_3_LowMass", 0,1);
    RooRealVar Event("Event", "Event", 0,10000000000);
    string event_reduc = "int(Event)%"+to_string(event_frac)+"==0&&";

    RooFormulaVar* reduce = new RooFormulaVar("PPTM", (event_reduc+"HLT_DoubleMu4_3_LowMass==1").c_str(), RooArgList(Event,trigger));

    RooDataSet *Data_Phi    = new RooDataSet("DATA_Phi", "DATA_Phi", DataTree_Phi, RooArgSet(Event,trigger,Mm_mass_Phi),*reduce);
    RooDataSet *Data_Jpsi    = new RooDataSet("DATA_Jpsi", "DATA_Jpsi", DataTree_Jpsi,  RooArgSet(Event,trigger,Mm_mass_Jpsi),*reduce);
    RooDataSet *Data_Psi2    = new RooDataSet("DATA_Phi", "DATA_Phi", DataTree_Psi2,  RooArgSet(Event,trigger,Mm_mass_Psi2),*reduce);
   
    // RooDataHist* dh_Phi     = Data_Phi->binnedClone();
    // RooDataHist* dh_Jpsi     = Data_Jpsi->binnedClone();
    // RooDataHist* dh_Psi2     = Data_Psi2->binnedClone();

    // SIMULTANEOUS FIT
    RooCategory sample("sample","sample") ;
    sample.defineType("Phi") ;
    sample.defineType("Jpsi") ;
    sample.defineType("Psi2") ;

    // ws.import(sample);

    // Mm_mass_Phi.setBins(dh_Phi->numEntries());   
    // Mm_mass_Jpsi.setBins(dh_Jpsi->numEntries());   
    // Mm_mass_Psi2.setBins(dh_Psi2->numEntries());   
    // RooDataHist combData("combData","combined data",RooArgSet(Mm_mass_Phi,Mm_mass_Jpsi,Mm_mass_Psi2),Index(sample),Import("Phi",*dh_Phi),Import("Jpsi",*dh_Jpsi),Import("Psi2",*dh_Psi2));
    // mean.setConstant(kTRUE)

    auto simPdf = AddModel(ws, sample);
    RooDataSet combData("combData","combined data",RooArgSet(Mm_mass_Phi,Mm_mass_Jpsi,Mm_mass_Psi2),Index(sample),Import("Phi",*Data_Phi),Import("Jpsi",*Data_Jpsi),Import("Psi2",*Data_Psi2));

    //Store logs
    string logpath =  string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/models/Multi_fit_output.log");
    ofstream logFile((logpath).c_str());
    
    RooFitResult* fitres = new RooFitResult;
    fitres = simPdf.fitTo(combData, RooFit::Save(), RooFit::Strategy(1)); //, RooFit::Minos());
    fitres->printMultiline(logFile,0,true);

    string wspath =  string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/models/Multi_fit_output.root");
    ws.writeToFile(wspath.c_str());


    // RooRealVar* yield_ALL = (RooRealVar*) fitres->floatParsFinal().find("n_signal_total");
    // RooRealVar* yield_PASS = (RooRealVar*) fitres->floatParsFinal().find("n_signal_total_pass");

    plot("Phi", Mm_mass_Phi, Data_Phi, model_Phi, n_signal_Phi, n_bkg_Phi, logFile);
    plot("Jpsi", Mm_mass_Jpsi, Data_Jpsi, model_Jpsi, n_signal_Jpsi, n_bkg_Jpsi, logFile);
    plot("Psi2", Mm_mass_Psi2, Data_Psi2, model_Psi2, n_signal_Psi2, n_bkg_Psi2, logFile);

    logFile.close();
    return;
}


RooSimultaneous AddModel(RooWorkspace& ws, RooCategory& sample){
    // mass model for single resonance. Gauss + dCB.
    RooRealVar mu_Phi("mu_Phi", "Phi Mass", 1.019, 0.9, 1.1);
    RooRealVar mu_Jpsi("mu_Jpsi", "J/Psi Mass", 3.09809, 3.05, 3.15);
    RooRealVar mu_Psi2("mu_Psi2", "Psi2 Mass", 3.686, 3.58, 3.75);

    // RooRealVar sigma("sigma", "Width of G Jpsi", 0.05, 0.01, 0.1, "GeV");
    RooRealVar reso_l("reso_l", "Width of BW", 0.05, 0.01, 0.1, "");
    RooRealVar reso("reso", "peak resolution", 0.01, 0.005, 0.05, "");

    RooRealVar q_Phi("q_Phi", "Scale of Voigtian sigma and l phi", 1, 0.1, 5, "");
    RooRealVar q_Psi2("q_Psi2", "Scale of Voigtian sigma and l Psi2", 1, 0.1, 5, "");

    RooFormulaVar sigma_V_Phi("sigma_V_Phi", "reso*mu_Phi*q_Phi", RooArgList(reso, mu_Phi, q_Phi));
    RooFormulaVar l_Phi("l_Phi", "reso_l*mu_Phi*q_Phi", RooArgList(reso_l, mu_Phi, q_Phi));
    RooFormulaVar sigma_V_Jpsi("sigma_V_Jpsi", "reso*mu_Jpsi", RooArgList(reso, mu_Jpsi));
    RooFormulaVar l_Jpsi("l_Jpsi", "reso_l*mu_Jpsi", RooArgList(reso_l, mu_Jpsi));
    RooFormulaVar sigma_V_Psi2("sigma_V_Psi2", "reso*mu_Psi2*q_Psi2", RooArgList(reso, mu_Psi2, q_Psi2));
    RooFormulaVar l_Psi2("l_Psi2", "reso_l*mu_Psi2*q_Psi2", RooArgList(reso_l, mu_Psi2, q_Psi2));

    RooVoigtian Voigtian_Phi("Voigtian_Phi", "Voigtian_Phi", Mm_mass_Phi, mu_Phi, sigma_V_Phi,l_Phi);
    RooVoigtian Voigtian_Jpsi("Voigtian_Jpsi", "Voigtian_Jpsi", Mm_mass_Jpsi, mu_Jpsi, sigma_V_Jpsi,l_Jpsi);
    RooVoigtian Voigtian_Psi2("Voigtian_Psi2", "Voigtian_Psi2", Mm_mass_Psi2, mu_Psi2, sigma_V_Psi2,l_Psi2);

    RooRealVar ql_Phi("ql_Phi", "Scale of sigma of left CB ql_Phi", 1, 0.1, 5, "");
    RooRealVar qr_Phi("qr_Phi", "Scale of sigma of right CB qr_Phi", 1, 0.1, 5, "");
    RooRealVar ql_Jpsi("ql_Jpsi", "Scale of sigma of left CB ql_Jpsi", 1, 0.1, 5, "");
    RooRealVar qr_Jpsi("qr_Jpsi", "Scale of sigma of right CB qr_Jpsi", 1, 0.1, 5, "");
    RooRealVar ql_Psi2("ql_Psi2", "Scale of sigma of left CB ql_Psi2", 1, 0.1, 5, "");
    RooRealVar qr_Psi2("qr_Psi2", "Scale of sigma of right CB qr_Psi2", 1, 0.1, 5, "");

    RooRealVar nL("nL", "nL CB", 5, 1.5,15, "");
    RooRealVar alphaL("alphaR", "Alpha right CB", 5, 2, 5, "");
    RooRealVar nR("nR", "nR CB", 5, 1.5,15, "");
    RooRealVar alphaR("alphaL", "Alpha left CB", 5, 2, 5, "");

    RooFormulaVar sigmaL_Phi("sigmaL_Phi", "reso*mu_Phi*ql_Phi", RooArgList(reso,mu_Phi, ql_Phi));
    RooFormulaVar sigmaR_Phi("sigmaR_Phi", "reso*mu_Phi*qr_Phi", RooArgList(reso,mu_Phi, qr_Phi));
    RooFormulaVar sigmaL_Jpsi("sigmaL_Jpsi", "reso*mu_Jpsi*ql_Jpsi", RooArgList(reso, mu_Jpsi, ql_Jpsi));
    RooFormulaVar sigmaR_Jpsi("sigmaR_Jpsi", "reso*mu_Jpsi*qr_Jpsi", RooArgList(reso, mu_Jpsi, qr_Jpsi));
    RooFormulaVar sigmaL_Psi2("sigmaL_Psi2", "reso*mu_Psi2*ql_Psi2", RooArgList(reso, mu_Psi2, ql_Psi2));
    RooFormulaVar sigmaR_Psi2("sigmaR_Psi2", "reso*mu_Psi2*qr_Psi2", RooArgList(reso, mu_Psi2, qr_Psi2));

    RooCrystalBall CB_Phi("CB_Phi", "CB_Phi", Mm_mass_Phi, mu_Phi,     sigmaL_Phi, sigmaR_Phi,  alphaL,nL,alphaR,nR);
    RooCrystalBall CB_Jpsi("CB_Jpsi", "CB_Jpsi", Mm_mass_Jpsi, mu_Jpsi, sigmaL_Jpsi, sigmaR_Jpsi, alphaL,nL,alphaR,nR);
    RooCrystalBall CB_Psi2("CB_Psi2", "CB_Psi2", Mm_mass_Psi2, mu_Psi2, sigmaL_Psi2, sigmaR_Psi2, alphaL,nL,alphaR,nR);

    RooRealVar GaussFraction("GaussFraction", "Fraction of Voigtian", 0.55, 0, 0.8, "");

    RooAddPdf sigModel_Phi("sigModel_Phi", "Phi mass model", RooArgList(Voigtian_Phi, CB_Phi), GaussFraction);
    RooAddPdf sigModel_Jpsi("sigModel_Jpsi", "J/psi mass model", RooArgList(Voigtian_Jpsi, CB_Jpsi), GaussFraction);
    RooAddPdf sigModel_Psi2("sigModel_Psi2", "psi2 mass model", RooArgList(Voigtian_Psi2, CB_Psi2), GaussFraction);
    // --------------------------------------

    // Make three separate bkg models 

    RooRealVar bkgDecayConst_Phi("bkgDecayConst_Phi", "Decay const for bkg mass spectrum", -2., -3, 3, "1/GeV");
    RooExponential  bkgModel_Phi ("bkgModel_Phi", "bkg Mass Model", Mm_mass_Phi, bkgDecayConst_Phi);
    RooRealVar bkgDecayConst_Jpsi("bkgDecayConst_Jpsi", "Decay const for bkg mass spectrum", -2., -3, 3, "1/GeV");
    RooExponential  bkgModel_Jpsi ("bkgModel_Jpsi", "bkg Mass Model", Mm_mass_Jpsi, bkgDecayConst_Jpsi);
    RooRealVar bkgDecayConst_Psi2("bkgDecayConst_Psi2", "Decay const for bkg mass spectrum", -2., -3, 3, "1/GeV");
    RooExponential  bkgModel_Psi2 ("bkgModel_Psi2", "bkg Mass Model", Mm_mass_Psi2, bkgDecayConst_Psi2);

    //Estimate initial yield for each resonance
    double n_signal_initial_Phi =(Data_Phi->sumEntries("abs(Mm_mass_Phi-1.019)<0.1"));
    double n_bkg_initial_Phi = (Data_Phi->sumEntries("abs(Mm_mass_Phi-1.019)>0.3"));
    double n_signal_initial_Jpsi =(Data_Jpsi->sumEntries("abs(Mm_mass-3.098)<0.1"));
    double n_bkg_initial_Jpsi = (Data_Jpsi->sumEntries("abs(Mm_mass-3.098)>0.3"));
    double n_signal_initial_Psi2 =(Data_Psi2->sumEntries("abs(Mm_mass_Psi2-3.686)<0.1"));
    double n_bkg_initial_Psi2 = (Data_Psi2->sumEntries("abs(Mm_mass_Psi2-3.686)>0.3"));

    RooRealVar n_signal_Phi("n_signal_Phi","n_signal_Phi",n_signal_initial_Phi,0.,Data_Phi->sumEntries());
    RooRealVar n_signal_Jpsi("n_signal_Jpsi","n_signal_Jpsi",n_signal_initial_Jpsi,0.,Data_Jpsi->sumEntries());
    RooRealVar n_signal_Psi2("n_signal_Psi2","n_signal_Psi2",n_signal_initial_Psi2,0.,Data_Psi2->sumEntries());

    cout << "\nTotal entries Phi: " << Data_Phi->sumEntries() << "\nTotal entries Jpsi: " << Data_Jpsi->sumEntries() << "\nTotal entries Psi2: " << Data_Psi2->sumEntries();

    RooRealVar n_bkg_Phi("n_bkg_Phi","n_bkg_Phi",n_bkg_initial_Phi,0.,Data_Phi->sumEntries());
    RooRealVar n_bkg_Jpsi("n_bkg_Jpsi","n_bkg_Jpsi",n_bkg_initial_Jpsi,0.,Data_Jpsi->sumEntries());
    RooRealVar n_bkg_Psi2("n_bkg_Psi2","n_bkg_Psi2",n_bkg_initial_Psi2,0.,Data_Psi2->sumEntries());

    RooAddPdf* model_Phi;
    RooAddPdf* model_Jpsi;
    RooAddPdf* model_Psi2;
    model_Phi      = new RooAddPdf("model_Phi","model_Phi", RooArgList(sigModel_Phi, bkgModel_Phi),RooArgList(n_signal_Phi, n_bkg_Phi));
    model_Jpsi      = new RooAddPdf("model_Jpsi","model_Jpsi", RooArgList(sigModel_Jpsi, bkgModel_Jpsi),RooArgList(n_signal_Jpsi, n_bkg_Jpsi));
    model_Psi2      = new RooAddPdf("model_Psi2","model_Psi2", RooArgList(sigModel_Psi2, bkgModel_Psi2),RooArgList(n_signal_Psi2, n_bkg_Psi2));

    RooSimultaneous simPdf("simPdf","simultaneous pdf",sample) ;

    simPdf.addPdf(*model_Phi,"Phi");
    simPdf.addPdf(*model_Jpsi,"Jpsi");
    simPdf.addPdf(*model_Psi2,"Psi2");

    ws.import(simPdf);
    return simPdf;
}