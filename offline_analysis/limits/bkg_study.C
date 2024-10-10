void bkg_study(float m = 6){

   // Load the combine Library
   gSystem->Load("libHiggsAnalysisCombinedLimit.so");


	float bias (float m, int func){
   
		// mass variable
		RooRealVar mass("mass","mass",m-(m*0.013*5.),m+(m*0.013*5.));

		//Bernstein
		RooRealVar par1("par1", "par1", 0.2, 0, 10);
		RooRealVar par2("par2", "par2", 1.5, 0, 10);
		RooRealVar par3("par3", "par3", 2.0, 0, 10);
		RooRealVar par4("par4", "par4", 2.0, 0, 10);
		RooArgList alist(par1, par2, par3, par4);
		RooBernstein bkg_model_bern4("bkg_model_bern4", "bkg_model_bern4", mass, alist);
		
		// //Polynomial times exp
		RooRealVar lar1("lar1", "lar1", 0.001, -10.0, 10.0);
		RooRealVar lar2("lar2", "lar2", 0.0, -10.0, 10.0);
		RooRealVar lar3("lar3", "lar3", 0.0, -10.0, 10.0);
		RooRealVar lar4("lar4", "lar4", 0.0, -10.0, 10.0);
		RooArgList llist(lar1, lar2, lar3, lar4);
		RooPolynomial bkg_model_line4("bkg_model_line", "bkg_model_line", mass, llist, 1);
		//Exponentials
		RooRealVar car1("car1", "car1", -0.5, -10, 10);
		RooExponential bkg_model_exp4("bkg_model_exp4", "bkg_model_exp4", mass, car1);
		//Product of the two
		RooProdPdf bkg_model_pol4xexp("bkg_model_pol4xexp", "bkg_model_pol4xexp", bkg_model_line4, bkg_model_exp4);

		//Sum of exp
		RooRealVar bar1("bar1", "bar1", -0.5, -10, 10);                            
		RooRealVar bf1("bf1","bf1",0.2,0.0,1.0);   
		RooExponential exp1("exp1", "exp1", mass, bar1);
		RooRealVar bar2("bar2", "bar2", -0.5, -10, 10);                                                                                           
		RooRealVar bf2("bf2","bf2",0.2,0.0,1.0);                                                                                                  
		RooExponential exp2("exp2", "exp2", mass, bar2);
		RooRealVar bar3("bar3", "bar3", -0.5, -10, 10);
		RooRealVar bf3("bf3","bf3",0.2,0.0,1.0);
		RooExponential exp3("exp3", "exp3", mass, bar3);
		RooRealVar bar4("bar4", "bar4", -0.5, -10, 10);
		RooRealVar bf4("bf4","bf4",0.2,0.0,1.0);
		RooExponential exp4("exp4", "exp4", mass, bar4);
		RooRealVar bar5("bar5", "bar5", -0.5, -10, 10);
		RooRealVar bf5("bf5","bf5",0.2,0.0,1.0);
		RooExponential exp5("exp5", "exp5", mass, bar5);
		RooRealVar bar6("bar6", "bar6", -0.5, -10, 10);
		RooRealVar bf6("bf6","bf6",0.2,0.0,1.0);
		RooExponential exp6("exp6", "exp6", mass, bar6);
		RooRealVar bar7("bar7", "bar7", -0.5, -10, 10);
		RooExponential exp7("exp7", "exp7", mass, bar7);
		RooArgList explist(exp1,exp2,exp3,exp4,exp5,exp6);
		RooArgList expclist(bf1,bf2,bf3,bf4,bf5);
		RooAddPdf bkg_model_exp7("bkg_model_exp7","bkg_model_exp7",explist,expclist,true);

		//powerlaw + bernstein
		RooRealVar pow_1("pow_1","exponent of power law",0,-10,10);
		RooRealVar pf1("pf1","frac of power law",0.2,0.0,1.0);
		RooGenericPdf plaw1("plaw1","TMath::Power(@0,@1)",RooArgList(mass,pow_1));
		RooRealVar qar1("qar1", "qar1", 0.2, 0, 10);
		RooRealVar qar2("qar2", "qar2", 1.5, 0, 10);
		RooRealVar qar3("qar3", "qar3", 2.0, 0, 10);
		RooArgList qlist(qar1, qar2, qar3);
		RooRealVar bfp1("bfp1","frac of bernstein",0.2,0.0,1.0);
		RooBernstein bern3("bkg_model_bern3", "bkg_model_bern3", mass, qlist);
		RooArgList plawlist1(plaw1, bern3);
		RooArgList plawclist1(pf1, bfp1);
		RooAddPdf bkg_model_bern3p1("bkg_model_bern3p1","bkg_model_bern3p1",plawlist1,plawclist1,false);


		// Generate some data (lets use the power lay function for it)
		// Here we are using unbinned data, but binning the data is also fine
		RooDataSet *data = bkg_model_bern3p1.generate(mass,RooFit::NumEvents(1000));

		// First we fit the pdfs to the data (gives us a sensible starting value of parameters for, e.g - blind limits)

		bkg_model_bern4.fitTo(*data,RooFit::Verbose(false));//,RooMinimizer::PrintLevel(-1));
		bkg_model_pol4xexp.fitTo(*data,RooFit::Verbose(false));//,RooMinimizer::PrintLevel(-1));
		bkg_model_exp7.fitTo(*data,RooFit::Verbose(false));//,RooMinimizer::PrintLevel(-1));
		bkg_model_bern3p1.fitTo(*data,RooFit::Verbose(false));//,RooMinimizer::PrintLevel(-1));


		// Make a plot (data is a toy dataset)
		TCanvas t ("canvas", "plot", 800, 600);
		RooPlot *plot = mass.frame();   data->plotOn(plot);
		bkg_model_bern4.plotOn(plot,RooFit::LineColor(kGreen));
		bkg_model_pol4xexp.plotOn(plot,RooFit::LineColor(kBlue));
		bkg_model_exp7.plotOn(plot,RooFit::LineColor(kBlue));
		bkg_model_bern3p1.plotOn(plot,RooFit::LineColor(kRed));
		plot->SetTitle("PDF fits to toy data");
		plot->Draw();
		t.Update();
		t.SaveAs("ber3p1.png");

		RooCategory  pdf_index("pdf_index","Index of the background PDF which is active");
		RooArgList bkg_pdf_list;		
		bkg_pdf_list.add(bkg_model_bern4);
		bkg_pdf_list.add(bkg_model_pol4xexp);
		bkg_pdf_list.add(bkg_model_exp7);
		bkg_pdf_list.add(bkg_model_bern3p1);

		RooMultiPdf bkg_model("bkg_model", "All Pdfs", pdf_index, bkg_pdf_list);	       
		bkg_model.setCorrectionFactor(0.5); 

		RooRealVar norm("roomultipdf_norm","Number of background events",1000,0,10000);
			
		RooRealVar mu("mu", "mu", m);
		RooRealVar reso( "reso", "reso", 0.007814);
		RooRealVar reso_l( "reso_l", "reso_l", 0.0051107);
		RooRealVar alphaL( "alphaL", "alphaL", 1.2906);
		RooRealVar nL( "nL", "nL", 2.8840);
		RooRealVar alphaR( "alphaR", "alphaR", 2.0);
		RooRealVar nR( "nR", "nR", 10);
		RooFormulaVar sigma("sigma", "sigma", "mu*reso", RooArgList(mu,reso));
		RooFormulaVar l("l", "l", "mu*reso_l", RooArgList(mu,reso_l));
		RooVoigtian Voigtian("Voigtian", "Voigtian", mass, mu, l,sigma);
		RooCrystalBall CB("CB", "CB", mass, mu, sigma, sigma, alphaL,nL,alphaR,nR);
		RooRealVar VoigtFraction("VoigtFraction", "Fraction of Voigtian", 0.50539);
		RooAddPdf signalModel("signalModel", "mass model", RooArgList(Voigtian, CB), VoigtFraction);


		// Save to a new workspace
		TFile *fout = new TFile((string("/work/submit/mori25/Darkphotons_ludo/offline_analysis/limits/bkg_workspaces/") + string(m) + string("_") + string(func) + string(".root")).c_str(),"RECREATE");
		RooWorkspace wout("workspace","workspace");

		data->SetName("data");
		wout.import(*data);
		wout.import(pdf_index);
		wout.import(norm);
		wout.import(bkg_model);
		wout.import(signalModel);
		wout.Print();
		wout.Write();

		

		return 
	}


	ofstream effs_file("/work/submit/mori25/Darkphotons_ludo/offline_analysis/limits/bias.csv");
	effs_file << "mass" << "," << "index" << "\n";	

	float m_tmp=2;
	while(m_tmp<8){
		if (m_tmp ==3 || m_tmp == 4) continue;
		for (int i = 0; i<3;i++){
			effs_file << masses[i] << "," << effs[i] << "\n";
		}
	}

		m_tmp ++;
		
	}
	effs_file.close()


}