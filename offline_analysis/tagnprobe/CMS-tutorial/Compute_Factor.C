#include <unordered_map>
using namespace std;

void Compute_Factor(string quantity_str,bool isBarrel=false, bool isEndcap=false){

    string MuonId = "PassingProbeSoftId";
    
    unordered_map<string, const char*> paths;
    paths["Data_MinBias"] = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Jpsi/TP_samples_Jpsi.root";
    paths["Data_Offline"] = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Jpsi/TP_samples_Jpsi.root";

    string Eff_MinBias_str = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/") + quantity_str + string("/Efficiency_MinBias.root");
    paths["Eff_MinBias"] = (Eff_MinBias_str).c_str();
    string Eff_Offline_str = string("/data/submit/mori25/dark_photons_ludo/DimuonTrees/tagnprobe/") + MuonId + string("/") + quantity_str + string("/Efficiency_Run3.root");
    paths["Eff_Offline"] = (Eff_Offline_str).c_str();
    
    //need to check this is the right approach! 
    paths["Data_lmDY"] = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/DP/TP_samples_DP.root";

    TFile* off_file = TFile::Open(paths["Eff_Offline"]);
    TFile* MinBias_file = TFile::Open(paths["Eff_MinBias"]);
    TFile* lmDY_file = TFile::Open(paths["Data_lmDY"]);

    // Get the TEfficiency objects
    const char* effName = "Efficiency";
    TEfficiency* eff_off = (TEfficiency*)off_file->Get(effName);
    TEfficiency* eff_MinBias = (TEfficiency*)MinBias_file->Get(effName);

    // Check that the TEfficiency objects exist
    if (!eff_off || !eff_MinBias) {
        cout << "Error: TEfficiency objects not found" << endl;
        return;
    }
    
    // Get the TTree
    TTree* tree = (TTree*)lmDY_file->Get("tree");


    // Define the cut CHECK THIS
    string massCut = "(true || (Mm_mass>1.1 && Mm_mass<2.6) || (Mm_mass>4.2 && Mm_mass<7.9)) &&";

    vector<double> bins; 
    int bin_n;
    double* limits = new double[2];

    if (quantity_str=="Probe_pt"){
        double bins_tmp[] =  {4, 4.2, 4.4, 4.7, 4.9, 5.1, 5.2, 5.4, 5.5, 5.6, 5.7, 5.8, 5.9, 6.2, 6.4, 6.6, 6.8, 7.3, 9.5, 13.0, 17.0, 25, 40}; // pt
        bin_n = 22;
        bins.assign(bins_tmp, bins_tmp + bin_n + 1);
        limits[0] = 0;
        limits[1] = 100;
    }   
    if (quantity_str=="Probe_abs_eta"){
        double bins_tmp[] =  {0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8,}; // |eta|
        bin_n = 18;
        bins.assign(bins_tmp, bins_tmp + bin_n + 1);
        limits[0] = 0;
        limits[1] = 3;
    }    
    if (quantity_str=="Probe_eta"){
        double bins_tmp[] =  {-1.8,-1.6,-1.4,-1.2,-1.0,-0.8,-0.6,-0.4,-0.2, 0., 0.2, 0.4, 0.6, 0.8, 1., 1.2, 1.4,  1.6, 1.8 }; // |eta|
        bin_n = 18;
        bins.assign(bins_tmp, bins_tmp + bin_n + 1);
        limits[0] = -3;
        limits[1] = 3;
    }    
    if (quantity_str=="Mm_dR"){
        double bins_tmp[] =  { 0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8 }; 
        bin_n = 8;
        bins.assign(bins_tmp, bins_tmp + bin_n + 1);
        limits[0] = 0;
        limits[1] = 2;
    }    


    string* conditions = new string[bin_n];
    string barrelOrEndcap = "";
    if (isBarrel) barrelOrEndcap = "isBarrelMuon==1&&";
    else if (isEndcap) barrelOrEndcap = "isBarrelMuon==0&&";

    string trigger_str = "HLT_DoubleMu4_3_LowMass==1&&";


    for (int i = 0; i < bin_n; i++){
        conditions[i] = massCut + barrelOrEndcap + quantity_str + ">" + to_string(bins[i]).substr(0,4) + "&&" + quantity_str + "<" + to_string(bins[i+1]).substr(0,4);
    }

    RooRealVar quantity((quantity_str).c_str(), (quantity_str).c_str(), limits[0], limits[1]);
    RooRealVar isBarrelMuon("isBarrelMuon","isBarrelMuon",0,1);
    RooRealVar Mm_mass("Mm_mass", "Mm_mass", 1.1, 7.9);
    RooRealVar trigger("HLT_DoubleMu4_3_LowMass", "HLT_DoubleMu4_3_LowMass", 0,1);

    // Multiply the number of events in each bin by a scale factor and add them up
    double kin_integral_sf = 0.0;
    double total_events = 0.0;
    double kin_integral_sf_unc_A = 0.0;
    double kin_integral_sf_unc_B_up = 0.0;
    double kin_integral_sf_unc_B_low = 0.0;
    double kin_integral_sf_unc_C = 0.0;
    
    for (int i = 0; i < bin_n; i++) {
        string condition = trigger_str + massCut + barrelOrEndcap + quantity_str + ">" + to_string(bins[i]).substr(0,4) + "&&" + quantity_str + "<" + to_string(bins[i+1]).substr(0,4);
        RooFormulaVar* reduce = new RooFormulaVar("PPTM", condition.c_str(), ((isBarrel||isEndcap) ? RooArgList(trigger,Mm_mass,isBarrelMuon,quantity) : RooArgList(trigger,Mm_mass,quantity)));
        RooDataSet *Data_bin  = new RooDataSet("DATA", "DATA", tree, ((isBarrel||isEndcap) ? RooArgSet(trigger,Mm_mass,isBarrelMuon,quantity) : RooArgSet(trigger,Mm_mass,quantity)),*reduce);
        double bin_entries = Data_bin->sumEntries();
        double scale_factor = eff_off->GetEfficiency(i+1)/eff_MinBias->GetEfficiency(i+1);
        double scale_factor_unc_A = abs(eff_off->GetEfficiency(i+1)-eff_MinBias->GetEfficiency(i+1)); // just efficiency difference
        double scale_factor_unc_B_up = scale_factor*sqrt(pow(eff_off->GetEfficiencyErrorUp(i+1)/eff_off->GetEfficiency(i+1),2) + pow(eff_MinBias->GetEfficiencyErrorUp(i+1)/eff_MinBias->GetEfficiency(i+1),2)); // integrated CP intervals (asymm.) 
        double scale_factor_unc_B_low = scale_factor*sqrt(pow(eff_off->GetEfficiencyErrorLow(i+1)/eff_off->GetEfficiency(i+1),2) + pow(eff_MinBias->GetEfficiencyErrorLow(i+1)/eff_MinBias->GetEfficiency(i+1),2));

        double eff_rel_unc_off = (1-eff_off->GetEfficiency(i+1))/eff_off->GetTotalHistogram()->GetBinContent(i+1)/eff_off->GetEfficiency(i+1); //squared relative unc.
        double eff_rel_unc_MinBias = (1-eff_MinBias->GetEfficiency(i+1))/eff_MinBias->GetTotalHistogram()->GetBinContent(i+1)/eff_MinBias->GetEfficiency(i+1);
        double scale_factor_unc_C = scale_factor*sqrt(eff_rel_unc_off + eff_rel_unc_MinBias); // binomial error
        // cout<< i <<" scale factor : "<< scale_factor<< "+/-" << scale_factor_unc << "% \n" << "bin entries : "<<bin_entries<<"\n";
        kin_integral_sf += bin_entries * scale_factor;
        kin_integral_sf_unc_A += bin_entries * scale_factor_unc_A;
        kin_integral_sf_unc_B_up += bin_entries * scale_factor_unc_B_up;
        kin_integral_sf_unc_B_low += bin_entries * scale_factor_unc_B_low;
        kin_integral_sf_unc_C += bin_entries * scale_factor_unc_C;
        total_events += bin_entries;
    }

    // Use the sum for further calculations or output
    cout << "total events : " << total_events<< "\nFinal SF: " << kin_integral_sf/total_events << "\nErrors:\nSimple efficiency difference: " << kin_integral_sf_unc_A/total_events ; 
    cout << "\nClopper-Pearson, upper: " << kin_integral_sf_unc_B_up/total_events << "\nClopper-Pearson, lower: " << kin_integral_sf_unc_B_low/total_events;
    cout << "\nNormal binomial " <<  kin_integral_sf_unc_C/total_events <<  endl;

    return;
}
