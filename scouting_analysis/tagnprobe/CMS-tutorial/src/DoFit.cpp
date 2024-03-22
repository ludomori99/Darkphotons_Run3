using namespace RooFit;

double* doFit(string condition, string MuonID_str, string quant, double* init_conditions, bool save = true) // RETURNS ARRAY WITH [yield_all, yield_pass, err_all, err_pass]    ->   OUTPUT ARRAY
{
    TFile *file0    = TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Jpsi/TP_samples_Jpsi.root");
    TTree *DataTree = (TTree*)file0->Get(("tree"));
    
    double lowRange = 2.6;  double highRange = 3.56;
    
    RooRealVar MuonID(MuonID_str.c_str(), MuonID_str.c_str(), 0, 1); //Muon_Id
    
    RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange);
    
    double* limits = new double[2];
    if (quant == "Probe_pt") {
        limits[0] = 0;
        limits[1] = 40;
    }
    if (quant == "Eta") {
        limits[0] = -3;
        limits[1] = 3;
    }
    RooRealVar quantity(quant.c_str(), quant.c_str(), limits[0], limits[1]);
    
    RooFormulaVar* redeuce = new RooFormulaVar("PPTM", condition.c_str(), RooArgList(quantity));
    RooDataSet *Data_ALL    = new RooDataSet("DATA", "DATA", DataTree, RooArgSet(quantity, MuonID,Mm_mass),*redeuce);
    RooFormulaVar* cutvar = new RooFormulaVar("PPTM", (condition + " && " + MuonID_str + " == 1").c_str() , RooArgList(quantity,MuonID));

    RooDataSet *Data_PASSING = new RooDataSet("DATA_PASS", "DATA_PASS", DataTree, RooArgSet(quantity,MuonID,Mm_mass), *cutvar);
    
    RooDataHist* dh_ALL     = Data_ALL->binnedClone();
    RooDataHist* dh_PASSING = Data_PASSING->binnedClone();
    
    TCanvas* c_all  = new TCanvas("ALL","ALL",800,800);
    TCanvas* c_pass = new TCanvas("PASS","PASS",800,800);
    
    RooPlot *frame = Mm_mass.frame(RooFit::Title("Invariant Mass"));

    // mass model for single resonance. Gauss + dCB. In this case employed for Jpsi
    RooRealVar mu("mu", "J/Psi Mass", 3.09809, lowRange, highRange);
    RooRealVar sigma("sigma", "Width of Gaussian", 0.0378, 0.001, 10, "GeV");
    RooRealVar l("l", "Width of BW", 0.0378, 0.001, 10, "GeV");
    RooVoigtian Voigtian("Voigtian", "Voigtian", Mm_mass, mu, sigma,l);

    RooRealVar sigmaL("sigmaL", "Width of left CB", 0.01956, 0.001, 10, "GeV");
    RooRealVar sigmaR("sigmaR", "Width of right CB", 0.01957, 0.001, 11, "GeV");
    RooRealVar nL("nL", "nL CB", 0.8, 0.1,15, "");
    RooRealVar alphaL("alphaR", "Alpha right CB", 2.5, 0.001, 5, "");
    RooRealVar nR("nR", "nR CB", 0.3, 0.1,15, "");
    RooRealVar alphaR("alphaL", "Alpha left CB", 1.7, 0.001, 5, "");
    RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigmaL, sigmaR, alphaL,nL,alphaR,nR);

    RooRealVar GaussFraction("GaussFraction", "Fraction of Voigtian", 0.5, 0, 1, "");

    // Final model is sigModel
    RooAddPdf sigModel("sigModel", "J/psi mass model", RooArgList(Voigtian, CB), GaussFraction);
    // --------------------------------------
    RooRealVar bkgDecayConst("bkgDecayConst", "Decay const for bkg mass spectrum", -2., -100, 100, "1/GeV");
    RooExponential bkgModel("bkgModel", "bkg Mass Model", Mm_mass, bkgDecayConst);
    
    double n_signal_initial =(Data_ALL->sumEntries(TString::Format("abs(Mm_mass-%g)<0.1",init_conditions[1])));
    double n_back_initial = (Data_ALL->sumEntries(TString::Format("abs(Mm_mass-%g)>0.3",init_conditions[1])));
    
    RooRealVar n_signal_total("n_signal_total","n_signal_total",n_signal_initial,0.,Data_ALL->sumEntries());
    RooRealVar n_signal_total_pass("n_signal_total_pass","n_signal_total_pass",n_signal_initial,0.,Data_PASSING->sumEntries());
    
    RooRealVar n_back("n_back","n_back",n_back_initial,0.,Data_ALL->sumEntries());
    RooRealVar n_back_pass("n_back_pass","n_back_pass",n_back_initial,0.,Data_PASSING->sumEntries());

    RooAddPdf* model;
    RooAddPdf* model_pass;
    model      = new RooAddPdf("model","model", RooArgList(sigModel, bkgModel),RooArgList(n_signal_total, n_back));
    model_pass = new RooAddPdf("model_pass", "model_pass", RooArgList(sigModel, bkgModel),RooArgList(n_signal_total_pass, n_back_pass));
    
    // SIMULTANEOUS FIT
    RooCategory sample("sample","sample") ;
    sample.defineType("All") ;
    sample.defineType("PASSING") ;
    
    RooDataHist combData("combData","combined data",Mm_mass,Index(sample),Import("ALL",*dh_ALL),Import("PASSING",*dh_PASSING));
    
    RooSimultaneous simPdf("simPdf","simultaneous pdf",sample) ;
    
    simPdf.addPdf(*model,"ALL");
    simPdf.addPdf(*model_pass,"PASSING");
    
    RooFitResult* fitres = new RooFitResult;
    fitres = simPdf.fitTo(combData, RooFit::Save());
    
    // OUTPUT ARRAY
    double* output = new double[4];
    
    RooRealVar* yield_ALL = (RooRealVar*) fitres->floatParsFinal().find("n_signal_total");
    RooRealVar* yield_PASS = (RooRealVar*) fitres->floatParsFinal().find("n_signal_total_pass");
    
    output[0] = yield_ALL->getVal();
    output[1] = yield_PASS->getVal();
    
    output[2] = yield_ALL->getError();
    output[3] = yield_PASS->getError();

    frame->SetTitle("ALL");
    frame->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");
    Data_ALL->plotOn(frame);
    
    model->plotOn(frame);
    model->plotOn(frame,RooFit::Components("sigModel"),RooFit::LineStyle(kDashed),RooFit::LineColor(kGreen));
    model->plotOn(frame,RooFit::Components("bkgModel"),RooFit::LineStyle(kDashed),RooFit::LineColor(kRed));
    
    c_all->cd();
    frame->Draw("");
    
    RooPlot *frame_pass = Mm_mass.frame(RooFit::Title("Invariant Mass"));
    
    c_pass->cd();
    
    frame_pass->SetTitle("PASSING");
    frame_pass->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");
    Data_PASSING->plotOn(frame_pass);
    
    model_pass->plotOn(frame_pass);
    model_pass->plotOn(frame_pass,RooFit::Components("sigModel"),RooFit::LineStyle(kDashed),RooFit::LineColor(kGreen));
    model_pass->plotOn(frame_pass,RooFit::Components("bkgModel"),RooFit::LineStyle(kDashed),RooFit::LineColor(kRed));
    
    frame_pass->Draw();

    if(save)
    {
        c_pass->SaveAs(("/work/submit/mori25/Darkphotons_ludo/tagnprobe/CMS-tutorial/Fit Result/" + condition + "_PASS.png").c_str());
        c_all->SaveAs (("/work/submit/mori25/Darkphotons_ludo/tagnprobe/CMS-tutorial/Fit Result/" + condition + "_ALL.png").c_str());
    }
        
    // DELETING ALLOCATED MEMORY
    delete[] limits;
    //
    delete file0;
    //
    delete Data_ALL;
    delete Data_PASSING;
    //
    delete dh_ALL;
    delete dh_PASSING;
    //
    delete cutvar;
    delete redeuce;
    //
    delete c_all;
    delete c_pass;
    //
    delete model;
    delete model_pass;
    delete fitres;
    
    return output;
}