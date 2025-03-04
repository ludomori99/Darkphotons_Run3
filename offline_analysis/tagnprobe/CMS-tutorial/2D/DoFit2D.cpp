#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/fit_with_pulls.C"

double* doFit2D(const char* filepath, string condition, string MuonID_str, string quantx, string quanty,bool fix_l = true) // RETURNS ARRAY WITH [yield_all, yield_pass, err_all, err_pass]    ->   OUTPUT ARRAY
{
    TFile *file0    = TFile::Open(filepath);
    TTree *DataTree = (TTree*)file0->Get(("tree"));
    double lowRange = 2.6;  double highRange = 3.56;

    RooRealVar MuonID(MuonID_str.c_str(), MuonID_str.c_str(), 0, 1); //Muon_Id
    RooRealVar isBarrelMuon("isBarrelMuon","isBarrelMuon",0,1);
    RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange);
    RooRealVar trigger("HLT_DoubleMu4_3_LowMass", "HLT_DoubleMu4_3_LowMass", 0,1);

    RooRealVar xquantity((quantx).c_str(), (quantx).c_str(),0,100);
    RooRealVar yquantity((quanty).c_str(), (quanty).c_str(),0,100);
 
    RooFormulaVar* reduce = new RooFormulaVar("PPTM", condition.c_str(), RooArgList(trigger, xquantity,yquantity));
    RooFormulaVar* cutvar = new RooFormulaVar("PPTM", (condition + "&&" + MuonID_str + "==1").c_str() , RooArgList(trigger,xquantity,yquantity,MuonID));
    RooDataSet *Data_ALL    = new RooDataSet("DATA", "DATA", DataTree,RooArgSet(trigger,xquantity,yquantity,MuonID,Mm_mass),*reduce);
    RooDataSet *Data_PASSING = new RooDataSet("DATA_PASS", "DATA_PASS", DataTree, RooArgSet(trigger,xquantity,yquantity,MuonID,Mm_mass), *cutvar);


    // mass model for single resonance. Gauss + dCB. In this case employed for Jpsi
    RooRealVar mu("mu", "J/Psi Mass", 3.09809, lowRange, highRange);
    RooRealVar sigma("sigma", "Width of Gaussian", 0.02, 0.007, 0.1, "GeV");
    RooRealVar l("l", "Width of BW", 0.06, 0.007, 0.1, "GeV");
    RooVoigtian Voigtian("Voigtian", "Voigtian", Mm_mass, mu, l, sigma);
    
    // l.setConstant(kTRUE);

    RooRealVar sigmaL("sigmaL", "Width of left CB", 0.02, 0.01, 0.08, "GeV");
    RooRealVar sigmaR("sigmaR", "Width of right CB", 0.02, 0.01, 0.08, "GeV");
    RooRealVar nL("nL", "nL CB", 2, 2,15, "");
    RooRealVar alphaL("alphaR", "Alpha right CB", 5, 1, 5, "");
    RooRealVar nR("nR", "nR CB", 2, 2,15, "");
    RooRealVar alphaR("alphaL", "Alpha left CB", 5, 1, 5, "");



    // CHECK FIXS OR NOT 

    // RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigmaL, sigmaR, alphaL,nL,alphaR,nR);
    RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigma, sigma, alphaL,nL,alphaR,nR);

    RooRealVar GaussFraction("GaussFraction", "Fraction of Voigtian", 0.55, 0, 0.8, "");

    // Final model is sigModel
    RooAddPdf sigModel("sigModel", "J/psi mass model", RooArgList(Voigtian, CB), GaussFraction);
    // --------------------------------------
    RooRealVar bkgDecayConst("bkgDecayConst", "Decay const for bkg mass spectrum", -2., -100, 100, "1/GeV");
    RooExponential bkgModel("bkgModel", "bkg Mass Model", Mm_mass, bkgDecayConst);

    double n_signal_initial =(Data_ALL->sumEntries(TString::Format("abs(Mm_mass-%g)<0.1",3.09809)));
    double n_back_initial = (Data_ALL->sumEntries(TString::Format("abs(Mm_mass-%g)>0.3",3.09809)));

    RooRealVar n_signal_total("n_signal_total","n_signal_total",n_signal_initial,0.,Data_ALL->sumEntries());
    RooRealVar n_signal_total_pass("n_signal_total_pass","n_signal_total_pass",n_signal_initial,0.,Data_PASSING->sumEntries());

    RooRealVar n_back("n_back","n_back",2*n_back_initial,0.,Data_ALL->sumEntries());
    RooRealVar n_back_pass("n_back_pass","n_back_pass",2*n_back_initial,0.,Data_PASSING->sumEntries());

    RooAddPdf* model;
    RooAddPdf* model_pass;
    model      = new RooAddPdf("model","model", RooArgList(sigModel, bkgModel),RooArgList(n_signal_total, n_back));
    model_pass = new RooAddPdf("model_pass", "model_pass", RooArgList(sigModel, bkgModel),RooArgList(n_signal_total_pass, n_back_pass));
    // SIMULTANEOUS FIT
    RooCategory sample("sample","sample") ;
    sample.defineType("ALL") ;
    sample.defineType("PASSING") ;

    RooDataHist* dh_ALL     = new RooDataHist(Data_ALL->GetName(),    Data_ALL->GetTitle(),     RooArgSet(Mm_mass), *Data_ALL);
	RooDataHist* dh_PASSING = new RooDataHist(Data_PASSING->GetName(),Data_PASSING->GetTitle(), RooArgSet(Mm_mass), *Data_PASSING);
	
    
    RooDataHist combData("combData","combined data",Mm_mass,Index(sample),Import("ALL",*dh_ALL),Import("PASSING",*dh_PASSING));

    RooSimultaneous simPdf("simPdf","simultaneous pdf",sample) ;

    simPdf.addPdf(*model,"ALL");
    simPdf.addPdf(*model_pass,"PASSING");

    //Store logs
    string logpath =  "/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits_fixS/" + MuonID_str + "/" + quantx+"_"+quanty + "/" + condition + "_output.log";
    ofstream logFile((logpath).c_str());
    
    RooFitResult* fitres = new RooFitResult;
    fitres = simPdf.fitTo(combData, RooFit::Save(), RooFit::Strategy(2), RooFit::Minos());
    fitres->printMultiline(logFile,0,true);
    cout<<"fit done \n";
    // OUTPUT ARRAY
    double* output = new double[4];

    RooRealVar* yield_ALL = (RooRealVar*) fitres->floatParsFinal().find("n_signal_total");
    RooRealVar* yield_PASS = (RooRealVar*) fitres->floatParsFinal().find("n_signal_total_pass");

    output[0] = yield_ALL->getVal();
    output[1] = yield_PASS->getVal();

    output[2] = yield_ALL->getError();
    output[3] = yield_PASS->getError();



    /// CHECK FIXS OR NOT
    string path = "/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits_fixS/" + MuonID_str + "/" + quantx+"_"+quanty + "/" + condition;

    plot("", Mm_mass, Data_ALL, model, n_signal_total, n_back, logFile, path+"_ALL.png");
    plot("", Mm_mass, Data_PASSING, model, n_signal_total_pass, n_back_pass, logFile, path+"_PASS.png");


    delete Data_ALL;
    delete Data_PASSING;
    //
    delete dh_ALL;
    delete dh_PASSING;
    //
    delete cutvar;
    delete reduce;
    // //
    return output;
}

