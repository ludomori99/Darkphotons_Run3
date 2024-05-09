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

    TString offline="/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/dump_post_BDT/*.root";
    TString MC_InclusiveMinBias="/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/dump_post_BDT/DimuonTree*_BDT.root";
    TString MC_lmDY="/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/dump_post_BDT/DimuonTree*_BDT.root";

    string particle="Jpsi";

    // for now use the looser efficiency cuts
    float dimuon_mva=0.857;
    float muonID=.4;

    double* massLimits = new double[2];
    
    if(particle=="Jpsi"){
        massLimits[0] = 2.6;
        massLimits[1] = 3.56;
    }
    else if(particle=="Upsilon"){
        massLimits[0] = 8.5;
        massLimits[1] = 11.2;
    }
    else{
        cout << "Error: particle not recognized" << endl;
        return;
    }
    
    float off_eff=compute_eff(offline, dimuon_mva, muonID);
    float MC_eff=compute_eff(MC, dimuon_mva, muonID);
    cout << MC_eff << "\n MC efficiency ";// << MC_eff << endl; 
    
    // cout << "Now look at ID only" << endl;
    // float off_eff=compute_eff(offline_dump, 0, muonID);
    // float MC_eff=compute_eff(MC_dump, 0, muonID);
    // cout << "offline efficiency " << off_eff << "\n MC efficiency " << MC_eff << endl; 
}

float compute_eff(TString dump,  float dimuon_mva, float muonID, double* massLimits){
    //Take as input a folder containing .root files with the events. outputs the efficiency of the selection procedure

    TChain* chain = new TChain("tree");
    chain->Add(dump); 

    TTreeReader reader(chain);
    TTreeReaderValue<double>          mass  (reader, "Mm_mass"    );
    TTreeReaderValue<double>          forest_prompt_mva  (reader, "forest_prompt_mva"    );
    TTreeReaderValue<double>          soft1  (reader, "Muon_softMva1"    );
    TTreeReaderValue<double>          soft2  (reader, "Muon_softMva2"    );
.
    int j=0;
    int count[2]={0};
    while(reader.Next()) {
        count[0]++;
	    j++;
	    if (j%1000000==0){
	        cout << "\r" << j << " events done" <<endl;
            cout.flush();
	    }
        if (*mass < massLimits[0] || *mass > massLimits[1]) continue;
	    if (*forest_prompt_mva <dimuon_mva) continue;
	    if (*soft1 < muonID || *soft2 < muonID ) continue;	
        count[1]++;
    }

    float efficiency = float(count[1])/count[0];
    
    cout << "final showdown: \n " << "total count=" << count[0] << " selected count=" << count[1] << " efficiency= " << efficiency << endl;
    return efficiency;
}