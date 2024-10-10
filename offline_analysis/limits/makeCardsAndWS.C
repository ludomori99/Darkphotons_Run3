#include <iostream>
#include <TLegend.h>
#include <sstream>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include "TH1D.h"
#include "TH2D.h"
#include <THStack.h>
#include "TProfile.h"
#include "TGraph.h"
#include "TFile.h"
#include "TTree.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TFractionFitter.h"
#include <string>
#include <vector>
#include <math.h>
#include <TLatex.h>
#include <TLine.h>
#include <TMarker.h>
#include <TPave.h>
#include <TPaveStats.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <TString.h>
#include "TGraphErrors.h"
#include "TF1.h"
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <RooPlot.h>
#include <RooArgSet.h>
#include <RooArgList.h>
#include <RooDataSet.h>
#include <RooDataHist.h>
#include <RooGaussian.h>
#include <RooPolynomial.h>
#include <RooRealVar.h> 
#include <RooFormulaVar.h> 
#include <RooWorkspace.h> 
#include <RooMsgService.h> 
#include "compute_eff.C"
#include "compute_mva_sf.C"
#include "HiggsAnalysis/CombinedLimit/interface/RooMultiPdf.h"

using namespace std;

void makeCardsAndWS(){
  

  TH1F* histChi2_A=new TH1F("histChi2_A","histChi2_A",50,0.5,1.5);

  //WHICH YEAR
  	TString year="2022-2023";
	TString suff="full";
  //INPUT FILE WITH HISTOGRAMS TO FIT BACKGROUND
  	TFile* file=TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/histograms/offline/output_histogram_full_1to8.root");
	
  //PREPARE EXPECTED NUMBER OF SIGNAL EVENTS PER CATEGORY

	//LUMINOSITY
	double luminosity = 62400;//pb-1

	int num_mass_regions = 199 ; //139 from m=2
    float growth_factor = 0.01;

	//WORKING POINTS
	double id_wp = 0.4;
	double mva_wp = 0.8;

	//EFFICIENCY
	string trigger = "HLT_DoubleMu4_3_LowMass";
	double trigger_sf = 0.943;
	double trigger_sf_unc = 1 + 0.057/0.943;

	double ID_sf = 0.9915;
	double ID_sf_unc = 1 + 0.0258/0.9915;
	
	//vertex mva efficiency needs to be interpolated 
	
	//scale ???
	double xSec = 1.;

	// double unfittable_regions[8][2] = { {0.97,1.12}, {2.8,3.85}};

	double* masses = new double[num_mass_regions];
	double* effs = new double[num_mass_regions];
	bool save_efficiency=false;


   //LOOP OVER MASS INDICES AND MAKE THE CARDS/WORKSPACES
	double mass;
	for(int i=0; i<num_mass_regions; i++){

		// if (i%2==0) continue;

	  	//get the histograms
	  	TH1* catC=(TH1*)file->Get(Form("massforLimit_CatC%d",i));
	  	double massLow  =  catC->GetXaxis()->GetXmin();
      	double massHigh =  catC->GetXaxis()->GetXmax();
		double* massLimits = new double[2];
		massLimits[0]=massLow;
		massLimits[1]=massHigh;

		mass = 0.5*(massLow+massHigh);
		masses[i] = mass;
		cout<<"mass: "<<mass<<endl;
		// cout<<"massLow: "<<massLow<<endl;
		// cout<<"massHigh: "<<massHigh<<endl;

		double global_eff_lmDY = compute_eff("/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/dump_post_BDT/*", trigger, mva_wp,id_wp,massLimits,true);
		effs[i] = global_eff_lmDY;
		double mva_sf = compute_mva_sf(mass);
		double mva_sf_err = 1 + 0.059/mva_sf;
		cout << mva_sf << endl;

		if ((mass >= 2.6) && (mass <= 4.16)) continue;

		RooRealVar Mm_mass  ("Mm_mass"  	, "Mm_mass " 	, massLow		, massHigh);

		// mass model for single resonance. Voigtian + dCB according to multifit. Can add floating nuisances in card below (uncomment vars and formuals )
		RooRealVar mu("mu", "mu", mass);
		RooRealVar reso( "reso", "reso", 0.007814);
		RooRealVar reso_l( "reso_l", "reso_l", 0.0051107);
		RooRealVar alphaL( "alphaL", "alphaL", 1.2906);
		RooRealVar nL( "nL", "nL", 2.8840);
		RooRealVar alphaR( "alphaR", "alphaR", 2.0);
		RooRealVar nR( "nR", "nR", 10);
		RooFormulaVar sigma("sigma", "sigma", "mu*reso", RooArgList(mu,reso));
		RooFormulaVar l("l", "l", "mu*reso_l", RooArgList(mu,reso_l));

		// RooRealVar sigma("sigma", "Width of Gaussian",mass*0.01);
		// RooRealVar resl("resl", "resolution of BW", 1,0.2, 2, "GeV");
		// RooFormulaVar l("l", "l", "@0*@1", RooArgList(sigma,resl));
		// RooVoigtian Voigtian("Voigtian", "Voigtian", Mm_mass, mu, mass*reso_l,mass*reso);

		// RooRealVar sigmaL_res("sigmaL_res", "reso of left CB", 1, 0.2, 2, "GeV");
		// RooRealVar sigmaR_res("sigmaR_res", "reso of right CB", 1, 0.2, 2, "GeV");
		// RooFormulaVar sigmaL("sigmaL", "sigmaL", "sigma*sigmaL_res", RooArgList(sigma,sigmaL_res));
		// RooFormulaVar sigmaR("sigmaR", "sigmaR", "sigma*sigmaR_res", RooArgList(sigma,sigmaR_res));

		// RooRealVar nL("nL", "nL CB", 4, 3,15, "");
		// RooRealVar alphaL("alphaL", "Alpha left CB", 5, 2, 5, "");
		// RooRealVar nR("nR", "nR CB", 4, 3,15, "");
		// RooRealVar alphaR("alphaR", "Alpha right CB", 5, 2, 5, "");

		RooVoigtian Voigtian("Voigtian", "Voigtian", Mm_mass, mu, l,sigma);
		RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigma, sigma, alphaL,nL,alphaR,nR);

		RooRealVar VoigtFraction("VoigtFraction", "Fraction of Voigtian", 0.50539);

        // Final model is sigModel
        RooAddPdf signalModel_catC("signalModel_catC", "mass model", RooArgList(Voigtian, CB), VoigtFraction);

      	RooDataHist data_obs_catC("data_obs_catC", "", RooArgList(Mm_mass), catC);
	
		// RooRealVar bkg_norm("bkg_norm", "",catC->Integral());

		//Bernstein
		RooRealVar par1("par1", "par1", 0.2, 0, 10);
		RooRealVar par2("par2", "par2", 1.5, 0, 10);
		RooRealVar par3("par3", "par3", 2.0, 0, 10);
		RooRealVar par4("par4", "par4", 2.0, 0, 10);
		RooArgList alist(par1, par2, par3, par4);
		RooBernstein bkg_model_bern4("bkg_model_bern4", "bkg_model_bern4", Mm_mass, alist);
		bkg_model_bern4.fitTo(data_obs_catC,RooFit::Verbose(false));//,RooMinimizer::PrintLevel(-1));
		

		// //Polynomial times exp
		RooRealVar lar1("lar1", "lar1", 0.0, -10.0, 10.0);
		RooRealVar lar2("lar2", "lar2", 0.0, -10.0, 10.0);
		RooRealVar lar3("lar3", "lar3", 0.0, -10.0, 10.0);
		RooRealVar lar4("lar4", "lar4", 0.0, -10.0, 10.0);
		RooArgList llist(lar1, lar2, lar3, lar4);
		RooPolynomial bkg_model_line4("bkg_model_line", "bkg_model_line", Mm_mass, llist, 1);
		//Exponentials
		RooRealVar car1("car1", "car1", -0.5, -10, 10);
		RooExponential bkg_model_exp4("bkg_model_exp4", "bkg_model_exp4", Mm_mass, car1);
		//Product of the two
		RooProdPdf bkg_model_pol4xexp("bkg_model_pol4xexp", "bkg_model_pol4xexp", bkg_model_line4, bkg_model_exp4);
		bkg_model_pol4xexp.chi2FitTo(data_obs_catC,RooFit::Verbose(false));//,RooMinimizer::PrintLevel(-1));


		//Sum of exp
		RooRealVar bar1("bar1", "bar1", -0.5, -10, 10);                            
		RooRealVar bf1("bf1","bf1",0.2,0.0,1.0);   
		RooExponential exp1("exp1", "exp1", Mm_mass, bar1);
		RooRealVar bar2("bar2", "bar2", -0.5, -10, 10);                                                                                           
		RooRealVar bf2("bf2","bf2",0.2,0.0,1.0);                                                                                                  
		RooExponential exp2("exp2", "exp2", Mm_mass, bar2);
		RooRealVar bar3("bar3", "bar3", -0.5, -10, 10);
		RooRealVar bf3("bf3","bf3",0.2,0.0,1.0);
		RooExponential exp3("exp3", "exp3", Mm_mass, bar3);
		RooRealVar bar4("bar4", "bar4", -0.5, -10, 10);
		RooRealVar bf4("bf4","bf4",0.2,0.0,1.0);
		RooExponential exp4("exp4", "exp4", Mm_mass, bar4);
		RooRealVar bar5("bar5", "bar5", -0.5, -10, 10);
		RooRealVar bf5("bf5","bf5",0.2,0.0,1.0);
		RooExponential exp5("exp5", "exp5", Mm_mass, bar5);
        RooRealVar bar6("bar6", "bar6", -0.5, -10, 10);
		RooRealVar bf6("bf6","bf6",0.2,0.0,1.0);
		RooExponential exp6("exp6", "exp6", Mm_mass, bar6);
		RooRealVar bar7("bar7", "bar7", -0.5, -10, 10);
		RooExponential exp7("exp7", "exp7", Mm_mass, bar7);
		RooArgList explist(exp1,exp2,exp3,exp4,exp5,exp6);
		RooArgList expclist(bf1,bf2,bf3,bf4,bf5);
		RooAddPdf bkg_model_exp7("bkg_model_exp7","bkg_model_exp7",explist,expclist,true);
		bkg_model_exp7.fitTo(data_obs_catC,RooFit::Verbose(false));//,RooMinimizer::PrintLevel(-1));

		//powerlaw + bernstein
		RooRealVar pow_1("pow_1","exponent of power law",0,-10,10);
		RooRealVar pf1("pf1","frac of power law",0.2,0.0,1.0);
		RooGenericPdf plaw1("plaw1","TMath::Power(@0,@1)",RooArgList(Mm_mass,pow_1));
		RooRealVar qar1("qar1", "qar1", 0.2, 0, 10);
		RooRealVar qar2("qar2", "qar2", 1.5, 0, 10);
		RooRealVar qar3("qar3", "qar3", 2.0, 0, 10);
		RooArgList qlist(qar1, qar2, qar3);
		RooRealVar bfp1("bfp1","frac of bernstein",0.2,0.0,1.0);
		RooBernstein bern3("bkg_model_bern3", "bkg_model_bern3", Mm_mass, qlist);
		RooArgList plawlist1(plaw1, bern3);
		RooArgList plawclist1(pf1, bfp1);
		RooAddPdf bkg_model_bern3p1("bkg_model_bern3p1","bkg_model_bern3p1",plawlist1,plawclist1,false);
		bkg_model_bern3p1.fitTo(data_obs_catC,RooFit::Verbose(false));//,RooMinimizer::PrintLevel(-1));


		RooCategory  pdf_index("pdf_index","Index of the background PDF which is active");
		RooArgList bkg_pdf_list;		
		bkg_pdf_list.add(bkg_model_bern4);
		bkg_pdf_list.add(bkg_model_pol4xexp);
		bkg_pdf_list.add(bkg_model_exp7);
		bkg_pdf_list.add(bkg_model_bern3p1);

		RooMultiPdf bkg_model("bkg_model", "All Pdfs", pdf_index, bkg_pdf_list);	       
		bkg_model.setCorrectionFactor(0.5); //Note, by default, the multipdf will tell combine to add 0.5 to the NLL for each parameter in the pdf. This is known as the penalty term (or correction factor) for the discrete profiling method. 
	

		//fit and get the background PDF
		// TF1 *bkfit = new TF1("bkfit","pol3",massLow,massHigh);
		// catC->Fit(bkfit);
		// /*TCanvas * catCc = new TCanvas("catCc", "catCc", 500, 500);
		// catC->Draw();
		// catCc->SaveAs(Form("catC%d"+year+".png",i));*/ 
		// if(i>24. && (i<200. || i>240.)) {histChi2_A->Fill(bkfit->GetChisquare()/bkfit->GetNDF());}
		
        // RooRealVar a0a("a0a","a0a",bkfit->GetParameter(0),(bkfit->GetParameter(0)-bkfit->GetParError(0)*5.),(bkfit->GetParameter(0)+bkfit->GetParError(0)*5.));
		// RooRealVar a1a("a1a","a1a",bkfit->GetParameter(1),(bkfit->GetParameter(1)-bkfit->GetParError(1)*5.),(bkfit->GetParameter(1)+bkfit->GetParError(1)*5.));
      	// RooRealVar a2a("a2a","a2a",bkfit->GetParameter(2),(bkfit->GetParameter(2)-bkfit->GetParError(2)*5.),(bkfit->GetParameter(2)+bkfit->GetParError(2)*5.));
		// RooRealVar a3a("a3a","a3a",bkfit->GetParameter(3),(bkfit->GetParameter(3)-bkfit->GetParError(3)*5.),(bkfit->GetParameter(3)+bkfit->GetParError(3)*5.));
		// RooPolynomial bkg_mass_catC("bkg_mass_catC","bkg_mass_catC",Mm_mass,RooArgList(a0a,a1a,a2a,a3a),0);
		
		//save into ROO workspace
		RooWorkspace dpworkspace("dpworkspace", "");
		dpworkspace.import(data_obs_catC);
		dpworkspace.import(signalModel_catC);
		dpworkspace.import(bkg_model);
		// dpworkspace.import(bkg_model_bern4);
		dpworkspace.writeToFile(Form("/data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/dCB_V_multibkg_fit_1to8/dpWorkspace"+year+suff+"_%d.root",i));

		//write the datacard
		char inputShape[200];
        sprintf(inputShape,"/data/submit/mori25/dark_photons_ludo/DimuonTrees/cards/dCB_V_multibkg/dpCard_"+year+suff+"_%d.txt",i);
        ofstream newcardShape;
        newcardShape.open(inputShape);
        newcardShape << Form("imax 1 number of channels\n");
        newcardShape << Form("jmax * number of background\n");
        newcardShape << Form("kmax * number of nuisance parameters\n");
		newcardShape << Form("shapes data_obs	catC /data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/dCB_V_multibkg_fit_1to8/dpWorkspace"+year+suff+"_%d.root dpworkspace:data_obs_catC\n",i);
		newcardShape << Form("shapes bkg_mass	catC /data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/dCB_V_multibkg_fit_1to8/dpWorkspace"+year+suff+"_%d.root dpworkspace:bkg_model\n",i);
		newcardShape << Form("shapes signalModel	catC /data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/dCB_V_multibkg_fit_1to8/dpWorkspace"+year+suff+"_%d.root dpworkspace:signalModel_catC\n",i);
        newcardShape << Form("bin		catC \n");
        newcardShape << Form("observation 	-1.0\n");
        newcardShape << Form("bin     		catC		catC \n");
        newcardShape << Form("process 		signalModel  	bkg_mass\n");
        newcardShape << Form("process 		0    		1 \n");
        newcardShape << Form("rate    		%f  		%f	\n", global_eff_lmDY*trigger_sf*ID_sf*mva_sf*luminosity*xSec, catC->Integral()); 
		newcardShape << Form("lumi13p6TeV lnN 	1.014 	-\n");
		newcardShape << Form("sig_model_unc lnN	1.0314 	-\n");
		newcardShape << Form("sig_width_unc lnN	1.2486 	-\n");
		newcardShape << Form("trigger_sf lnN %f	 -\n", trigger_sf_unc);
		newcardShape << Form("mva_sf lnN %f	 -\n", mva_sf_err);
		newcardShape << Form("id_sf lnN	%f 	-\n", ID_sf_unc);
		newcardShape << Form("pdf_index discrete");
		//newcardShape << Form("bkg_norm_catC rateParam catC bkg_mass %f\n",catC->Integral());
		//newcardShape << Form("resA param %f %f\n",resA.getValV(),resA.getValV()*0.1);
		newcardShape.close();
		
	}

	if (save_efficiency){
		// histChi2_A->SaveAs("/data/submit/mori25/dark_photons_ludo/DimuonTrees/histograms/dCB_V_multibkg_fit_1to8/histAchi2_"+year+".root");
		ofstream effs_file("/work/submit/mori25/Darkphotons_ludo/offline_analysis/limits/efficiencies.csv");
		if (!effs_file.is_open()) {
			std::cerr << "Could not open the effs_file." << std::endl;
		}
		else{
			effs_file << "mass" << "," << "effs" << "\n";
			// Write the data to the effs_file
			for (int i = 0; i < num_mass_regions; i++) {
				effs_file << masses[i] << "," << effs[i] << "\n";
			}
			// Close the effs_file
			effs_file.close();
		}
	}

}