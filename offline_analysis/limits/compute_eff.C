#include "TFileCollection.h"
#include "TChain.h"
#include "TFile.h"
#include <TTreeReader.h>
#include "TH1D.h"
#include "TH2D.h"
#include <TTreeReaderValue.h>
#include "TLorentzVector.h"

using namespace std;


float compute_eff(TString dump, string trigger_str, float dimuon_mva, float muonID, double* massLimits,bool apply_massLimits = false){
    //Take as input a folder containing .root files with the events. outputs the efficiency of the selection procedure

    TChain* chain = new TChain("tree");
    chain->Add(dump); 

    TTreeReader reader(chain);
    TTreeReaderValue<double>          mass  (reader, "Mm_mass"    );
    TTreeReaderValue<double>          forest_prompt_mva  (reader, "forest_prompt_Jpsi_mva"    );
    TTreeReaderValue<double>          soft1  (reader, "Muon_softMva1"    );
    TTreeReaderValue<double>          soft2  (reader, "Muon_softMva2"    );
    TTreeReaderValue<double>             trigger  (reader, trigger_str.c_str()    );

    int j=0;
    int count[2]={0};
    while(reader.Next()) {
        if (apply_massLimits && (*mass < massLimits[0] || *mass > massLimits[1])) continue;

        count[0]++;
	    j++;
	    if (j%1000000==0){
	        cout << "\r" << j << " events done" <<endl;
            cout.flush();
	    }
        if (*trigger != 1) continue;
	    if (*forest_prompt_mva < dimuon_mva) continue;
	    if (*soft1 < muonID || *soft2 < muonID ) continue;	
        count[1]++;
    }

    float efficiency = float(count[1])/count[0];
    
    cout << "final showdown: \n " << "total count=" << count[0] << " selected count=" << count[1] << " efficiency= " << efficiency << endl;
    return efficiency;
}