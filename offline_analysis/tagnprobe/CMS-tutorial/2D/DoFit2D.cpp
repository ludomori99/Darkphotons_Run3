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
    RooRealVar sigma("sigma", "Width of Gaussian", 0.06, 0.01, 0.1, "GeV");
    RooRealVar l("l", "Width of BW", 0.01, 0.01, 0.1, "GeV");
    RooVoigtian Voigtian("Voigtian", "Voigtian", Mm_mass, mu, sigma,l);
    
    // l.setConstant(kTRUE);

    RooRealVar sigmaL("sigmaL", "Width of left CB", 0.02, 0.01, 0.08, "GeV");
    RooRealVar sigmaR("sigmaR", "Width of right CB", 0.02, 0.01, 0.08, "GeV");
    RooRealVar nL("nL", "nL CB", 2, 2,15, "");
    RooRealVar alphaL("alphaR", "Alpha right CB", 5, 1, 5, "");
    RooRealVar nR("nR", "nR CB", 2, 2,15, "");
    RooRealVar alphaR("alphaL", "Alpha left CB", 5, 1, 5, "");
    RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigmaL, sigmaR, alphaL,nL,alphaR,nR);

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
	
    // RooDataHist* dh_ALL     = Data_ALL->binnedClone();
    // RooDataHist* dh_PASSING = Data_PASSING->binnedClone();
    
    RooDataHist combData("combData","combined data",Mm_mass,Index(sample),Import("ALL",*dh_ALL),Import("PASSING",*dh_PASSING));

    RooSimultaneous simPdf("simPdf","simultaneous pdf",sample) ;

    simPdf.addPdf(*model,"ALL");
    simPdf.addPdf(*model_pass,"PASSING");

    //Store logs
    string logpath =  "/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits/" + MuonID_str + "/" + quantx+"_"+quanty + "/" + condition + "_output.log";
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

    string path = "/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits/" + MuonID_str + "/" + quantx+"_"+quanty + "/" + condition;

    plot("", Mm_mass, Data_ALL, model, n_signal_total, n_back, logFile, path+"_ALL.png");
    plot("", Mm_mass, Data_PASSING, model, n_signal_total_pass, n_back_pass, logFile, path+"_PASS.png");

    // TCanvas* c_all  = new TCanvas("ALL","ALL",800,800);
    // TCanvas* c_pass = new TCanvas("PASS","PASS",800,800);
    
    // setTDRStyle();
    // c_all->Divide(1,2);
    // c_all->cd(1);

    // RooPlot *frame = Mm_mass.frame(RooFit::Title("Invariant Mass"));

    // frame->SetTitle((MuonID_str + string("; ALL")).c_str());
    // frame->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");

    // Data_ALL->plotOn(frame,XErrorSize(1), Name("data"), MarkerSize(1.), DrawOption("PZ"));
    // model->plotOn(frame,Name("Full model"));
    // model->plotOn(frame,Components("sigModel"),Name("Signal model"),DrawOption("F"),FillColor(3), FillStyle(3001), LineColor(0));
    // model->plotOn(frame,Components("bkgModel"),Name("Background model"),LineStyle(7), LineColor(2));

    // gPad->SetLeftMargin(0.15);
    // gPad->SetTopMargin(0.075);
    // gPad->SetBottomMargin(0.02);
    // gPad->SetPad(0.01,0.28,0.99,0.99);
    // frame->GetYaxis()->SetLabelSize(0.05);
    // frame->GetYaxis()->SetTitleSize(0.058);
    // frame->GetYaxis()->SetTitleOffset(1.5);
    // frame->GetXaxis()->SetLabelSize(0.);
    // frame->GetXaxis()->SetTitleSize(0.);

    // TLegend leg(0.7, 0.6, 0.9,0.85);
    // leg.AddEntry(frame->findObject("data"), "Data", "lep");
    // leg.AddEntry(frame->findObject("Full model"), "Full model", "l");
    // leg.AddEntry(frame->findObject("Signal model"), "Signal model", "f");
    // leg.AddEntry(frame->findObject("Background model"), "Bkg. model", "l");
    // leg.SetBorderSize(0);
    // leg.SetFillStyle(0);
    // leg.SetTextSize(0.04);
    // leg.SetTextFont(42);

    // //compute chisquare
    // RooAbsCollection *flparams = model->getParameters(Data_ALL)->selectByAttrib("Constant", kFALSE);
    // Int_t nflparams = flparams->getSize();
    // Double_t chisquare = -1;
    // chisquare = frame->chiSquare("Full model", "data", nflparams);

    // TPaveText label_2(0.25, 0.63, 0.34, 0.85, "NDC");
    // label_2.SetBorderSize(0);
    // label_2.SetFillColor(0);
    // label_2.SetTextSize(0.041);
    // label_2.SetTextFont(42);
    // gStyle->SetStripDecimals(kTRUE);
    // label_2.SetTextAlign(11);
    // TString sYield = to_string(int(round(n_signal_total.getValV())));
    // TString bYield = to_string(int(round(n_back.getValV())));
    // TString csquare =  to_string(0.01*float(int(round(100*chisquare)))).substr(0,4);
    // // TString csquare = format("{:.2f}", chisquare);

    // // stringstream stream;
    // // stream << fixed << setprecision(2) << chisquare;
    // // TString csquare = stream.str();
    
    // label_2.AddText(MuonID_str.c_str());
    // label_2.AddText("N_{sig} = " + sYield);
    // label_2.AddText("N_{bkg} = " + bYield);
    // label_2.AddText("#chi^{2}_{red} = " + csquare);
    // logFile << "\nchisquare=" << chisquare<<"\n";

    // frame->Draw();
    // label_2.Draw();     
    // leg.Draw();

    // CMS(c_all);

    // c_all->cd(2);
    // RooHist *hpull = frame->pullHist("data", "Full model"); //massModel->GetName());
    // RooPlot *frame_pulls = Mm_mass.frame(Title("Pull"));
    // TLine* line = new TLine(Mm_mass.getMin(), 0, Mm_mass.getMax(), 0);
    // line->SetLineColor(kBlue);
    // frame_pulls->addObject(line);
    // frame_pulls->addPlotable(hpull, "P"); //,"E3")
    // frame_pulls->SetMarkerStyle(2);
    // frame_pulls->SetMarkerSize(0.01);

    // gPad->SetLeftMargin(0.15);
    // gPad->SetTopMargin(0.05);
    // gPad->SetBottomMargin(0.4); 
    // gPad->SetPad(0.01, 0.01, 0.99, 0.25);
    // frame_pulls->GetYaxis()->SetNdivisions(202);
    // frame_pulls->GetYaxis()->SetRangeUser(-4, 4);
    // frame_pulls->GetXaxis()->SetTitle("m_{#mu#mu} [GeV]");
    // frame_pulls->GetYaxis()->SetTitle("Pulls");
    // frame_pulls->GetXaxis()->SetTitleSize(0.16);
    // frame_pulls->GetYaxis()->SetTitleSize(0.16);
    // frame_pulls->GetXaxis()->SetLabelSize(0.14);
    // frame_pulls->GetYaxis()->SetLabelSize(0.14);
    // frame_pulls->GetXaxis()->SetLabelOffset(0.01);
    // frame_pulls->GetYaxis()->SetLabelOffset(0.01);
    // frame_pulls->GetXaxis()->SetTitleOffset(1.1);
    // frame_pulls->GetYaxis()->SetTitleOffset(0.25);
    // frame_pulls->GetXaxis()->SetTickLength(0.1);
    // gPad->SetFrameFillColor(0);
    // gPad->SetFrameBorderMode(0);
    // gPad->SetFrameFillColor(0);
    // gPad->SetFrameBorderMode(0);
    // frame_pulls->Draw();



    // RooPlot *frame_pass = Mm_mass.frame(RooFit::Title("Invariant Mass"));
    // c_pass->Divide(1,2);
    // c_pass->cd(1);

    // frame_pass->SetTitle((MuonID_str + string("; PASSING")).c_str());
    // frame_pass->SetXTitle("#mu^{+}#mu^{-} invariant mass [GeV/c^{2}]");    

    // Data_PASSING->plotOn(frame_pass, Name("data"), MarkerSize(1.), DrawOption("PZ"));
    // model_pass->plotOn(frame_pass,Name("Full model"));
    // model_pass->plotOn(frame_pass,Components("sigModel"),Name("Signal model"),DrawOption("F"),FillColor(3), FillStyle(3001), LineColor(0));
    // model_pass->plotOn(frame_pass,Components("bkgModel"),Name("Background model"),LineStyle(7), LineColor(2));
    
    // gPad->SetLeftMargin(0.15);
    // gPad->SetTopMargin(0.075);
    // gPad->SetBottomMargin(0.02);
    // gPad->SetPad(0.01,0.28,0.99,0.99);
    // frame_pass->GetYaxis()->SetLabelSize(0.05);
    // frame_pass->GetYaxis()->SetTitleSize(0.05);
    // frame_pass->GetYaxis()->SetTitleOffset(1.5);
    // frame_pass->GetXaxis()->SetLabelSize(0.);
    // frame_pass->GetXaxis()->SetTitleSize(0.);

    // TLegend leg_pass(0.7, 0.6, 0.9,0.85);
    // leg_pass.AddEntry(frame_pass->findObject("data"), "Data", "lep");
    // leg_pass.AddEntry(frame_pass->findObject("Full model"), "Full model", "l");
    // leg_pass.AddEntry(frame_pass->findObject("Signal model"), "Signal model", "f");
    // leg_pass.AddEntry(frame_pass->findObject("Background model"), "Bkg. model", "l");
    // leg_pass.SetBorderSize(0);
    // leg_pass.SetFillStyle(0);
    // leg_pass.SetTextSize(0.04);
    // leg_pass.SetTextFont(42);

    // // compute chisquare
    // RooAbsCollection *flparams_pass = model->getParameters(Data_PASSING)->selectByAttrib("Constant", kFALSE);
    // Int_t nflparams_pass = flparams_pass->getSize();
    // Double_t chisquare_pass = -1;
    // chisquare_pass = frame_pass->chiSquare("Full model", "data", nflparams_pass);

    // TPaveText label_pass(0.25, 0.63, 0.34, 0.85, "NDC");
    // label_pass.SetBorderSize(0);
    // label_pass.SetFillColor(0);
    // label_pass.SetTextSize(0.041);
    // label_pass.SetTextFont(42);
    // gStyle->SetStripDecimals(kTRUE);
    // label_pass.SetTextAlign(11);
    // TString sYield_pass = to_string(int(round(n_signal_total_pass.getValV())));
    // TString bYield_pass = to_string(int(round(n_back_pass.getValV())));
    // TString csquare_pass =  to_string(0.01*float(int(round(100*chisquare_pass)))).substr(0,4);
    // // TString csquare_pass = format("{:.2f}", chisquare_pass);

    // // stringstream s1;
    // // s1 << fixed << setprecision(2) << chisquare_pass;
    // // TString csquare_pass = s1.str();

    // label_pass.AddText(MuonID_str.c_str());
    // label_pass.AddText("N_{sig} = " + sYield_pass);
    // label_pass.AddText("N_{bkg} = " + bYield_pass);
    // label_pass.AddText("#chi^{2}_{red} = " + csquare_pass);

    // frame_pass->Draw();
    // leg_pass.Draw();
    // label_pass.Draw();     

    // CMS(c_pass);


    // c_pass->cd(2);
    // RooHist *hpull_pass = frame_pass->pullHist("data", "Full model"); //massModel->GetName());
    // RooPlot *frame_pulls_pass = Mm_mass.frame(Title("Pull"));
    // TLine* line1 = new TLine(Mm_mass.getMin(), 0, Mm_mass.getMax(), 0);
    // line1->SetLineColor(kBlue);
    // frame_pulls_pass->addObject(line1);
    // frame_pulls_pass->addPlotable(hpull_pass, "P"); //,"E3")
    // frame_pulls_pass->SetMarkerStyle(2);
    // frame_pulls_pass->SetMarkerSize(0.01);

    // gPad->SetLeftMargin(0.15);
    // gPad->SetTopMargin(0.05);
    // gPad->SetBottomMargin(0.4); 
    // gPad->SetPad(0.01, 0.01, 0.99, 0.25);
    // frame_pulls_pass->GetYaxis()->SetNdivisions(202);
    // frame_pulls_pass->GetYaxis()->SetRangeUser(-4, 4);
    // frame_pulls_pass->GetXaxis()->SetTitle("m_{#mu#mu} [GeV]");
    // frame_pulls_pass->GetYaxis()->SetTitle("Pulls");
    // frame_pulls_pass->GetXaxis()->SetTitleSize(0.16);
    // frame_pulls_pass->GetYaxis()->SetTitleSize(0.16);
    // frame_pulls_pass->GetXaxis()->SetLabelSize(0.14);
    // frame_pulls_pass->GetYaxis()->SetLabelSize(0.14);
    // frame_pulls_pass->GetXaxis()->SetLabelOffset(0.01);
    // frame_pulls_pass->GetYaxis()->SetLabelOffset(0.01);
    // frame_pulls_pass->GetXaxis()->SetTitleOffset(1.2);
    // frame_pulls_pass->GetYaxis()->SetTitleOffset(0.3);
    // frame_pulls_pass->GetXaxis()->SetTickLength(0.1);
    // gPad->SetFrameFillColor(0);
    // gPad->SetFrameBorderMode(0);
    // gPad->SetFrameFillColor(0);
    // gPad->SetFrameBorderMode(0);
    // frame_pulls_pass->Draw();

    // c_pass->SaveAs(("/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits/" + MuonID_str + "/" + quantx+"_"+quanty + "/" + condition + "_PASS.png").c_str());
    // c_all->SaveAs(("/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits/"  + MuonID_str + "/" + quantx+"_"+quanty + "/" + condition + "_ALL.png").c_str());
    

    // logFile.close();

    // // DELETING ALLOCATED MEMORY
    // //
    delete Data_ALL;
    delete Data_PASSING;
    //
    delete dh_ALL;
    delete dh_PASSING;
    //
    delete cutvar;
    delete reduce;
    // //
    // delete c_all;
    // delete c_pass;
    // //
    // delete model;
    // delete model_pass;
    // delete fitres;

    return output;
}

