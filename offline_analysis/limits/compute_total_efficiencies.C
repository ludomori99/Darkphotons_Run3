#include "TFileCollection.h"
#include "TChain.h"
#include "TFile.h"
#include <TTreeReader.h>
#include "TH1D.h"
#include "TH2D.h"
#include <TTreeReaderValue.h>
#include "TLorentzVector.h"
#include "compute_eff.C"

using namespace std;

void compute_total_efficiencies(){

    // TString offline="/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/dump_post_BDT/*.root";
    TString offline="/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/dump_post_BDT/*.root";
    TString MC_InclusiveMinBiasJpsi="/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/dump_post_BDT/DimuonTreeJpsi*_BDT.root";
    TString MC_InclusiveMinBiasY1="/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/dump_post_BDT/DimuonTreeY1*_BDT.root";
    TString MC_lmDY="/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/dump_post_BDT/DimuonTree*_BDT.root";

    string particle="Jpsi";

    // for now use the looser efficiency cuts
    float dimuon_mva=0.873;
    float muonID=.426;

    double* massLimits = new double[2];
    string trigger_str="";
    
    if(particle=="Jpsi"){
        massLimits[0] = 3.0886;
        massLimits[1] = 3.0986;
        trigger_str="HLT_DoubleMu4_3_LowMass";
    }
    else if(particle=="Upsilon"){
        massLimits[0] = 9.41;
        massLimits[1] = 9.51;
        trigger_str="HLT_Dimuon10_Upsilon_y1p4"
    }
    else{
        cout << "Error: particle not recognized" << endl;
        return;
    }
    
    // float off_eff=compute_eff(offline, dimuon_mva, muonID, massLimits,true);
    // float MinBias_effJpsi=compute_eff(MC_InclusiveMinBiasJpsi, dimuon_mva, muonID,massLimits,false);

    // float lmDY_eff=compute_eff(MC_lmDY, trigger_str, dimuon_mva, muonID);
    // cout << "offline Jpsi" << off_eff <<"\n MinBias Jpsi efficiency : " << MinBias_effJpsi  << endl; 
    
}
