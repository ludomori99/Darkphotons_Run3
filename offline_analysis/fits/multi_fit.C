#include "CMS.C"

using namespace RooFit;

void plot(const char* name, RooRealVar Mm_mass, RooDataSet* Data, RooAbsPdf* model, RooRealVar n_signal, RooRealVar n_back, ofstream& logFile, bool save=true);


void multi_fit(int event_frac = 15)
{
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

    // mass model for single resonance. Gauss + dCB.
    RooRealVar mu_Phi("mu_Phi", "Phi Mass", 1.019, 0.9, 1.1);
    RooRealVar mu_Jpsi("mu_Jpsi", "J/Psi Mass", 3.09809, 3.05, 3.15);
    RooRealVar mu_Psi2("mu_Psi2", "Psi2 Mass", 3.686, 3.58, 3.75);

    // RooRealVar sigma("sigma", "Width of G Jpsi", 0.05, 0.01, 0.1, "GeV");
    RooRealVar reso("reso", "peak resolution", 0.05, 0.01, 0.05, "");

    RooRealVar q_Phi("q_Phi", "Scale of Voigtian/Gaussian sigma and l phi", 1, 0.1, 5, "");
    RooRealVar q_Psi2("q_Psi2", "Scale of Voigtian/Gaussian sigma and l Psi2", 1, 0.1, 5, "");

    RooFormulaVar sigma_V_Phi("sigma_V_Phi", "reso*mu_Phi*q_Phi", RooArgList(reso, mu_Phi,q_Phi)); 
    RooFormulaVar sigma_V_Jpsi("sigma_V_Jpsi", "reso*mu_Jpsi", RooArgList(reso, mu_Jpsi));
    RooFormulaVar sigma_V_Psi2("sigma_V_Psi2", "reso*mu_Psi2*q_Psi2", RooArgList(reso, mu_Psi2, q_Psi2));

    RooRealVar ql_Phi("ql_Phi", "Scale of sigma of left CB ql_Phi", 1, 0.1, 5, "");
    RooRealVar qr_Phi("qr_Phi", "Scale of sigma of right CB qr_Phi", 1, 0.1, 5, "");
    RooRealVar ql_Jpsi("ql_Jpsi", "Scale of sigma of left CB ql_Jpsi", 1, 0.1, 5, "");
    RooRealVar qr_Jpsi("qr_Jpsi", "Scale of sigma of right CB qr_Jpsi", 1, 0.1, 5, "");
    RooRealVar ql_Psi2("ql_Psi2", "Scale of sigma of left CB ql_Psi2", 1, 0.1, 5, "");
    RooRealVar qr_Psi2("qr_Psi2", "Scale of sigma of right CB qr_Psi2", 1, 0.1, 5, "");

    RooRealVar nL("nL", "nL CB", 15, 3,15, "");
    RooRealVar alphaL("alphaR", "Alpha right CB", 5, 2, 5, "");
    RooRealVar nR("nR", "nR CB", 15, 3,15, "");
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


    //Choice to make: Gassian or Voigtian? 

    //Gaussian

    RooGaussian Gaussian_Phi("Gaussian_Phi", "Gaussian_Phi", Mm_mass_Phi, mu_Phi, sigma_V_Phi);
    RooGaussian Gaussian_Jpsi("Gaussian_Jpsi", "Gaussian_Jpsi", Mm_mass_Jpsi, mu_Jpsi, sigma_V_Jpsi);
    RooGaussian Gaussian_Psi2("Gaussian_Psi2", "Gaussian_Psi2", Mm_mass_Psi2, mu_Psi2, sigma_V_Psi2);

    RooAddPdf sigModel_Phi("sigModel_Phi", "Phi mass model", RooArgList(Gaussian_Phi, CB_Phi), GaussFraction);
    RooAddPdf sigModel_Jpsi("sigModel_Jpsi", "J/psi mass model", RooArgList(Gaussian_Jpsi, CB_Jpsi), GaussFraction);
    RooAddPdf sigModel_Psi2("sigModel_Psi2", "psi2 mass model", RooArgList(Gaussian_Psi2, CB_Psi2), GaussFraction);

    //Voigtian

    // RooRealVar reso_l("reso_l", "Width of BW", 0.001, 0.001, 0.1, "");
    // reso_l.setConstant(kTRUE);

    // RooFormulaVar l_Phi("l_Phi", "reso_l*mu_Phi*q_Phi", RooArgList(reso_l, mu_Phi, q_Phi));
    // RooFormulaVar l_Jpsi("l_Jpsi", "reso_l*mu_Jpsi", RooArgList(reso_l, mu_Jpsi));
    // RooFormulaVar l_Psi2("l_Psi2", "reso_l*mu_Psi2*q_Psi2", RooArgList(reso_l, mu_Psi2, q_Psi2));
    // RooVoigtian Voigtian_Phi("Voigtian_Phi", "Voigtian_Phi", Mm_mass_Phi, mu_Phi, sigma_V_Phi,l_Phi);
    // RooVoigtian Voigtian_Jpsi("Voigtian_Jpsi", "Voigtian_Jpsi", Mm_mass_Jpsi, mu_Jpsi, sigma_V_Jpsi,l_Jpsi);
    // RooVoigtian Voigtian_Psi2("Voigtian_Psi2", "Voigtian_Psi2", Mm_mass_Psi2, mu_Psi2, sigma_V_Psi2,l_Psi2);

    // RooAddPdf sigModel_Phi("sigModel_Phi", "Phi mass model", RooArgList(Voigtian_Phi, CB_Phi), GaussFraction);
    // RooAddPdf sigModel_Jpsi("sigModel_Jpsi", "J/psi mass model", RooArgList(Voigtian_Jpsi, CB_Jpsi), GaussFraction);
    // RooAddPdf sigModel_Psi2("sigModel_Psi2", "psi2 mass model", RooArgList(Voigtian_Psi2, CB_Psi2), GaussFraction);
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

    // SIMULTANEOUS FIT
    RooCategory sample("sample","sample") ;
    sample.defineType("Phi") ;
    sample.defineType("Jpsi") ;
    sample.defineType("Psi2") ;

    // Mm_mass_Phi.setBins(dh_Phi->numEntries());   
    // Mm_mass_Jpsi.setBins(dh_Jpsi->numEntries());   
    // Mm_mass_Psi2.setBins(dh_Psi2->numEntries());   
    // RooDataHist combData("combData","combined data",RooArgSet(Mm_mass_Phi,Mm_mass_Jpsi,Mm_mass_Psi2),Index(sample),Import("Phi",*dh_Phi),Import("Jpsi",*dh_Jpsi),Import("Psi2",*dh_Psi2));


    // mean.setConstant(kTRUE)


    RooDataSet combData("combData","combined data",RooArgSet(Mm_mass_Phi,Mm_mass_Jpsi,Mm_mass_Psi2),Index(sample),Import("Phi",*Data_Phi),Import("Jpsi",*Data_Jpsi),Import("Psi2",*Data_Psi2));
    RooSimultaneous simPdf("simPdf","simultaneous pdf",sample) ;

    simPdf.addPdf(*model_Phi,"Phi");
    simPdf.addPdf(*model_Jpsi,"Jpsi");
    simPdf.addPdf(*model_Psi2,"Psi2");

    //Store logs
    string logpath =  string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/models/")+string("Multi_fit_output_GdCB.log");
    ofstream logFile((logpath).c_str());
    
    RooFitResult* fitres = new RooFitResult;
    fitres = simPdf.fitTo(combData, RooFit::Save(), RooFit::Strategy(1)); //, RooFit::Minos());
    fitres->printMultiline(logFile,0,true);

    // RooRealVar* yield_ALL = (RooRealVar*) fitres->floatParsFinal().find("n_signal_total");
    // RooRealVar* yield_PASS = (RooRealVar*) fitres->floatParsFinal().find("n_signal_total_pass");

    plot("Phi", Mm_mass_Phi, Data_Phi, model_Phi, n_signal_Phi, n_bkg_Phi, logFile);
    plot("Jpsi", Mm_mass_Jpsi, Data_Jpsi, model_Jpsi, n_signal_Jpsi, n_bkg_Jpsi, logFile);
    plot("Psi2", Mm_mass_Psi2, Data_Psi2, model_Psi2, n_signal_Psi2, n_bkg_Psi2, logFile);

    logFile.close();
    // DELETING ALLOCATED MEMORY
    // delete File_Phi;
    // delete File_Jpsi;
    // delete File_Psi2;
    // //
    // delete DataTree_Phi;
    // delete DataTree_Jpsi;
    // delete DataTree_Psi2;
    // //
    // delete Data_Phi;
    // delete Data_Jpsi;
    // delete Data_Psi2;
    // //
    // // delete dh_Phi;
    // // delete dh_Jpsi;
    // // delete dh_Psi2;
    // //
    // delete reduce;
    // //
    // delete model_Phi;
    // delete model_Jpsi;
    // delete model_Psi2;
    // //
    // delete fitres;

    return;
}

void plot(const char* name, RooRealVar Mm_mass, RooDataSet* Data, RooAbsPdf* model, RooRealVar n_signal, RooRealVar n_back, ofstream& logFile, bool save=true){
    TCanvas* canvas  = new TCanvas(name,name,800,800);
    setTDRStyle();

    canvas->Divide(1,2);
    canvas->cd(1);

    RooPlot *frame = Mm_mass.frame(RooFit::Title("Invariant Mass"));

    frame->SetTitle(name);
    frame->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");

    Data->plotOn(frame,XErrorSize(1), Name("data"), MarkerSize(1.), DrawOption("PZ"));
    model->plotOn(frame,Name("Full model"));
    model->plotOn(frame,Components(("sigModel_"+string(name)).c_str()),Name("Signal model"),DrawOption("F"),FillColor(3), FillStyle(3001), LineColor(0));
    model->plotOn(frame,Components(("bkgModel_"+string(name)).c_str()),Name("Background model"),LineStyle(7), LineColor(2));

    gPad->SetTopMargin(0.08);
    gPad->SetLeftMargin(0.15);
    gPad->SetBottomMargin(0.02);
    gPad->SetPad(0.01,0.3,0.99,0.99);
    frame->GetYaxis()->SetLabelSize(0.05);
    frame->GetYaxis()->SetTitleSize(0.058);
    frame->GetYaxis()->SetTitleOffset(1.1);
    frame->GetXaxis()->SetLabelSize(0.);
    frame->GetXaxis()->SetTitleSize(0.);

    TLegend leg(0.75, 0.6, 0.85,0.85);
    leg.AddEntry(frame->findObject("data"), "Data", "lep");
    leg.AddEntry(frame->findObject("Full model"), "Full model", "l");
    leg.AddEntry(frame->findObject("Signal model"), "Signal model", "f");
    leg.AddEntry(frame->findObject("Background model"), "Bkg. model", "l");
    leg.SetBorderSize(0);
    leg.SetFillStyle(0);
    leg.SetTextSize(0.04);
    leg.SetTextFont(42);

    //compute chisquare
    RooAbsCollection *flparams = model->getParameters(Data)->selectByAttrib("Constant", kFALSE);
    Int_t nflparams = flparams->getSize();
    Double_t chisquare = -1;
    chisquare = frame->chiSquare("Full model", "data", nflparams);

    TPaveText label_2(0.25, 0.63, 0.34, 0.82, "NDC");
    label_2.SetBorderSize(0);
    label_2.SetFillColor(0);
    label_2.SetTextSize(0.041);
    label_2.SetTextFont(42);
    gStyle->SetStripDecimals(kTRUE);
    label_2.SetTextAlign(11);
    TString sYield = to_string(int(round(n_signal.getValV())));
    TString bYield = to_string(int(round(n_back.getValV())));
    TString csquare = to_string(chisquare);
    label_2.AddText("N_{sig} = " + sYield);
    label_2.AddText("N_{bkg} = " + bYield);
    label_2.AddText("#chi^{2} = " + csquare);
    logFile << "\nchisquare=" << chisquare<<"\n";

    frame->Draw();
    label_2.Draw();     
    leg.Draw();

    CMS(canvas);

    canvas->cd(2);
    RooHist *hpull = frame->pullHist("data", "Full model"); //massModel->GetName());
    RooPlot *frame_pulls = Mm_mass.frame(Title("Pull"));
    TLine* line = new TLine(Mm_mass.getMin(), 0, Mm_mass.getMax(), 0);
    line->SetLineColor(kBlue);
    frame_pulls->addObject(line);
    frame_pulls->addPlotable(hpull, "P"); //,"E3")
    frame_pulls->SetMarkerStyle(2);
    frame_pulls->SetMarkerSize(0.01);

    gPad->SetLeftMargin(0.15);
    gPad->SetPad(0.01, 0.01, 0.99, 0.3);
    gPad->SetTopMargin(0.01);
    gPad->SetBottomMargin(0.5); 
    frame_pulls->GetYaxis()->SetNdivisions(202);
    frame_pulls->GetYaxis()->SetRangeUser(-4, 4);
    frame_pulls->GetXaxis()->SetTitle("m_{#mu#mu} [GeV]");
    frame_pulls->GetYaxis()->SetTitle("Pulls");
    frame_pulls->GetXaxis()->SetTitleSize(0.13);
    frame_pulls->GetYaxis()->SetTitleSize(0.1);
    frame_pulls->GetXaxis()->SetLabelSize(0.1);
    frame_pulls->GetYaxis()->SetLabelSize(0.1);
    frame_pulls->GetXaxis()->SetLabelOffset(0.01);
    frame_pulls->GetYaxis()->SetLabelOffset(0.01);
    frame_pulls->GetXaxis()->SetTitleOffset(1.15);
    frame_pulls->GetYaxis()->SetTitleOffset(0.3);
    frame_pulls->GetXaxis()->SetTickLength(0.1);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    frame_pulls->Draw();

    if(save)
    {
        canvas->SaveAs(("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/multi_fit_"+string(name)+"_G_dG_.png").c_str());
        // canvas->SaveAs(("/data/submit/mori25/dark_photons_ludo/DimuonTrees/fits/figures/multi_fit_"+string(name)+"_V_dG.C").c_str());
    }
    // delete canvas;
    // delete frame;
    // delete frame_pulls;
    // delete hpull;
    // delete line;
    // delete flparams;
}