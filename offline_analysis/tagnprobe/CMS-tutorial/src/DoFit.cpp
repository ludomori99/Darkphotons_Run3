#include "tdrstyle.C"
using namespace RooFit;

double* doFit(string condition, string MuonID_str, string quant, double* init_conditions, bool isBarrel, bool isEndcap, bool save = true) // RETURNS ARRAY WITH [yield_all, yield_pass, err_all, err_pass]    ->   OUTPUT ARRAY
{
    TFile *file0    = TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Jpsi/TP_samples_Jpsi.root");
    TTree *DataTree = (TTree*)file0->Get(("tree"));

    double lowRange = 2.6;  double highRange = 3.56;

    RooRealVar MuonID(MuonID_str.c_str(), MuonID_str.c_str(), 0, 1); //Muon_Id
    RooRealVar isBarrelMuon("isBarrelMuon","isBarrelMuon",0,1);
    RooRealVar Mm_mass("Mm_mass", "Mm_mass", lowRange, highRange);

    double* limits = new double[2];
    if (quant == "Probe_pt") {
        limits[0] = 0;
        limits[1] = 40;
    }
    if (quant == "Probe_abs_eta") {
        limits[0] = 0;
        limits[1] = 3;
    }
    if (quant == "Probe_eta") {
        limits[0] = -3;
        limits[1] = 3;
    }   
    if (quant == "Mm_dR") {
        limits[0] = 0;
        limits[1] = 2;
    }

    RooRealVar quantity(quant.c_str(), quant.c_str(), limits[0], limits[1]);


    RooFormulaVar* reduce = new RooFormulaVar("PPTM", condition.c_str(), ((isBarrel||isEndcap) ? RooArgList(isBarrelMuon,quantity) : RooArgList(quantity)));
    RooFormulaVar* cutvar = new RooFormulaVar("PPTM", (condition + "&&" + MuonID_str + "==1").c_str() , ((isBarrel||isEndcap) ? RooArgList(isBarrelMuon,quantity,MuonID) : RooArgList(quantity,MuonID)));

    RooDataSet *Data_ALL    = new RooDataSet("DATA", "DATA", DataTree, ((isBarrel||isEndcap) ? RooArgSet(isBarrelMuon,quantity,MuonID,Mm_mass) : RooArgSet(quantity,MuonID,Mm_mass)),*reduce);
    RooDataSet *Data_PASSING = new RooDataSet("DATA_PASS", "DATA_PASS", DataTree, ((isBarrel||isEndcap) ? RooArgSet(isBarrelMuon,quantity,MuonID,Mm_mass) : RooArgSet(quantity,MuonID,Mm_mass)), *cutvar);
   
    RooDataHist* dh_ALL     = Data_ALL->binnedClone();
    RooDataHist* dh_PASSING = Data_PASSING->binnedClone();


    // mass model for single resonance. Gauss + dCB. In this case employed for Jpsi
    RooRealVar mu("mu", "J/Psi Mass", 3.09809, lowRange, highRange);
    RooRealVar sigma("sigma", "Width of Gaussian", 0.09, 0.005, 0.1, "GeV");
    RooRealVar l("l", "Width of BW", 0.0378, 0.01, 10, "GeV");
    RooVoigtian Voigtian("Voigtian", "Voigtian", Mm_mass, mu, sigma,l);

    RooRealVar sigmaL("sigmaL", "Width of left CB", 0.05, 0.01, 1, "GeV");
    RooRealVar sigmaR("sigmaR", "Width of right CB", 0.5, 0.01, 1, "GeV");
    RooRealVar nL("nL", "nL CB", 0.8, 0.1,15, "");
    RooRealVar alphaL("alphaR", "Alpha right CB", 2.5, 0.1, 5, "");
    RooRealVar nR("nR", "nR CB", 0.3, 0.1,15, "");
    RooRealVar alphaR("alphaL", "Alpha left CB", 1.7, 0.1, 5, "");
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

    //Store logs
    ofstream logFile(("/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits/" + MuonID_str + "/" + quant + "/" + condition + "_output.log").c_str());
    streambuf* oldCoutStreamBuf = cout.rdbuf();
    cout.rdbuf(logFile.rdbuf());


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

    TCanvas* c_all  = new TCanvas("ALL","ALL",800,800);
    TCanvas* c_pass = new TCanvas("PASS","PASS",800,800);

    setTDRStyle();

    c_all->Divide(1,2);
    c_all->cd(1);
    RooPlot *frame = Mm_mass.frame(RooFit::Title("Invariant Mass"));

    frame->SetTitle((MuonID_str + string("; ALL")).c_str());
    frame->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");

    Data_ALL->plotOn(frame,XErrorSize(1), Name("data"), MarkerSize(1.), DrawOption("PZ"));
    model->plotOn(frame,Name("Full model"));
    model->plotOn(frame,Components("sigModel"),Name("Signal model"),DrawOption("F"),FillColor(3), FillStyle(3001), LineColor(0));
    model->plotOn(frame,Components("bkgModel"),Name("Background model"),LineStyle(7), LineColor(2));

    gPad->SetLeftMargin(0.15);
    gPad->SetBottomMargin(0.02);
    gPad->SetPad(0.01,0.3,0.99,0.99);
    frame->GetYaxis()->SetLabelSize(0.05);
    frame->GetYaxis()->SetTitleSize(0.05);
    frame->GetYaxis()->SetTitleOffset(1.4);
    frame->GetXaxis()->SetLabelSize(0.);
    frame->GetXaxis()->SetTitleSize(0.);

    TLegend leg(0.8, 0.6, 0.9,0.85);
    leg.AddEntry(frame->findObject("data"), "Data", "lep");
    leg.AddEntry(frame->findObject("Full model"), "Full model", "l");
    leg.AddEntry(frame->findObject("Signal model"), "Signal model", "f");
    leg.AddEntry(frame->findObject("Background model"), "Bkg model", "l");
    leg.SetBorderSize(0);
    leg.SetFillStyle(0);
    leg.SetTextSize(0.04);
    leg.SetTextFont(42);

    //compute chisquare
    RooAbsCollection *flparams = model->getParameters(Data_ALL)->selectByAttrib("Constant", kFALSE);
    Int_t nflparams = flparams->getSize();
    Double_t chisquare = -1;
    chisquare = frame->chiSquare("Full model", "data", nflparams);

    TPaveText label_2(0.25, 0.63, 0.34, 0.85, "NDC");
    label_2.SetBorderSize(0);
    label_2.SetFillColor(0);
    label_2.SetTextSize(0.041);
    label_2.SetTextFont(42);
    gStyle->SetStripDecimals(kTRUE);
    label_2.SetTextAlign(11);
    TString sYield = to_string(int(round(n_signal_total.getValV())));
    TString bYield = to_string(int(round(n_back.getValV())));
    TString csquare = to_string(chisquare);
    label_2.AddText(MuonID_str.c_str());
    label_2.AddText("N_{sig} = " + sYield);
    label_2.AddText("N_{bkg} = " + bYield);
    label_2.AddText("#chi^{2} = " + csquare);

    frame->Draw();
    label_2.Draw();     
    leg.Draw();


    c_all->cd(2);
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
    frame_pulls->GetXaxis()->SetTitleSize(0.1);
    frame_pulls->GetYaxis()->SetTitleSize(0.1);
    frame_pulls->GetXaxis()->SetLabelSize(0.1);
    frame_pulls->GetYaxis()->SetLabelSize(0.1);
    frame_pulls->GetXaxis()->SetLabelOffset(0.01);
    frame_pulls->GetYaxis()->SetLabelOffset(0.01);
    frame_pulls->GetXaxis()->SetTitleOffset(1.2);
    frame_pulls->GetYaxis()->SetTitleOffset(0.3);
    frame_pulls->GetXaxis()->SetTickLength(0.1);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    frame_pulls->Draw();



    RooPlot *frame_pass = Mm_mass.frame(RooFit::Title("Invariant Mass"));
    c_pass->Divide(1,2);
    c_pass->cd(1);

    frame_pass->SetTitle((MuonID_str + string("; PASSING")).c_str());
    frame_pass->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");    

    Data_PASSING->plotOn(frame_pass, Name("data"), MarkerSize(1.), DrawOption("PZ"));
    model_pass->plotOn(frame_pass,Name("Full model"));
    model_pass->plotOn(frame_pass,Components("sigModel"),Name("Signal model"),DrawOption("F"),FillColor(3), FillStyle(3001), LineColor(0));
    model_pass->plotOn(frame_pass,Components("bkgModel"),Name("Background model"),LineStyle(7), LineColor(2));
    
    gPad->SetLeftMargin(0.15);
    gPad->SetBottomMargin(0.02);
    gPad->SetPad(0.01,0.3,0.99,0.99);
    frame_pass->GetYaxis()->SetLabelSize(0.05);
    frame_pass->GetYaxis()->SetTitleSize(0.05);
    frame_pass->GetYaxis()->SetTitleOffset(1.4);
    frame_pass->GetXaxis()->SetLabelSize(0.);
    frame_pass->GetXaxis()->SetTitleSize(0.);

    TLegend leg_pass(0.8, 0.6, 0.9,0.85);
    leg_pass.AddEntry(frame_pass->findObject("data"), "Data", "lep");
    leg_pass.AddEntry(frame_pass->findObject("Full model"), "Full model", "l");
    leg_pass.AddEntry(frame_pass->findObject("Signal model"), "Signal model", "f");
    leg_pass.AddEntry(frame_pass->findObject("Background model"), "Bkg. model", "l");
    leg_pass.SetBorderSize(0);
    leg_pass.SetFillStyle(0);
    leg_pass.SetTextSize(0.04);
    leg_pass.SetTextFont(42);

    // compute chisquare
    RooAbsCollection *flparams_pass = model->getParameters(Data_PASSING)->selectByAttrib("Constant", kFALSE);
    Int_t nflparams_pass = flparams_pass->getSize();
    Double_t chisquare_pass = -1;
    chisquare_pass = frame_pass->chiSquare("Full model", "data", nflparams_pass);

    TPaveText label_pass(0.25, 0.63, 0.34, 0.85, "NDC");
    label_pass.SetBorderSize(0);
    label_pass.SetFillColor(0);
    label_pass.SetTextSize(0.041);
    label_pass.SetTextFont(42);
    gStyle->SetStripDecimals(kTRUE);
    label_pass.SetTextAlign(11);
    TString sYield_pass = to_string(int(round(n_signal_total_pass.getValV())));
    TString bYield_pass = to_string(int(round(n_back_pass.getValV())));
    TString csquare_pass = to_string(chisquare_pass);
    label_pass.AddText(MuonID_str.c_str());
    label_pass.AddText("N_{sig} = " + sYield_pass);
    label_pass.AddText("N_{bkg} = " + bYield_pass);
    label_pass.AddText("#chi^{2} = " + csquare_pass);

    frame_pass->Draw();
    leg_pass.Draw();
    label_pass.Draw();     

    c_pass->cd(2);
    RooHist *hpull_pass = frame_pass->pullHist("data", "Full model"); //massModel->GetName());
    RooPlot *frame_pulls_pass = Mm_mass.frame(Title("Pull"));
    TLine* line1 = new TLine(Mm_mass.getMin(), 0, Mm_mass.getMax(), 0);
    line1->SetLineColor(kBlue);
    frame_pulls_pass->addObject(line1);
    frame_pulls_pass->addPlotable(hpull_pass, "P"); //,"E3")
    frame_pulls_pass->SetMarkerStyle(2);
    frame_pulls_pass->SetMarkerSize(0.01);

    gPad->SetLeftMargin(0.15);
    gPad->SetPad(0.01, 0.01, 0.99, 0.3);
    gPad->SetTopMargin(0.01);
    gPad->SetBottomMargin(0.5); 
    frame_pulls_pass->GetYaxis()->SetNdivisions(202);
    frame_pulls_pass->GetYaxis()->SetRangeUser(-4, 4);
    frame_pulls_pass->GetXaxis()->SetTitle("m_{#mu#mu} [GeV]");
    frame_pulls_pass->GetYaxis()->SetTitle("Pulls");
    frame_pulls_pass->GetXaxis()->SetTitleSize(0.1);
    frame_pulls_pass->GetYaxis()->SetTitleSize(0.1);
    frame_pulls_pass->GetXaxis()->SetLabelSize(0.1);
    frame_pulls_pass->GetYaxis()->SetLabelSize(0.1);
    frame_pulls_pass->GetXaxis()->SetLabelOffset(0.01);
    frame_pulls_pass->GetYaxis()->SetLabelOffset(0.01);
    frame_pulls_pass->GetXaxis()->SetTitleOffset(1.2);
    frame_pulls_pass->GetYaxis()->SetTitleOffset(0.3);
    frame_pulls_pass->GetXaxis()->SetTickLength(0.1);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    gPad->SetFrameFillColor(0);
    gPad->SetFrameBorderMode(0);
    frame_pulls_pass->Draw();


    if(save)
    {
        c_pass->SaveAs(("/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits/" + MuonID_str + "/" + quant + "/" + condition + "_PASS.png").c_str());
        c_all->SaveAs(("/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits/"  + MuonID_str + "/" + quant + "/" + condition + "_ALL.png").c_str());
    }

    // Restore old cout stream buffer
    cout.rdbuf(oldCoutStreamBuf);
    logFile.close();

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
    delete reduce;
    //
    delete c_all;
    delete c_pass;
    //
    delete model;
    delete model_pass;
    delete fitres;

    return output;
}

