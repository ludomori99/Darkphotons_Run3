
#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/CMS.C"
#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/setTDRStyle.C"
#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/sandbox.C"
#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/fit_with_pulls.C"


void multi_fit(int event_frac = 15)
{

    sandbox(); //To absorb the weirdly behaving text in the first plot 

    TFile *File_Phi    = TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Phi/merged_A_var.root");
    TTree *DataTree_Phi = (TTree*)File_Phi->Get(("tree"));
    TFile *File_Jpsi    = TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Jpsi/merged_A.root");
    TTree *DataTree_Jpsi = (TTree*)File_Jpsi->Get(("tree"));
    TFile *File_Psi2    = TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Psi2/merged_A_var.root");
    TTree *DataTree_Psi2 = (TTree*)File_Psi2->Get(("tree"));



    // Very annoying, as per root forum need to rename mass variables in ntuples if want to do simfit. 

    RooRealVar Mm_mass_Phi("Mm_mass_Phi", "Mm_mass_Phi", 0.85, 1.17);
    RooRealVar Mm_mass_Jpsi("Mm_mass", "Mm_mass", 2.6, 3.56);
    // RooRealVar Mm_mass_Psi2("Mm_mass_Psi2", "Mm_mass_Psi2", 3.25,4.15);
    RooRealVar Mm_mass_Psi2("Mm_mass_Psi2", "Mm_mass_Psi2", 3.3,4.1);
    RooRealVar trigger("HLT_DoubleMu4_3_LowMass", "HLT_DoubleMu4_3_LowMass", 0,1);
    RooRealVar Event("Event", "Event", 0,10000000000);
    string event_reduc = "int(Event)%"+to_string(event_frac)+"==0&&";

    RooFormulaVar* reduce = new RooFormulaVar("PPTM", (event_reduc+"HLT_DoubleMu4_3_LowMass==1").c_str(), RooArgList(Event,trigger));

    RooDataSet *Data_Phi    = new RooDataSet("DATA_Phi", "DATA_Phi", DataTree_Phi, RooArgSet(Event,trigger,Mm_mass_Phi),*reduce);
    RooDataSet *Data_Jpsi    = new RooDataSet("DATA_Jpsi", "DATA_Jpsi", DataTree_Jpsi,  RooArgSet(Event,trigger,Mm_mass_Jpsi),*reduce);
    RooDataSet *Data_Psi2    = new RooDataSet("DATA_Phi", "DATA_Phi", DataTree_Psi2,  RooArgSet(Event,trigger,Mm_mass_Psi2),*reduce);

    // mass model for single resonance. Gauss + dCB.
    RooRealVar mu_Phi("mu_Phi", "Phi Mass", 1.019, 0.9, 1.1);
    RooRealVar mu_Jpsi("mu_Jpsi", "J/Psi Mass", 3.095, 3.05, 3.15);
    RooRealVar mu_Psi2("mu_Psi2", "Psi2 Mass", 3.686, 3.58, 3.75);

    // RooRealVar sigma("sigma", "Width of G Jpsi", 0.05, 0.01, 0.1, "GeV");
    RooRealVar reso("reso", "peak resolution", 0.0105, 0.005, 0.05, "");

    RooRealVar q_Phi("q_Phi", "Scale of Voigtian/Gaussian sigma and l phi", 1, 0.5, 2, "");
    RooRealVar q_Psi2("q_Psi2", "Scale of Voigtian/Gaussian sigma and l Psi2", 1, 0.5, 2, "");

    RooFormulaVar sigma_V_Phi("sigma_V_Phi", "reso*mu_Phi*q_Phi", RooArgList(reso, mu_Phi,q_Phi)); 
    RooFormulaVar sigma_V_Jpsi("sigma_V_Jpsi", "reso*mu_Jpsi", RooArgList(reso, mu_Jpsi));
    RooFormulaVar sigma_V_Psi2("sigma_V_Psi2", "reso*mu_Psi2*q_Psi2", RooArgList(reso, mu_Psi2, q_Psi2));

    RooRealVar ql_Phi("ql_Phi", "Scale of sigma of left CB ql_Phi", 1, 0.5, 2, "");
    RooRealVar qr_Phi("qr_Phi", "Scale of sigma of right CB qr_Phi", 1, 0.5, 2, "");
    RooRealVar ql_Jpsi("ql_Jpsi", "Scale of sigma of left CB ql_Jpsi", 0.66, 0.5, 2, "");
    RooRealVar qr_Jpsi("qr_Jpsi", "Scale of sigma of right CB qr_Jpsi", 0.6, 0.5, 2, "");
    RooRealVar ql_Psi2("ql_Psi2", "Scale of sigma of left CB ql_Psi2", 0.66, 0.5, 2, "");
    RooRealVar qr_Psi2("qr_Psi2", "Scale of sigma of right CB qr_Psi2", 0.55, 0.5, 2, "");

    RooRealVar nL("nL", "nL CB", 1.36, 1,10, "");
    RooRealVar alphaL("alphaL", "Alpha left CB", 1.57, 1, 5, "");
    RooRealVar nR("nR", "nR CB", 3.05, 1,10, "");
    RooRealVar alphaR("alphaR", "Alpha right CB", 2.2, 2, 5, "");

    // RooFormulaVar sigmaL_Phi("sigmaL_Phi", "reso*mu_Phi*ql_Phi", RooArgList(reso,mu_Phi, ql_Phi));
    // RooFormulaVar sigmaR_Phi("sigmaR_Phi", "reso*mu_Phi*qr_Phi", RooArgList(reso,mu_Phi, qr_Phi));
    // RooFormulaVar sigmaL_Jpsi("sigmaL_Jpsi", "reso*mu_Jpsi*ql_Jpsi", RooArgList(reso, mu_Jpsi, ql_Jpsi));
    // RooFormulaVar sigmaR_Jpsi("sigmaR_Jpsi", "reso*mu_Jpsi*qr_Jpsi", RooArgList(reso, mu_Jpsi, qr_Jpsi));
    // RooFormulaVar sigmaL_Psi2("sigmaL_Psi2", "reso*mu_Psi2*ql_Psi2", RooArgList(reso, mu_Psi2, ql_Psi2));
    // RooFormulaVar sigmaR_Psi2("sigmaR_Psi2", "reso*mu_Psi2*qr_Psi2", RooArgList(reso, mu_Psi2, qr_Psi2))

    RooFormulaVar sigmaL_Phi("sigmaL_Phi", "reso*mu_Phi*q_Phi", RooArgList(reso,mu_Phi, q_Phi));
    RooFormulaVar sigmaR_Phi("sigmaR_Phi", "reso*mu_Phi*q_Phi", RooArgList(reso,mu_Phi, q_Phi));
    RooFormulaVar sigmaL_Jpsi("sigmaL_Jpsi", "reso*mu_Jpsi", RooArgList(reso, mu_Jpsi));
    RooFormulaVar sigmaR_Jpsi("sigmaR_Jpsi", "reso*mu_Jpsi", RooArgList(reso, mu_Jpsi));
    RooFormulaVar sigmaL_Psi2("sigmaL_Psi2", "reso*mu_Psi2*q_Psi2", RooArgList(reso, mu_Psi2, q_Psi2));
    RooFormulaVar sigmaR_Psi2("sigmaR_Psi2", "reso*mu_Psi2*q_Psi2", RooArgList(reso, mu_Psi2, q_Psi2));

    RooCrystalBall CB_Phi("CB_Phi", "CB_Phi", Mm_mass_Phi, mu_Phi,     sigmaL_Phi, sigmaR_Phi,  alphaL,nL,alphaR,nR);
    RooCrystalBall CB_Jpsi("CB_Jpsi", "CB_Jpsi", Mm_mass_Jpsi, mu_Jpsi, sigmaL_Jpsi, sigmaR_Jpsi, alphaL,nL,alphaR,nR);
    RooCrystalBall CB_Psi2("CB_Psi2", "CB_Psi2", Mm_mass_Psi2, mu_Psi2, sigmaL_Psi2, sigmaR_Psi2, alphaL,nL,alphaR,nR);

    RooRealVar GaussFraction("GaussFraction", "Fraction of Voigtian", 0.55, 0, 0.8, "");


    //Choice to make: Gassian or Voigtian? 

    // string model = "VdCB_var";
    // string model = "GdCB_var";

    // string model = "VdCB_var_fixReso";
    string model = "GdCB_var_fixReso";

    //Gaussian

    RooGaussian Gaussian_Phi("Gaussian_Phi", "Gaussian_Phi", Mm_mass_Phi, mu_Phi, sigma_V_Phi);
    RooGaussian Gaussian_Jpsi("Gaussian_Jpsi", "Gaussian_Jpsi", Mm_mass_Jpsi, mu_Jpsi, sigma_V_Jpsi);
    RooGaussian Gaussian_Psi2("Gaussian_Psi2", "Gaussian_Psi2", Mm_mass_Psi2, mu_Psi2, sigma_V_Psi2);

    RooAddPdf sigModel_Phi("sigModel_Phi", "Phi mass model", RooArgList(Gaussian_Phi, CB_Phi), GaussFraction);
    RooAddPdf sigModel_Jpsi("sigModel_Jpsi", "J/psi mass model", RooArgList(Gaussian_Jpsi, CB_Jpsi), GaussFraction);
    RooAddPdf sigModel_Psi2("sigModel_Psi2", "psi2 mass model", RooArgList(Gaussian_Psi2, CB_Psi2), GaussFraction);


    //Voigtian

    // RooRealVar reso_l("reso_l", "Width of BW", 0.0012, 0.001, 0.1, "");

    // RooFormulaVar l_Phi("l_Phi", "reso_l*mu_Phi*q_Phi", RooArgList(reso_l, mu_Phi, q_Phi));
    // RooFormulaVar l_Jpsi("l_Jpsi", "reso_l*mu_Jpsi", RooArgList(reso_l, mu_Jpsi));
    // RooFormulaVar l_Psi2("l_Psi2", "reso_l*mu_Psi2*q_Psi2", RooArgList(reso_l, mu_Psi2, q_Psi2));
    // RooVoigtian Voigtian_Phi("Voigtian_Phi", "Voigtian_Phi", Mm_mass_Phi, mu_Phi,l_Phi,sigma_V_Phi);
    // RooVoigtian Voigtian_Jpsi("Voigtian_Jpsi", "Voigtian_Jpsi", Mm_mass_Jpsi, mu_Jpsi,l_Jpsi,sigma_V_Jpsi);
    // RooVoigtian Voigtian_Psi2("Voigtian_Psi2", "Voigtian_Psi2", Mm_mass_Psi2, mu_Psi2,l_Psi2,sigma_V_Psi2);

    // RooAddPdf sigModel_Phi("sigModel_Phi", "Phi mass model", RooArgList(Voigtian_Phi, CB_Phi), GaussFraction);
    // RooAddPdf sigModel_Jpsi("sigModel_Jpsi", "J/psi mass model", RooArgList(Voigtian_Jpsi, CB_Jpsi), GaussFraction);
    // RooAddPdf sigModel_Psi2("sigModel_Psi2", "psi2 mass model", RooArgList(Voigtian_Psi2, CB_Psi2), GaussFraction);
    // --------------------------------------


    //fix reso

    q_Phi.setConstant(kTRUE);
    q_Psi2.setConstant(kTRUE);

    //probably better don't set  constant
    // reso_l.setConstant(kTRUE);
    // qr_Psi2.setConstant(kTRUE);
    // ql_Phi.setConstant(kTRUE);
    // qr_Phi.setConstant(kTRUE);
    // ql_Psi2.setConstant(kTRUE);



    // Make three separate bkg models 

    RooRealVar bkgDecayConst_Phi("bkgDecayConst_Phi", "Decay const for bkg mass spectrum", 0.25, -3, 3, "1/GeV");
    RooExponential  bkgModel_Phi ("bkgModel_Phi", "bkg Mass Model", Mm_mass_Phi, bkgDecayConst_Phi);
    RooRealVar bkgDecayConst_Jpsi("bkgDecayConst_Jpsi", "Decay const for bkg mass spectrum", -1.3, -3, 3, "1/GeV");
    RooExponential  bkgModel_Jpsi ("bkgModel_Jpsi", "bkg Mass Model", Mm_mass_Jpsi, bkgDecayConst_Jpsi);
    RooRealVar bkgDecayConst_Psi2("bkgDecayConst_Psi2", "Decay const for bkg mass spectrum", -1.07, -3, 3, "1/GeV");
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


    // SIMULTANEOUS FIT
    RooCategory sample("sample","sample") ;
    sample.defineType("Phi") ;
    sample.defineType("Jpsi") ;
    sample.defineType("Psi2") ;

    // mean.setConstant(kTRUE)

    RooDataSet combData("combData","combined data",RooArgSet(Mm_mass_Phi,Mm_mass_Jpsi,Mm_mass_Psi2),Index(sample),Import("Phi",*Data_Phi),Import("Jpsi",*Data_Jpsi),Import("Psi2",*Data_Psi2));
    RooSimultaneous simPdf("simPdf","simultaneous pdf",sample) ;

    simPdf.addPdf(*model_Phi,"Phi");
    simPdf.addPdf(*model_Jpsi,"Jpsi");
    simPdf.addPdf(*model_Psi2,"Psi2");

    //Store logs
    string logpath =  string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/models/")+string("Multi_fit_output_") + model + string(".log");
    ofstream logFile((logpath).c_str());
    
    RooFitResult* fitres = new RooFitResult;
    fitres = simPdf.fitTo(combData, RooFit::Save(), RooFit::Strategy(1)); //, RooFit::Minos());
    fitres->printMultiline(logFile,0,true);

    // RooDataHist combHist("comb hist","combined hist",Mm_mass,Index(sample),Import("ALL",*dh_ALL),Import("PASSING",*dh_PASSING));
    // RooDataHist* combHist = combData.binnedClone();
    
    // RooAbsReal * chi2_o_ndf = simPdf.createChi2(combData,Extended(true), DataError(RooAbsData::Poisson));
    // RooAbsCollection *flparams = simPdf->getParameters(combData)->selectByAttrib("Constant", kFALSE);
    // Int_t nflparams = flparams->getSize();
    // Int_t nbins = combData.();
    // logFile << "global chi2/ndof<: " << chi2_o_ndf->GetValV() <<"/"<<nflparams << endl;

    string path = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/multi_fit_";

    plot("_Phi", Mm_mass_Phi, Data_Phi, model_Phi, n_signal_Phi, n_bkg_Phi, logFile, path + "Phi_" + model + string(".png"), true, true);
    plot("_Jpsi", Mm_mass_Jpsi, Data_Jpsi, model_Jpsi, n_signal_Jpsi, n_bkg_Jpsi, logFile,path + "Jpsi_" + model + string(".png"), true, true);
    plot("_Psi2", Mm_mass_Psi2, Data_Psi2, model_Psi2, n_signal_Psi2, n_bkg_Psi2, logFile,path + "Psi2_" + model + string(".png"), true, true);

    logFile.close();
    return;
}
