#include "src/compare_efficiency.C"
#include "src/DoFit.cpp"
#include "src/get_conditions.cpp"
#include "src/get_efficiency.cpp"
#include "src/change_bin.cpp"
#include "src/make_hist.cpp"
#include "src/McYield.cpp"

void compute_eff(bool, string, string, bool=false, bool=false);

void Efficiency(){

    string MuonId = "PassingProbeSoftId";

    // compute_eff(true,"Probe_pt",MuonId);
    // compute_eff(false,"Probe_pt",MuonId);
    // compare_efficiency("Probe_pt", 
    //     (string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/Probe_pt/Efficiency_MC.root")).c_str(), 
    //     (string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/Probe_pt/Efficiency_Run3.root")).c_str(),
    //     MuonId);

    compute_eff(true,"Probe_eta",MuonId);
    compute_eff(false,"Probe_eta",MuonId);
    compare_efficiency("Probe_eta", 
        (string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/Probe_eta/Efficiency_MC.root")).c_str(), 
        (string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/Probe_eta/Efficiency_Run3.root")).c_str(),
         MuonId);
    
    // compute_eff(true,"Probe_abs_eta",MuonId);
    // compute_eff(false,"Probe_abs_eta",MuonId);
    // compare_efficiency("Probe_abs_eta", 
    //     (string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/Probe_abs_eta/Efficiency_MC.root")).c_str(), 
    //     (string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/Probe_abs_eta/Efficiency_Run3.root")).c_str());

    // compute_eff(true,"Mm_dR",MuonId);
    // compute_eff(true,"Mm_dR",MuonId, true,false);
    // compute_eff(true,"Mm_dR",MuonId, false,true);
    // compute_eff(false,"Mm_dR",MuonId);
    // compute_eff(false,"Mm_dR",MuonId, true,false);
    // compute_eff(false,"Mm_dR",MuonId,false, true);

    // compare_efficiency("Mm_dR", 
    //     (string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/Mm_dR/Efficiency_MC.root")).c_str(), 
    //     (string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/Mm_dR/Efficiency_Run3.root")).c_str(),
    //     MuonId, false); 
    // compare_efficiency("Mm_dR", 
    //     (string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/Mm_dR/Efficiency_MC.root")).c_str(), 
    //     (string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/Mm_dR/Efficiency_Run3.root")).c_str(),
    //     MuonId, true); 

}

void compute_eff(bool DataIsMC, string quantity, string MuonId, bool isBarrel=false, bool isEndcap=false){

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
        double bins_tmp[] =  { 0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8 }; // |eta|
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
            yields_n_errs[i] = McYield(conditions[i], MuonId, quantity,isBarrel,isEndcap);
            // cout<<yields_n_errs[i][0]<<"\n";
        }
        else{
            yields_n_errs[i] = doFit(conditions[i], MuonId, quantity, init_conditions, isBarrel,isEndcap);
            cout<<yields_n_errs[i][0]<<"\n";
            //doFit returns: [yield_all, yield_pass, err_all, err_pass]
        }
    }

    TH1F *yield_ALL  = make_hist("ALL" , yields_n_errs, 0, bin_n, bins.data(), DataIsMC,false);
    TH1F *yield_PASS = make_hist("PASS", yields_n_errs, 1, bin_n, bins.data(), DataIsMC,false);

    cout<< "\n\n\n Made histograms, now add to Eff file \n\n\n";
    //----------------------SAVING RESULTS TO Histograms.root--------------------//
    //useful if we require to change the fit on a specific set of bins
    // TFile* EfficiencyFile = TFile::Open("/work/submit/mori25/Darkphotons_ludo/tagnprobe/eff_Jpsi.root","RECREATE");
    // yield_ALL->SetDirectory(gDirectory);
    // yield_PASS->SetDirectory(gDirectory);
    // EfficiencyFile->Write();
    //-----------------------------------------------------------------//
    
    //If all of the fits seem correct we can proceed to generate the efficiency
    get_efficiency(yield_ALL, yield_PASS, quantity, MuonId, DataIsMC,isBarrel,isEndcap);


    //In case you want to change the fit on a specific, comment the loop and "result saving" code and uncomment the following function
    //change_bin(/*bin number you want to redo*/, /*condition (you can copy the title from the generated fit .pdf)*/, MuonId, quantity, DataIsMC, init_conditions);
    //bins start on 1
}
