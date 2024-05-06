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
	//EFFICIENCY

	double trigger_eff = 0.94
	
	//scale
	double xSec = 1.;

   //LOOP OVER MASS INDICES AND MAKE THE CARDS/WORKSPACES
	double mass = 1;
	for(int i=0; i<470; i++){

	  
	  	//get the histograms
	  	TH1D* catA=(TH1D*)file->Get(Form("massforLimit_CatA%d",i));
	  	double massLow  =  catA->GetXaxis()->GetXmin();
      	double massHigh =  catA->GetXaxis()->GetXmax();
	  

		//compute mass point and define ROOFit variables
	  	mass = mass+(mass*0.01);
		//cout<<"Spline: "<<effAgraph->Eval(mass,0,"S")<<endl;
		//cout<<"Graph : "<<effAgraph->Eval(mass)<<endl;
		RooRealVar Mm_mass  ("Mm_mass"  	, "Mm_mass " 	, massLow		, massHigh);

		RooRealVar mu    ("mu"       	, "mu"          , mass);	
		RooRealVar resA  ("resA"  	, "RFes. unc. " , mass*0.02);//to be checked
        RooGaussian Gaussian("Gaussian", "Gaussian", Mm_mass, mu, resA);

        RooRealVar sigmaL("sigmaL", "Width of left CB",  0.001, 10, "GeV");
        RooRealVar sigmaR("sigmaR", "Width of right CB", 0.001, 10, "GeV");
        RooRealVar nL("nL", "nL CB", 0.1,15, "");
        RooRealVar alphaL("alphaR", "Alpha right CB", 0.001, 5, "");
        RooRealVar nR("nR", "nR CB",0.1,15, "");
        RooRealVar alphaR("alphaL", "Alpha left CB",  0.001, 5, "");
        RooCrystalBall CB("CB", "CB", Mm_mass, mu, sigmaL, sigmaR, alphaL,nL,alphaR,nR);

        RooRealVar GaussFraction("GaussFraction", "Fraction of Gaussian", 0.5, 0, 1, "");

        // Final model is sigModel
        RooAddPdf signalModel_CatA("signalModel_CatA", "mass model", RooArgList(Gaussian, CB), GaussFraction);

      	RooDataHist data_obs_CatA("data_obs_CatA", "", RooArgList(Mm_mass), catA);
	
		RooRealVar bkg_norm("bkg_norm", "",catA->Integral());
	

		//fit and get the background PDF
		TF1 *bkfit = new TF1("bkfit","pol3",massLow,massHigh);
		catA->Fit(bkfit);
		/*TCanvas * catAc = new TCanvas("catAc", "catAc", 500, 500);
		catA->Draw();
		catAc->SaveAs(Form("catA%d"+year+".png",i));*/ 
		if(i>24. && (i<200. || i>240.)) {histChi2_A->Fill(bkfit->GetChisquare()/bkfit->GetNDF());}
		
        RooRealVar a0a("a0a","a0a",bkfit->GetParameter(0),(bkfit->GetParameter(0)-bkfit->GetParError(0)*5.),(bkfit->GetParameter(0)+bkfit->GetParError(0)*5.));
		RooRealVar a1a("a1a","a1a",bkfit->GetParameter(1),(bkfit->GetParameter(1)-bkfit->GetParError(1)*5.),(bkfit->GetParameter(1)+bkfit->GetParError(1)*5.));
      	RooRealVar a2a("a2a","a2a",bkfit->GetParameter(2),(bkfit->GetParameter(2)-bkfit->GetParError(2)*5.),(bkfit->GetParameter(2)+bkfit->GetParError(2)*5.));
		RooRealVar a3a("a3a","a3a",bkfit->GetParameter(3),(bkfit->GetParameter(3)-bkfit->GetParError(3)*5.),(bkfit->GetParameter(3)+bkfit->GetParError(3)*5.));
		RooPolynomial bkg_mass_CatA("bkg_mass_CatA","bkg_mass_CatA",Mm_mass,RooArgList(a0a,a1a,a2a,a3a),0);
		
		//save into ROO workspace
		RooWorkspace dpworkspace("dpworkspace", "");
      		dpworkspace.import(data_obs_CatA);
	
      		dpworkspace.import(signalModel_CatA);
	
      		dpworkspace.import(bkg_mass_CatA);
	
      		dpworkspace.writeToFile(Form("/data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/simple_model/dpWorkspace"+year+suff+"_%d.root",i));

		//write the datacard
		char inputShape[200];
        sprintf(inputShape,"/data/submit/mori25/dark_photons_ludo/DimuonTrees/cards/simple_model/dpCard_"+year+suff+"_%d.txt",i);
        ofstream newcardShape;
        newcardShape.open(inputShape);
        newcardShape << Form("imax 1 number of channels\n");
        newcardShape << Form("jmax * number of background\n");
        newcardShape << Form("kmax * number of nuisance parameters\n");
		newcardShape << Form("shapes data_obs	CatA /data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/simple_model/dpWorkspace"+year+suff+"_%d.root dpworkspace:data_obs_CatA\n",i);
		newcardShape << Form("shapes bkg_mass	CatA /data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/simple_model/dpWorkspace"+year+suff+"_%d.root dpworkspace:bkg_mass_CatA\n",i);
		newcardShape << Form("shapes signalModel	CatA /data/submit/mori25/dark_photons_ludo/DimuonTrees/workspaces/simple_model/dpWorkspace"+year+suff+"_%d.root dpworkspace:signalModel_CatA\n",i);
        newcardShape << Form("bin		CatA \n");
        newcardShape << Form("observation 	-1.0\n");
        newcardShape << Form("bin     		CatA		CatA \n");
        newcardShape << Form("process 		signalModel  	bkg_mass\n");
        newcardShape << Form("process 		0    		1 \n");
        newcardShape << Form("rate    		%f  		%f	\n", 0.64*luminosity*xSec, catA->Integral());
		newcardShape << Form("lumi13TeV_2022 lnN 	1.026 	-\n");
		newcardShape << Form("eff_mu_13TeV_2018 lnN	1.124 	-\n");
		newcardShape << Form("id_eff_mva_2022 lnN	1.040 	-\n");
		//newcardShape << Form("bkg_norm_cata rateParam CatA bkg_mass %f\n",catA->Integral());
		//newcardShape << Form("resA param %f %f\n",resA.getValV(),resA.getValV()*0.1);
		newcardShape.close();
		
	}
	histChi2_A->SaveAs("/data/submit/mori25/dark_photons_ludo/DimuonTrees/histograms/simple_model/histAchi2_"+year+".root");

}