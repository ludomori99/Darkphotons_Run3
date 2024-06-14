#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/CMS.C"
#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/setTDRStyle.C"
#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/sandbox.C"
#include "2D/create_TH2D.h"
#include "2D/DoFit2D.cpp"
#include "2D/McYield2D.cpp"
#include "2D/get_efficiency_TH2D.h"
#include "2D/yields_n_errs_to_TH2D_bin.h"

#include <unordered_map>


void compute2d(string dataset, bool DataIsMC, string xquantity, string yquantity, double* xbins, double* ybins, int nbinsx, int nbinsy, string MuonId);


void Efficiency2D(){

	sandbox(); //To absorb the weirdly behaving text in the first plot

	string MuonId   = "PassingProbeSoftId";

	// Bins to study
	// double xbins[] = {4, 4.2, 4.4, 4.7, 4.9, 5.2, 5.5, 5.8, 6.2, 6.6, 7.3, 9.5, 13.0, 17.0, 25, 40};
	double* xbins_Probe_pt = new double[9]{4, 5, 7, 10, 14, 20.0, 40};
	double* ybins_Probe_abs_eta = new double[10]{0., 0.2, 0.4, 0.6,0.8, 1., 1.2,  1.4, 1.6, 1.8};
	double* ybins_Mm_dR = new double[7]{0., 0.1, 0.2, 0.3,0.4,0.6,0.8 };

	compute2d("Offline",false, "Probe_pt", "Probe_abs_eta", xbins_Probe_pt, ybins_Probe_abs_eta, 6,9,MuonId);
	compute2d("MinBias",true, "Probe_pt", "Probe_abs_eta", xbins_Probe_pt, ybins_Probe_abs_eta, 6,9,MuonId);

	compute2d("Offline",false, "Probe_pt", "Mm_dR", xbins_Probe_pt, ybins_Mm_dR,6,6, MuonId);
	compute2d("MinBias",true, "Probe_pt", "Mm_dR", xbins_Probe_pt, ybins_Mm_dR,6,6, MuonId);
}


void compute2d(string dataset, bool DataIsMC, string xquantity, string yquantity, double* xbins, double* ybins, int nbinsx, int nbinsy, string MuonId)
{

	// Dataset = MinBias, Offline, lmDY
	unordered_map<string, const char*> paths;
	paths["MinBias"] = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Jpsi/TP_samples_Jpsi.root";
	paths["Offline"] = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Jpsi/TP_samples_Jpsi.root";
	paths["lmDY"] = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/lmDY/Jpsi/inclusive/TP_samples_Jpsi.root";


	//input file
	const char* filepath = paths[dataset];

	//Path where is going to save fit results png for every bin 
	// string path_bins_fit_folder = string("/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits/")+MuonId+string("/")+study_name+string("/");
	// create_folder(path_bins_fit_folder.c_str(), true);

	string isMC="";
	if (DataIsMC) isMC = "MC";
	else isMC = "Data";
	//Path where is going to save the efficiency results
	string directoryToSave = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId +string("/");
	// create_folder(directoryToSave.c_str());

	//Get number of bins
	// const int nbinsx = sizeof(xbins)/sizeof(*xbins) - 1;
	// const int nbinsy = sizeof(ybins)/sizeof(*ybins) - 1;

// CHECK FIXS OR NOT
	string file_path = directoryToSave + xquantity + "_" + yquantity + "/" + isMC + "_fixS.root";
	cout<<"file_path: "<<file_path<<endl;
	TFile* generatedFile = new TFile(file_path.c_str(),"recreate");
	generatedFile->mkdir("histograms/");
	generatedFile->   cd("histograms/");

	TH2D *hist_all_nominal     = create_TH2D("all_nominal"   ,  "All Nominal",     xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_all_2gauss      = create_TH2D("all_2xGauss"   ,  "All 2xGauss",     xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_all_massup      = create_TH2D("all_MassUp"    ,  "All MassUp",      xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_all_massdown    = create_TH2D("all_MassDown"  ,  "All MassDown",    xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_all_binup       = create_TH2D("all_BinUp"     ,  "All BinUp",       xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_all_bindown     = create_TH2D("all_BinDown"   ,  "All BinDown",     xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_all_systematic  = create_TH2D("all_systematic",  "All Systematic",  xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_all_final       = create_TH2D("all_final"     ,  "All Final",       xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);

	TH2D *hist_pass_nominal    = create_TH2D("pass_nominal"   , "Pass Nominal",    xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_pass_2gauss     = create_TH2D("pass_2xGauss"   , "Pass 2xGauss",    xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_pass_massup     = create_TH2D("pass_MassUp"    , "Pass MassUp",     xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_pass_massdown   = create_TH2D("pass_MassDown"  , "Pass MassDown",   xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_pass_binup      = create_TH2D("pass_BinUp"     , "Pass BinUp",      xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_pass_bindown    = create_TH2D("pass_BinDown"   , "Pass BinDown",    xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_pass_systematic = create_TH2D("pass_systematic", "Pass Systematic", xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);
	// TH2D *hist_pass_final      = create_TH2D("pass_final"     , "Pass Final",      xquantity, yquantity, nbinsx, nbinsy, xbins, ybins);


	//Loop and fits
	for (int i = 0; i < nbinsx; i++)
	{
		for (int j = 0; j < nbinsy; j++)
		{
			//Creates conditions
			string conditions = string( "HLT_DoubleMu4_3_LowMass==1&&" + xquantity + ">=" + to_string(xbins[i]));
			conditions +=       string("&&" + xquantity + "<" + to_string(xbins[i+1]));
			conditions +=       string("&&" + yquantity + ">=" + to_string(ybins[j]  ));
			conditions +=       string("&&" + yquantity + "<" + to_string(ybins[j+1]));

			string mmin_string;
			string mmax_string;
			double* yields_n_errs = NULL;
			double  yields_n_errs_systematic[4] = {0};
			double  yields_n_errs_final[4] = {0};

			//Nominal
			cout << "Nominal calculation "<<conditions<< "\n";
			if (DataIsMC) yields_n_errs =  McYield2D(filepath, conditions, MuonId,xquantity,yquantity);
			else yields_n_errs = doFit2D(filepath, conditions, MuonId,xquantity,yquantity);	//doFit returns: [yield_all, yield_pass, err_all, err_pass]
			yields_n_errs_systematic[0] =  yields_n_errs[0];
			yields_n_errs_systematic[1] =  yields_n_errs[1];
			yields_n_errs_final[0] =  yields_n_errs[0];
			yields_n_errs_final[1] =  yields_n_errs[1];
			yields_n_errs_final[2] =  yields_n_errs[2];
			yields_n_errs_final[3] =  yields_n_errs[3];
			yields_n_errs_to_TH2Ds_bin(hist_all_nominal, hist_pass_nominal, i+1, j+1, yields_n_errs);
			cout<<"added to bin\n";
			// //2Gauss
			// cout << "2xGassians calculation -----\n";
			// _mmin = default_min;
			// _mmax = default_max;
			// fit_bins = 100;
			// prefix_file_name = "2xgaus_";
			// yields_n_errs = doFit2xGaus(conditions, MuonId, string(path_bins_fit_folder + prefix_file_name).c_str());
			// yields_n_errs_systematic[2] += pow(yields_n_errs[2], 2);
			// yields_n_errs_systematic[3] += pow(yields_n_errs[3], 2);
			// yields_n_errs_to_TH2Ds_bin(hist_all_2gauss, hist_pass_2gauss, i+1, j+1, yields_n_errs);

			//MassUp
			// cout << "MassUp calculation -----\n";
			// _mmin = default_min - 0.05;
			// _mmax = default_max + 0.05;
			// fit_bins = 100;
			// mmin_string = to_string(_mmin);
			// mmax_string = to_string(_mmax);
			// replace(mmin_string.begin(), mmin_string.end(), '.', 'p');
			// replace(mmax_string.begin(), mmax_string.end(), '.', 'p');
			// prefix_file_name  = string("mass_") + mmin_string.substr(0, mmin_string.length()-4) + string("_");
			// prefix_file_name +=                   mmax_string.substr(0, mmax_string.length()-4) + string("_");
			// yields_n_errs = doFit(conditions, MuonId, string(path_bins_fit_folder + prefix_file_name).c_str());
			// yields_n_errs_systematic[2] += pow(yields_n_errs[2], 2);
			// yields_n_errs_systematic[3] += pow(yields_n_errs[3], 2);
			// yields_n_errs_to_TH2Ds_bin(hist_all_massup, hist_pass_massup, i+1, j+1, yields_n_errs);

			// //MassDown
			// cout << "MassDown calculation -----\n";
			// _mmin = default_min + 0.05;
			// _mmax = default_max - 0.05;
			// fit_bins = 100;
			// mmin_string = to_string(_mmin);
			// mmax_string = to_string(_mmax);
			// replace(mmin_string.begin(), mmin_string.end(), '.', 'p');
			// replace(mmax_string.begin(), mmax_string.end(), '.', 'p');
			// prefix_file_name  = string("mass_") + mmin_string.substr(0, mmin_string.length()-4) + string("_");
			// prefix_file_name +=                   mmax_string.substr(0, mmax_string.length()-4) + string("_");
			// yields_n_errs = doFit(conditions, MuonId, string(path_bins_fit_folder + prefix_file_name).c_str());
			// yields_n_errs_systematic[2] += pow(yields_n_errs[2], 2);
			// yields_n_errs_systematic[3] += pow(yields_n_errs[3], 2);
			// yields_n_errs_to_TH2Ds_bin(hist_all_massdown, hist_pass_massdown, i+1, j+1, yields_n_errs);

			// //BinUp
			// cout << "BinUp calculation -----\n";
			// _mmin = default_min;
			// _mmax = default_max;
			// fit_bins = 105;
			// prefix_file_name = "binfit105_";
			// yields_n_errs = doFit(conditions, MuonId, string(path_bins_fit_folder + prefix_file_name).c_str());
			// yields_n_errs_systematic[2] += pow(yields_n_errs[2], 2);
			// yields_n_errs_systematic[3] += pow(yields_n_errs[3], 2);
			// yields_n_errs_to_TH2Ds_bin(hist_all_binup, hist_pass_binup, i+1, j+1, yields_n_errs);

			// //BinDown
			// cout << "BinDown calculation -----\n";
			// _mmin = default_min;
			// _mmax = default_max;
			// fit_bins = 95;
			// prefix_file_name = "binfit95_";
			// yields_n_errs = doFit(conditions, MuonId, string(path_bins_fit_folder + prefix_file_name).c_str());
			// yields_n_errs_systematic[2] += pow(yields_n_errs[2], 2);
			// yields_n_errs_systematic[3] += pow(yields_n_errs[3], 2);
			// yields_n_errs_to_TH2Ds_bin(hist_all_bindown, hist_pass_bindown, i+1, j+1, yields_n_errs);



			// //Make the systematic calculations
			// yields_n_errs_systematic[2] = sqrt(yields_n_errs_systematic[2]);
			// yields_n_errs_systematic[3] = sqrt(yields_n_errs_systematic[3]);
			// yields_n_errs_to_TH2Ds_bin(hist_all_systematic, hist_pass_systematic, i+1, j+1, yields_n_errs_systematic);


			// //Make the statistic+systematic calculations
			// yields_n_errs_final[2] = sqrt(pow(yields_n_errs_final[2], 2) + pow(yields_n_errs_systematic[2], 2));
			// yields_n_errs_final[3] = sqrt(pow(yields_n_errs_final[3], 2) + pow(yields_n_errs_systematic[3], 2));
			// yields_n_errs_to_TH2Ds_bin(hist_all_final, hist_pass_final, i+1, j+1, yields_n_errs_final);


			delete yields_n_errs;
		}
	}

	generatedFile->cd("/");
	get_efficiency_TH2D(hist_all_nominal,    hist_pass_nominal, DataIsMC,   xquantity, yquantity, MuonId, "Nominal"   );
	// get_efficiency_TH2D(hist_all_2gauss,     hist_pass_2gauss,     xquantity, yquantity, MuonId, "2xGauss"   );
	// get_efficiency_TH2D(hist_all_massup,     hist_pass_massup,     xquantity, yquantity, MuonId, "MassUp"    );
	// get_efficiency_TH2D(hist_all_massdown,   hist_pass_massdown,   xquantity, yquantity, MuonId, "MassDown"  );
	// get_efficiency_TH2D(hist_all_binup,      hist_pass_binup,      xquantity, yquantity, MuonId, "BinUp"     );
	// get_efficiency_TH2D(hist_all_bindown,    hist_pass_bindown,    xquantity, yquantity, MuonId, "BinDown"   );
	// get_efficiency_TH2D(hist_all_systematic, hist_pass_systematic, xquantity, yquantity, MuonId, "Systematic");
	// get_efficiency_TH2D(hist_all_final,      hist_pass_final,      xquantity, yquantity, MuonId, "Final"     );

	generatedFile->Write();

	cout << "\n------------------------\n";
	cout << "Output: " << file_path;
	cout << "\n------------------------\n";
}