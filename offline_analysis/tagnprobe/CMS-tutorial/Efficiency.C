#include "src/compare_efficiency.C"
#include "src/DoFit.cpp"
#include "src/get_conditions.cpp"
#include "src/get_efficiency.cpp"
#include "src/change_bin.cpp"
#include "src/make_hist.cpp"
#include "src/McYield.cpp"

void compute_eff(bool,string);

void Efficiency(){
    // compute_eff(true,"Probe_pt");
    // compute_eff(false,"Probe_pt");
    compute_eff(true,"Probe_eta");
    // compute_eff(false,"Probe_eta");
    // compare_efficiency("Probe_pt", 
    //     "/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/Probe_pt/Efficiency_MC.root", 
    //     "/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/Probe_pt/Efficiency_Run3.root");

    // compare_efficiency("Probe_eta", 
    //     "/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/Probe_eta/Efficiency_MC.root", 
    //     "/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/Probe_eta/Efficiency_Run3.root");

}

void compute_eff(bool DataIsMC,string quantity){

    string MuonId   = "PassingProbeSoftId";
    vector<double> bins; 
    int bin_n;

    if (quantity=="Probe_pt"){
        double bins_tmp[] =  {4, 4.2, 4.4, 4.7, 4.9, 5.1, 5.2, 5.4, 5.5, 5.6, 5.7, 5.8, 5.9, 6.2, 6.4, 6.6, 6.8, 7.3, 9.5, 13.0, 17.0, 25, 40}; // pt
        bin_n = 23;
        bins.assign(bins_tmp, bins_tmp + bin_n + 1);
    }   
    if (quantity=="Probe_eta"){
        double bins_tmp[] =  {0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2. }; // |eta|
        bin_n = 19;
        bins.assign(bins_tmp, bins_tmp + bin_n + 1);
    }    
    if (quantity=="Probe_abs_eta"){
        double bins_tmp[] =  {-2.,-1.8,-1.6,-1.4,-1.2,-1.0,-0.8,-0.6,-0.4,-0.2, 0., 0.1, 0.2, 0.4, 0.6, 0.8, 1., 1.2, 1.4,  1.6, 1.8, 2. }; // |eta|
        bin_n = 19;
        bins.assign(bins_tmp, bins_tmp + bin_n + 1);
    }    
    
    //Now we must choose initial conditions in order to fit our data
    double *init_conditions = new double[4];
    /*-----------------------------------I N S E R T    C O D E    H E R E-----------------------------------*/
    init_conditions[0] = 3.09809 ;
    init_conditions[1] = 0.05;
    /*------------------------------------------------------------------------------------------------------*/
    
    
    string* conditions = get_conditions(bin_n, bins, quantity);
    double ** yields_n_errs = new double*[bin_n];
    
    for (int i = 0; i < bin_n; i++)
    {
        cout<<"Processing bin "<< conditions[i] << "\n";
        if (DataIsMC){
            yields_n_errs[i] = McYield(conditions[i], quantity);
            cout<<yields_n_errs[i][0]<<"\n";
        }
        else{
            yields_n_errs[i] = doFit(conditions[i], MuonId, quantity, init_conditions);
            cout<<yields_n_errs[i][0]<<"\n";
            //doFit returns: [yield_all, yield_pass, err_all, err_pass]
        }
    }
    cout<<"checkpoint 1\n\n\n";

    TH1F *yield_ALL  = make_hist("ALL" , yields_n_errs, 0, bin_n, bins.data(), DataIsMC);
    TH1F *yield_PASS = make_hist("PASS", yields_n_errs, 1, bin_n, bins.data(), DataIsMC);

            cout<<"checkpoint 2\n\n\n";

    //----------------------SAVING RESULTS TO Histograms.root--------------------//
    //useful if we require to change the fit on a specific set of bins
    // TFile* EfficiencyFile = TFile::Open("/work/submit/mori25/Darkphotons_ludo/tagnprobe/eff_Jpsi.root","RECREATE");
    // yield_ALL->SetDirectory(gDirectory);
    // yield_PASS->SetDirectory(gDirectory);
    // EfficiencyFile->Write();
    //-----------------------------------------------------------------//
    
    //If all of the fits seem correct we can proceed to generate the efficiency
    get_efficiency(yield_ALL, yield_PASS, quantity, DataIsMC);

            cout<<"checkpoint 3\n\n\n";


    //In case you want to change the fit on a specific, comment the loop and "result saving" code and uncomment the following function
    //change_bin(/*bin number you want to redo*/, /*condition (you can copy the title from the generated fit .pdf)*/, MuonId, quantity, DataIsMC, init_conditions);
    //bins start on 1
}
