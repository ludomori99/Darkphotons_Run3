#include "TFileCollection.h"
#include "TChain.h"
#include "TFile.h"
#include <TTreeReader.h>
#include "TH1D.h"
#include "TH2D.h"
#include <TTreeReaderValue.h>
#include "TLorentzVector.h"

using namespace std;

float compute_eff(TString dump, float dimuon_mva, float muonID);

void compute_total_efficiencies(){

    TString offline="/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Y/merged_A.root";
    TString MC="/data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/Y/merged_A.root";

    // for now use the looser efficiency cuts
    float dimuon_mva=0.9445454545454546;
    float muonID=-3; //0.3836734693877551;

    // float off_eff=compute_eff(offline, dimuon_mva, muonID);
    float MC_eff=compute_eff(MC, dimuon_mva, muonID);
    cout << "offline efficiency " << MC_eff << "\n MC efficiency ";// << MC_eff << endl; 
    
    // cout << "Now look at ID only" << endl;
    // float off_eff=compute_eff(offline_dump, 0, muonID);
    // float MC_eff=compute_eff(MC_dump, 0, muonID);
    // cout << "offline efficiency " << off_eff << "\n MC efficiency " << MC_eff << endl; 
}

float compute_eff(TString dump, float dimuon_mva, float muonID){
    //Take as input a folder containing .root files with the events. outputs the efficiency of the selection procedure

    TChain* chain = new TChain("tree");
    chain->Add(dump); 

    TTreeReader reader(chain);
    TTreeReaderValue<float>          mass  (reader, "Mm_mass"    );
    TTreeReaderValue<float>          forest_prompt_mva  (reader, "forest_prompt_Jpsi_mva"    );
    TTreeReaderValue<float>          soft1  (reader, "Muon_softMva1"    );
    TTreeReaderValue<float>          soft2  (reader, "Muon_softMva2"    );

    int j=0;
    int count[2]={0};
    while(reader.Next()) {
        count[0]++;
	    j++;
	    if (j%1000000==0){
	        cout << "\r" << j << " events done" <<endl;
            cout.flush();
	    }
	    if (*forest_prompt_mva <dimuon_mva) continue;
	    if (*soft1 < muonID || *soft2 < muonID ) continue;	
        count[1]++;
    }

    float efficiency = float(count[1])/count[0];
    
    cout << "final showdown: \n " << "total count=" << count[0] << " selected count=" << count[1] << " efficiency= " << efficiency << endl;
    return efficiency;
}