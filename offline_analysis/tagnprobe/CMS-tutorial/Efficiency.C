
#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/CMS.C"
#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/setTDRStyle.C"
#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/sandbox.C"
#include "src/DoFit.cpp"
#include "src/compare_efficiency.C"
#include "src/get_conditions.cpp"
#include "src/get_efficiency.cpp"
#include "src/change_bin.cpp"
#include "src/make_hist.cpp"
#include "src/McYield.cpp"
#include <cstdlib>

#include <unordered_map>


void compute_eff(unordered_map<string, const char*>, string,bool, string, string, bool=false, bool=false);
void full_study(string, string, bool);

void Efficiency(){

    sandbox(); //To absorb the weirdly behaving text in the first plot 

    string MuonId = "PassingProbeSoftId";

    full_study("Probe_pt", MuonId, false);
    full_study("Mm_dR", MuonId, false);
    full_study("Probe_eta", MuonId, false);
    full_study("Probe_abs_eta", MuonId, false);
}

void full_study(string quantity, string MuonId, bool barrelVsEndcap){

    unordered_map<string, const char*> paths;
    paths["Data_MinBias"] = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Jpsi/TP_samples_Jpsi.root";
    paths["Data_Offline"] = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Jpsi/TP_samples_Jpsi.root";
    // paths["lmDY"] = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/lmDY/Jpsi/TP_samples_Jpsi.root";
    string eff_minbias_str = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/") + quantity + string("/Efficiency_MinBias.root");
    string eff_offline_str = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/") + quantity + string("/Efficiency_Run3.root");
    paths["Eff_MinBias"] = eff_minbias_str.c_str();
    paths["Eff_Offline"] = eff_offline_str.c_str();


    compute_eff(paths, "MinBias",true,quantity,MuonId);
    compute_eff(paths, "Offline",false,quantity,MuonId);
    // if (barrelVsEndcap){
    //     compute_eff(paths, "MinBias",true,quantity,MuonId, true,false);
    //     compute_eff(paths, "MinBias",true,quantity,MuonId, false,true);
    //     compute_eff(paths, "Offline",false,quantity,MuonId, true,false);
    //     compute_eff(paths, "Offline",false,quantity,MuonId,false, true);
    //     compare_efficiency(quantity,paths["Eff_MinBias"],paths["Eff_Offline"],MuonId,true);
    // }
    
    // compute_eff("lmDY",true,quantity,MuonId);
    // compute_eff("lmDY",true,quantity,MuonId, true,false);
    // compute_eff("lmDY",true,quantity,MuonId,false, true);

    compare_efficiency(quantity,paths["Eff_MinBias"],paths["Eff_Offline"],MuonId,false); 
}


void compute_eff(unordered_map<string, const char*> paths, string dataset, bool DataIsMC, string quantity, string MuonId, bool isBarrel=false, bool isEndcap=false){

    if (dataset=="lmDY" && !DataIsMC){
        cout<<"lmDYPath is only for MC\n";
        return;
    }

    vector<double> bins; 
    int bin_n;

    if (quantity=="Probe_pt"){
        double bins_tmp[] =  {4, 4.2, 4.4, 4.7, 4.9, 5.1, 5.2, 5.4, 5.5, 5.6, 5.7, 5.8, 5.9, 6.2, 6.4, 6.6, 6.8, 7.3, 9.5, 13.0, 17.0, 25, 40}; // pt
        bin_n = 22;
        bins.assign(bins_tmp, bins_tmp + bin_n + 1);
    }   
    if (quantity=="Probe_abs_eta"){
        double bins_tmp[] =  {0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8,}; // |eta|
        bin_n = 18;
        bins.assign(bins_tmp, bins_tmp + bin_n + 1);
    }    
    if (quantity=="Probe_eta"){
        double bins_tmp[] =  {-1.8,-1.6,-1.4,-1.2,-1.0,-0.8,-0.6,-0.4,-0.2, 0., 0.2, 0.4, 0.6, 0.8, 1., 1.2, 1.4,  1.6, 1.8 }; // |eta|
        bin_n = 18;
        bins.assign(bins_tmp, bins_tmp + bin_n + 1);
    }    
    if (quantity=="Mm_dR"){
        double bins_tmp[] =  { 0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8 }; 
        bin_n = 8;
        bins.assign(bins_tmp, bins_tmp + bin_n + 1);
    }    

    //Now we must choose initial conditions in order to fit our data
    double *init_conditions = new double[4];
    /*-----------------------------------I N S E R T    C O D E    H E R E-----------------------------------*/
    init_conditions[0] = 3.09809 ;
    init_conditions[1] = 0.05;
    /*------------------------------------------------------------------------------------------------------*/

    
    string* conditions = get_conditions(bin_n, bins, quantity,isBarrel,isEndcap);
    double ** yields_n_errs = new double*[bin_n];
    
    for (int i = 0; i < bin_n; i++)
    {
        cout<<"Processing bin "<< conditions[i] << "\n";
        if (DataIsMC){
            yields_n_errs[i] = McYield(paths["Data_"+dataset], conditions[i], MuonId, quantity,isBarrel,isEndcap);
            // cout<<yields_n_errs[i][0]<<"\n";
        }
        else{
            yields_n_errs[i] = doFit(paths["Data_"+dataset], conditions[i], MuonId, quantity, init_conditions, isBarrel,isEndcap);
            // cout<<yields_n_errs[i][0]<<"\n";
            //doFit returns: [yield_all, yield_pass, err_all, err_pass]
        }
    }

    TH1F *yield_ALL  = make_hist("ALL" , yields_n_errs, 0, bin_n, bins.data(), DataIsMC,false);
    TH1F *yield_PASS = make_hist("PASS", yields_n_errs, 1, bin_n, bins.data(), DataIsMC,false);

    cout<< "\n\n\n Made histograms, now check fit quality, then add to Eff file \n\n\n";

    //----------------------SAVING RESULTS TO Histograms.root--------------------//
    //useful if we require to change the fit on a specific set of bins
    // TFile* EfficiencyFile = TFile::Open("/work/submit/mori25/Darkphotons_ludo/tagnprobe/eff_Jpsi.root","RECREATE");
    // yield_ALL->SetDirectory(gDirectory);
    // yield_PASS->SetDirectory(gDirectory);
    // EfficiencyFile->Write();
    //-----------------------------------------------------------------//
    
    //If all of the fits seem correct we can proceed to generate the efficiency

    get_efficiency(paths["Eff_"+dataset], yield_ALL, yield_PASS, quantity, MuonId, DataIsMC,isBarrel,isEndcap);

    //In case you want to change the fit on a specific, comment the loop and "result saving" code and uncomment the following function
    //change_bin(/*bin number you want to redo*/, /*condition (you can copy the title from the generated fit .pdf)*/, MuonId, quantity, DataIsMC, init_conditions);
    //bins start on 1
}
