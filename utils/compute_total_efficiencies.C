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

    TString offline_dump="/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/dump_post_BDT/*.root";
    TString MC_dump="/data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/dump_large_post_BDT/*.root";

    // for now use the looser efficiency cuts
    float dimuon_mva=0.9445454545454546;
    float muonID=0.3836734693877551;

    float off_eff=compute_eff(offline_dump, dimuon_mva, muonID);
    float MC_eff=compute_eff(MC_dump, dimuon_mva, muonID);
    cout << "offline efficiency " << off_eff << "\n MC efficiency " << MC_eff << endl; 
}

float compute_eff(TString dump, float dimuon_mva, float muonID){
    //Take as input a folder containing .root files with the events. outputs the efficiency of the selection procedure

    TChain* chain = new TChain("tree");
    chain->Add(dump); 

    TTreeReader reader(chain);
    TTreeReaderValue<double>          mass  (reader, "Mm_mass"    );
    TTreeReaderValue<double>          forest_prompt_mva  (reader, "forest_prompt_mva"    );
    TTreeReaderValue<double>          soft1  (reader, "Muon_softMva1"    );
    TTreeReaderValue<double>          soft2  (reader, "Muon_softMva2"    );
    TTreeReaderValue<double>          m1eta  (reader, "Mm_mu1_eta"    );
    TTreeReaderValue<double>          m2eta (reader, "Mm_mu2_eta"    );

    int j=0;
    int count[2]={0};
    while(reader.Next()) {
        count[0]++;
	    j++;
	    if (j%1000000==0){
	        cout << j << "events" <<endl;
	    }
	    if (*forest_prompt_mva <dimuon_mva) continue;
	    if (*soft1 < muonID || *soft2 < muonID ) continue;	
        count[1]++;
    }

    float efficiency = float(count[1])/count[0];
    
    cout << "final showdown: \n " << "total count=" << count[0] << " selected count=" << count[1] << " efficiency= " << efficiency << endl;
    return efficiency;
}