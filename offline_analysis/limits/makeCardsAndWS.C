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
	TString suff="";
  //INPUT FILE WITH HISTOGRAMS TO FIT BACKGROUND
  	TFile* file=TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/histograms/offline/output_histogram.root");
	
  //PREPARE EXPECTED NUMBER OF SIGNAL EVENTS PER CATEGORY

	//LUMINOSITY
	double luminosity = 34291.449027853321075;//pb-1

	int num_mass_regions = 278;
    float growth_factor = 0.005;

	//WORKING POINTS
	double id_wp = 0.426;
	double mva_wp = 0.873;

	//EFFICIENCY
	string trigger = "HLT_DoubleMu4_3_LowMass";
	double trigger_eff = 0.94;
	double trigger_eff_unc = 0.01;

	double ID_eff = 0.991;
	double ID_eff_unc = 1 + 0.009/0.991;
	
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

	  	//get the histograms
	  	TH1D* catA=(TH1D*)file->Get(Form("massforLimit_CatA%d",i));
	  	double massLow  =  catA->GetXaxis()->GetXmin();
      	double massHigh =  catA->GetXaxis()->GetXmax();
		double* massLimits = new double[2];
		massLimits[0]=massLow;
		massLimits[1]=massHigh;

		mass = 0.5*(massLow+massHigh);
		masses[i] = mass;

		double global_eff_lmDY = compute_eff("/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/dump_post_BDT/*", trigger, mva_wp,id_wp,massLimits,true);
		effs[i] = global_eff_lmDY;
		double mva_sf = compute_mva_sf(mass);
		cout << mva_sf << endl;

		if ((mass >= 2.6) && (mass <= 4.16)) continue;
		if ((mass >= 8.4) && (mass <= 11.2)) continue;

		RooRealVar Mm_mass  ("Mm_mass"  	, "Mm_mass " 	, massLow		, massHigh);

		// mass model for single resonance. Voigtian + dCB
		RooRealVar mu("mu", "mu", mass);
		RooRealVar sigma("sigma", "Width of Gaussian",mass*0.01);
		RooRealVar resl("resl", "resolution of BW", 1,0.2, 2, "GeV");
		RooFormulaVar l("l", "l", "@0*@1", RooArgList(sigma,resl));
		RooVoigtian Voigtian("Voigtian", "Voigtian", Mm_mass, mu, sigma,l);

		RooRealVar sigmaL_res("sigmaL_res", "reso of left CB", 1, 0.2, 2, "GeV");
		RooRealVar sigmaR_res("sigmaR_res", "reso of right CB", 1, 0.2, 2, "GeV");
		RooFormulaVar sigmaL("sigmaL", "sigmaL", "sigma*sigmaL_res", RooArgList(sigma,sigmaL_res));
		RooFormulaVar sigmaR("sigmaR", "sigmaR", "sigma*sigmaR_res", RooArgList(sigma,sigmaR_res));
		RooRealVar nL("nL", "nL CB", 4, 3,15, "");
		RooRealVar alphaL("alphaL", "Alpha left CB", 5, 2, 5, "");
		RooRealVar nR("nR", "nR CB", 4, 3,15, "");
		RooRealVar alphaR("alphaR", "Alpha right CB", 5, 2, 5, "");
		RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigmaL, sigmaR, alphaL,nL,alphaR,nR);

		RooRealVar VoigtFraction("VoigtFraction", "Fraction of Voigtian", 0.5, 0, 1, "");

        // Final model is sigModel
        RooAddPdf signalModel_CatA("signalModel_CatA", "mass model", RooArgList(Voigtian, CB), VoigtFraction);

      	RooDataHist data_obs_CatA("data_obs_CatA", "", RooArgList(Mm_mass), catA);
	
		// RooRealVar bkg_norm("bkg_norm", "",catA->Integral());

		//Bernstein
		RooRealVar par1_2022("par1_2022", "par1_2022", 0.2, 0, 10);
		RooRealVar par2_2022("par2_2022", "par2_2022", 1.5, 0, 10);
		RooRealVar par3_2022("par3_2022", "par3_2022", 2.0, 0, 10);
		RooRealVar par4_2022("par4_2022", "par4_2022", 2.0, 0, 10);
		RooArgList alist_2022(par1_2022, par2_2022, par3_2022, par4_2022);
		RooBernstein bkg_model_bern4_2022("bkg_model_bern4_2022", "bkg_model_bern4_2022", Mm_mass, alist_2022);
		bkg_model_bern4_2022.fitTo(data_obs_CatA);
		

		// //Polynomial times exp
		RooRealVar lar1_2022("lar1_2022", "lar1_2022", 0.0, -10.0, 10.0);
		RooRealVar lar2_2022("lar2_2022", "lar2_2022", 0.0, -10.0, 10.0);
		RooRealVar lar3_2022("lar3_2022", "lar3_2022", 0.0, -10.0, 10.0);
		RooRealVar lar4_2022("lar4_2022", "lar4_2022", 0.0, -10.0, 10.0);
		RooArgList llist_2022(lar1_2022, lar2_2022, lar3_2022, lar4_2022);
		RooPolynomial bkg_model_line4_2022("bkg_model_line_2022", "bkg_model_line_2022", Mm_mass, llist_2022, 1);
		//Exponentials
		RooRealVar car1_2022("car1_2022", "car1_2022", -0.5, -10, 10);
		RooExponential bkg_model_exp4_2022("bkg_model_exp4_2022", "bkg_model_exp4_2022", Mm_mass, car1_2022);
		//Product of the two
		RooProdPdf bkg_model_pol4xexp_2022("bkg_model_pol4xexp_2022", "bkg_model_pol4xexp_2022", bkg_model_line4_2022, bkg_model_exp4_2022);
		bkg_model_pol4xexp_2022.chi2FitTo(data_obs_CatA);


		//Sum of exp
		RooRealVar bar1_2022("bar1_2022", "bar1_2022", -0.5, -10, 10);                            
		RooRealVar bf1_2022("bf1_2022","bf1_2022",0.2,0.0,1.0);   
		RooExponential exp1_2022("exp1_2022", "exp1_2022", Mm_mass, bar1_2022);
		RooRealVar bar2_2022("bar2_2022", "bar2_2022", -0.5, -10, 10);                                                                                           
		RooRealVar bf2_2022("bf2_2022","bf2_2022",0.2,0.0,1.0);                                                                                                  
		RooExponential exp2_2022("exp2_2022", "exp2_2022", Mm_mass, bar2_2022);
		RooRealVar bar3_2022("bar3_2022", "bar3_2022", -0.5, -10, 10);
		RooRealVar bf3_2022("bf3_2022","bf3_2022",0.2,0.0,1.0);
		RooExponential exp3_2022("exp3_2022", "exp3_2022", Mm_mass, bar3_2022);
		RooRealVar bar4_2022("bar4_2022", "bar4_2022", -0.5, -10, 10);
		RooRealVar bf4_2022("bf4_2022","bf4_2022",0.2,0.0,1.0);
		RooExponential exp4_2022("exp4_2022", "exp4_2022", Mm_mass, bar4_2022);
		RooRealVar bar5_2022("bar5_2022", "bar5_2022", -0.5, -10, 10);
		RooRealVar bf5_2022("bf5_2022","bf5_2022",0.2,0.0,1.0);
		RooExponential exp5_2022("exp5_2022", "exp5_2022", Mm_mass, bar5_2022);
        RooRealVar bar6_2022("bar6_2022", "bar6_2022", -0.5, -10, 10);
		RooRealVar bf6_2022("bf6_2022","bf6_2022",0.2,0.0,1.0);
		RooExponential exp6_2022("exp6_2022", "exp6_2022", Mm_mass, bar6_2022);
		RooRealVar bar7_2022("bar7_2022", "bar7_2022", -0.5, -10, 10);
		RooExponential exp7_2022("exp7_2022", "exp7_2022", Mm_mass, bar7_2022);
		RooArgList explist_2022(exp1_2022,exp2_2022,exp3_2022,exp4_2022,exp5_2022,exp6_2022);
		RooArgList expclist_2022(bf1_2022,bf2_2022,bf3_2022,bf4_2022,bf5_2022);
		RooAddPdf bkg_model_exp7_2022("bkg_model_exp7_2022","bkg_model_exp7_2022",explist_2022,expclist_2022,true);
		bkg_model_exp7_2022.fitTo(data_obs_CatA);

		//powerlaw + bernstein
		RooRealVar pow_1_2022("pow_1_2022","exponent of power law",0,-10,10);
		RooRealVar pf1_2022("pf1_2022","frac of power law",0.2,0.0,1.0);
		RooGenericPdf plaw1_2022("plaw1_2022","TMath::Power(@0,@1)",RooArgList(Mm_mass,pow_1_2022));
		RooRealVar qar1_2022("qar1_2022", "qar1_2022", 0.2, 0, 10);
		RooRealVar qar2_2022("qar2_2022", "qar2_2022", 1.5, 0, 10);
		RooRealVar qar3_2022("qar3_2022", "qar3_2022", 2.0, 0, 10);
		RooArgList qlist_2022(qar1_2022, qar2_2022, qar3_2022);
		RooRealVar bfp1_2022("bfp1_2022","frac of bernstein",0.2,0.0,1.0);
		RooBernstein bern3_2022("bkg_model_bern3_2022", "bkg_model_bern3_2022", Mm_mass, qlist_2022);
		RooArgList plawlist1_2022(plaw1_2022, bern3_2022);
		RooArgList plawclist1_2022(pf1_2022, bfp1_2022);
		RooAddPdf bkg_model_bern3p1_2022("bkg_model_bern3p1_2022","bkg_model_bern3p1_2022",plawlist1_2022,plawclist1_2022,false);
		bkg_model_bern3p1_2022.fitTo(data_obs_CatA);


		RooCategory  pdf_index_2022("pdf_index_2022","Index of the background PDF which is active");
		RooArgList bkg_pdf_list_2022;		
		bkg_pdf_list_2022.add(bkg_model_bern4_2022);
		bkg_pdf_list_2022.add(bkg_model_pol4xexp_2022);
		bkg_pdf_list_2022.add(bkg_model_exp7_2022);
		bkg_pdf_list_2022.add(bkg_model_bern3p1_2022);

		RooMultiPdf bkg_model_2022("bkg_model_2022", "All Pdfs", pdf_index_2022, bkg_pdf_list_2022);	       
		bkg_model_2022.setCorrectionFactor(0.5); //Note, by default, the multipdf will tell combine to add 0.5 to the NLL for each parameter in the pdf. This is known as the penalty term (or correction factor) for the discrete profiling method. 
	

		//fit and get the background PDF
		// TF1 *bkfit = new TF1("bkfit","pol3",massLow,massHigh);
		// catA->Fit(bkfit);
		// /*TCanvas * catAc = new TCanvas("catAc", "catAc", 500, 500);
		// catA->Draw();
		// catAc->SaveAs(Form("catA%d"+year+".png",i));*/ 
		// if(i>24. && (i<200. || i>240.)) {histChi2_A->Fill(bkfit->GetChisquare()/bkfit->GetNDF());}
		
        // RooRealVar a0a("a0a","a0a",bkfit->GetParameter(0),(bkfit->GetParameter(0)-bkfit->GetParError(0)*5.),(bkfit->GetParameter(0)+bkfit->GetParError(0)*5.));
		// RooRealVar a1a("a1a","a1a",bkfit->GetParameter(1),(bkfit->GetParameter(1)-bkfit->GetParError(1)*5.),(bkfit->GetParameter(1)+bkfit->GetParError(1)*5.));
      	// RooRealVar a2a("a2a","a2a",bkfit->GetParameter(2),(bkfit->GetParameter(2)-bkfit->GetParError(2)*5.),(bkfit->GetParameter(2)+bkfit->GetParError(2)*5.));
		// RooRealVar a3a("a3a","a3a",bkfit->GetParameter(3),(bkfit->GetParameter(3)-bkfit->GetParError(3)*5.),(bkfit->GetParameter(3)+bkfit->GetParError(3)*5.));
		// RooPolynomial bkg_mass_CatA("bkg_mass_CatA","bkg_mass_CatA",Mm_mass,RooArgList(a0a,a1a,a2a,a3a),0);
		
		//save into ROO workspace
		RooWorkspace dpworkspace("dpworkspace", "");
		dpworkspace.import(data_obs_CatA);
		dpworkspace.import(signalModel_CatA);
		// dpworkspace.import(bkg_model_2022);
		dpworkspace.import(bkg_model_bern4_2022);
		dpworkspace.writeToFile(Form("/data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/dCB_V_multibkg_fit/dpWorkspace"+year+suff+"_%d.root",i));

		//write the datacard
		char inputShape[200];
        sprintf(inputShape,"/data/submit/mori25/dark_photons_ludo/DimuonTrees/cards/dCB_V_multibkg/dpCard_"+year+suff+"_%d.txt",i);
        ofstream newcardShape;
        newcardShape.open(inputShape);
        newcardShape << Form("imax 1 number of channels\n");
        newcardShape << Form("jmax * number of background\n");
        newcardShape << Form("kmax * number of nuisance parameters\n");
		newcardShape << Form("shapes data_obs	CatA /data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/dCB_V_multibkg_fit/dpWorkspace"+year+suff+"_%d.root dpworkspace:data_obs_CatA\n",i);
		newcardShape << Form("shapes bkg_mass	CatA /data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/dCB_V_multibkg_fit/dpWorkspace"+year+suff+"_%d.root dpworkspace:bkg_model_2022\n",i);
		newcardShape << Form("shapes signalModel	CatA /data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/dCB_V_multibkg_fit/dpWorkspace"+year+suff+"_%d.root dpworkspace:signalModel_CatA\n",i);
        newcardShape << Form("bin		CatA \n");
        newcardShape << Form("observation 	-1.0\n");
        newcardShape << Form("bin     		CatA		CatA \n");
        newcardShape << Form("process 		signalModel  	bkg_mass\n");
        newcardShape << Form("process 		0    		1 \n");
        newcardShape << Form("rate    		%f  		%f	\n", global_eff_lmDY*trigger_eff*ID_eff*mva_sf*luminosity*xSec, catA->Integral()); //used to be 0.64*lum*xSec
		newcardShape << Form("lumi13TeV_2022 lnN 	1.026 	-\n");
		newcardShape << Form("eff_mu_2022 lnN	1.071 	-\n");
		newcardShape << Form("sig_model_unc lnN	1.045 	-\n");
		newcardShape << Form("id_eff_mva_2022 lnN	%f 	-\n", ID_eff_unc);
		//newcardShape << Form("bkg_norm_cata rateParam CatA bkg_mass %f\n",catA->Integral());
		//newcardShape << Form("resA param %f %f\n",resA.getValV(),resA.getValV()*0.1);
		newcardShape.close();
		
	}

	if (save_efficiency){
		// histChi2_A->SaveAs("/data/submit/mori25/dark_photons_ludo/DimuonTrees/histograms/dCB_V_multibkg_fit/histAchi2_"+year+".root");
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