#include "TFileCollection.h"
#include "TChain.h"
#include "TFile.h"
#include <TTreeReader.h>
#include "TH1D.h"
#include "TH2D.h"
#include <TTreeReaderValue.h>
#include "TLorentzVector.h"

using namespace std;


void generateOffDimuonTree(TString inputfilename, const char* outfilename, int event_fraction = 10) {

    TFile* outfile = new TFile(outfilename, "RECREATE");
    TTree* outtree = new TTree("tree","tree");

    TFile *inputfile = TFile::Open(inputfilename);
    TTreeReader reader("Events", inputfile);
    TTreeReaderValue<int>          nmm (reader, "nmm");

    TTreeReaderArray<float>          mpt  (reader, "Muon_pt"    );
    TTreeReaderArray<float>          meta  (reader, "Muon_eta"    );
    TTreeReaderArray<int>          mcharge (reader, "Muon_charge"    );
    TTreeReaderArray<float>          Dxy (reader, "Muon_dxy" );
    TTreeReaderArray<float>          Dz (reader, "Muon_dz" );    

    TTreeReaderValue<int>   nMuon  (reader, "nMuon" );
    TTreeReaderValue<unsigned int>   Run  (reader, "run" );
    TTreeReaderValue<unsigned int>   LumSec (reader, "luminosityBlock" );
    TTreeReaderValue<unsigned long long>   event (reader, "event");
    TTreeReaderValue<int>            nMuonId (reader, "nMuonId");

    //triggers
    TTreeReaderValue<bool>            hlt_DoubleMu4_3_LowMass (reader, "HLT_DoubleMu4_3_LowMass");
    TTreeReaderValue<bool>            hlt_DoubleMu4_3_Jpsi (reader, "HLT_DoubleMu4_3_Jpsi");
    TTreeReaderValue<bool>            hlt_Dimuon10_Upsilon_y1p4 (reader, "HLT_Dimuon10_Upsilon_y1p4");
    TTreeReaderValue<bool>            hlt_Dimuon12_Upsilon_y1p4 (reader, "HLT_Dimuon12_Upsilon_y1p4");
    TTreeReaderArray<int>            MuonId_hlt_DoubleMu4_3_LowMass (reader, "MuonId_HLT_DoubleMu4_3_LowMass");
    TTreeReaderValue<double>            prescale_HLT_DoubleMu4_3_LowMass (reader, "prescale_HLT_DoubleMu4_3_LowMass");

TTreeReaderValue<bool>    l1_DoubleMu0er2p0_SQ_OS_dEta_Max1p6(reader,"L1_DoubleMu0er2p0_SQ_OS_dEta_Max1p6");
TTreeReaderValue<bool> l1_DoubleMu0er2p0_SQ_OS_dEta_Max1p5(reader,"L1_DoubleMu0er2p0_SQ_OS_dEta_Max1p5");
TTreeReaderValue<bool> l1_DoubleMu4_SQ_OS_dR_Max1p2(reader,"L1_DoubleMu4_SQ_OS_dR_Max1p2"); 
TTreeReaderValue<bool> l1_DoubleMu4p5_SQ_OS_dR_Max1p2(reader,"L1_DoubleMu4p5_SQ_OS_dR_Max1p2"); 
TTreeReaderValue<bool> l1_DoubleMu0er1p5_SQ_OS_dR_Max1p4(reader,"L1_DoubleMu0er1p5_SQ_OS_dR_Max1p4"); 
TTreeReaderValue<bool> l1_DoubleMu0er1p4_SQ_OS_dR_Max1p4(reader,"L1_DoubleMu0er1p4_SQ_OS_dR_Max1p4");

    // TTreeReaderArray<float>          muonId_chi2LocalPosition (reader, "MuonId_chi2LocalPosition");
    // TTreeReaderArray<float>          muonId_glbNormChi2 (reader, "MuonId_glbNormChi2");
    // TTreeReaderArray<float>          muonId_glbTrackProbability (reader, "MuonId_glbTrackProbability");
    // TTreeReaderArray<float>          muonId_match1_dX (reader, "MuonId_match1_dX");
    // TTreeReaderArray<float>          muonId_match1_dY (reader, "MuonId_match1_dY");
    // TTreeReaderArray<float>          muonId_match1_pullDxDz (reader, "MuonId_match1_pullDxDz");
    // TTreeReaderArray<float>          muonId_match1_pullDyDz (reader, "MuonId_match1_pullDyDz");
    // TTreeReaderArray<float>          muonId_match1_pullX (reader, "MuonId_match1_pullX");
    // TTreeReaderArray<float>          muonId_match1_pullY (reader, "MuonId_match1_pullY");
    // TTreeReaderArray<float>          muonId_match2_dX (reader, "MuonId_match2_dX");
    // TTreeReaderArray<float>          muonId_match2_dY (reader, "MuonId_match2_dY");
    // TTreeReaderArray<float>          muonId_match2_pullDxDz (reader, "MuonId_match2_pullDxDz");
    // TTreeReaderArray<float>          muonId_match2_pullDyDz (reader, "MuonId_match2_pullDyDz");
    // TTreeReaderArray<float>          muonId_match2_pullX (reader, "MuonId_match2_pullX");
    // TTreeReaderArray<float>          muonId_match2_pullY (reader, "MuonId_match2_pullY");
    // TTreeReaderArray<float>          muonId_trkKink (reader, "MuonId_trkKink");
    // TTreeReaderArray<float>          muonId_trkValidFrac (reader, "MuonId_trkValidFrac");
    // TTreeReaderArray<int>          muonId_nLostHitsInner (reader, "MuonId_nLostHitsInner");
    // TTreeReaderArray<int>          muonId_nLostHitsOn (reader, "MuonId_nLostHitsOn");
    // TTreeReaderArray<int>          muonId_nLostHitsOuter (reader, "MuonId_nLostHitsOuter");
    // TTreeReaderArray<int>          muonId_nPixels (reader, "MuonId_nPixels");
    // TTreeReaderArray<int>          muonId_nValidHits (reader, "MuonId_nValidHits");
    
    TTreeReaderArray<float>          mm_docatrk (reader, "mm_docatrk");
    TTreeReaderArray<float>          mm_iso (reader, "mm_iso");
    TTreeReaderArray<float>          mm_kin_alpha (reader, "mm_kin_alpha");
    TTreeReaderArray<float>          mm_kin_alphaBS (reader, "mm_kin_alphaBS");
    TTreeReaderArray<float>          mm_kin_l3d (reader, "mm_kin_l3d");
    TTreeReaderArray<float>          mm_kin_lxy (reader, "mm_kin_lxy");
    TTreeReaderArray<float>          mm_kin_pt (reader, "mm_kin_pt");
    TTreeReaderArray<float>          mm_kin_mass (reader, "mm_kin_mass");
    TTreeReaderArray<float>          mm_kin_eta (reader, "mm_kin_eta");
    TTreeReaderArray<float>          mm_kin_pvip (reader, "mm_kin_pvip");
    TTreeReaderArray<float>          mm_kin_pvlip (reader, "mm_kin_pvlip");
    TTreeReaderArray<float>          mm_kin_sl3d (reader, "mm_kin_sl3d");
    TTreeReaderArray<float>          mm_kin_slxy (reader, "mm_kin_slxy"); 
    TTreeReaderArray<float>          mm_kin_spvip (reader, "mm_kin_spvip");
    TTreeReaderArray<float>          mm_kin_vtx_chi2dof (reader, "mm_kin_vtx_chi2dof");
    TTreeReaderArray<float>          mm_kin_vtx_prob (reader, "mm_kin_vtx_prob");
    TTreeReaderArray<float>          mm_m1iso (reader, "mm_m1iso");
    TTreeReaderArray<float>          mm_m2iso (reader, "mm_m2iso");
    TTreeReaderArray<float>          mm_mass (reader, "mm_mass");
    TTreeReaderArray<float>          mm_mva (reader, "mm_mva");
    TTreeReaderArray<float>          mm_otherVtxMaxProb (reader, "mm_otherVtxMaxProb");
    TTreeReaderArray<float>          mm_otherVtxMaxProb1 (reader, "mm_otherVtxMaxProb1");
    TTreeReaderArray<float>          mm_otherVtxMaxProb2 (reader, "mm_otherVtxMaxProb2");
    TTreeReaderArray<int>          mm_closetrk (reader, "mm_closetrk");
    TTreeReaderArray<int>          mm_closetrks1 (reader, "mm_closetrks1");
    TTreeReaderArray<int>          mm_closetrks2 (reader, "mm_closetrks2");
    TTreeReaderArray<int>          mm_closetrks3 (reader, "mm_closetrks3");
    TTreeReaderArray<int>          mm_nDisTrks (reader, "mm_nDisTrks");


    TTreeReaderArray<float>          mm_mu1_pt (reader, "mm_mu1_pt");
    TTreeReaderArray<float>          mm_mu1_eta (reader, "mm_mu1_eta");
    TTreeReaderArray<float>          mm_mu2_pt (reader, "mm_mu2_pt");
    TTreeReaderArray<float>          mm_mu2_eta (reader, "mm_mu2_eta");
    TTreeReaderArray<float>          mm_mu1_phi (reader, "mm_mu1_phi");
    TTreeReaderArray<float>          mm_mu2_phi (reader, "mm_mu2_phi");

    TTreeReaderArray<int>           mm_mu1_index (reader, "mm_mu1_index");
    TTreeReaderArray<int>           mm_mu2_index (reader, "mm_mu2_index");
    TTreeReaderArray<int>           mm_kin_valid (reader, "mm_kin_valid");

    TTreeReaderArray<bool>          muon_looseId (reader, "Muon_looseId");
    TTreeReaderArray<bool>          muon_mediumId (reader, "Muon_mediumId");
    TTreeReaderArray<bool>          muon_mediumPromptId (reader, "Muon_mediumPromptId");
    TTreeReaderArray<bool>          muon_isGlobal (reader, "Muon_isGlobal");
    TTreeReaderArray<bool>          muon_softMvaId (reader, "Muon_softMvaId");
    TTreeReaderArray<bool>          muon_softId (reader, "Muon_softId");
    TTreeReaderArray<bool>          muon_isStandalone (reader, "Muon_isStandalone");
    TTreeReaderArray<bool>          muon_tightId (reader, "Muon_tightId");
    TTreeReaderArray<bool>          muon_triggerIdLoose (reader, "Muon_triggerIdLoose");
    TTreeReaderArray<bool>          muon_isTracker (reader, "Muon_isTracker");

    TTreeReaderArray<float>          muon_softMva (reader, "Muon_softMva");

    unsigned run; 
    unsigned luminosityBlock;
    unsigned long long Event;
    unsigned int NMuonId;

    bool HLT_Dimuon0_LowMass;
    bool HLT_DoubleMu4_3_LowMass;
    bool HLT_DoubleMu4_3_Jpsi;
    bool HLT_Dimuon10_Upsilon_y1p4;
    bool HLT_Dimuon12_Upsilon_y1p4;
    int MuonId_HLT_DoubleMu4_3_LowMass1;
    int MuonId_HLT_DoubleMu4_3_LowMass2;
    double Prescale_HLT_DoubleMu4_3_LowMass;

bool L1_DoubleMu0er2p0_SQ_OS_dEta_Max1p6;
bool L1_DoubleMu0er2p0_SQ_OS_dEta_Max1p5;
bool L1_DoubleMu4_SQ_OS_dR_Max1p2; 
bool L1_DoubleMu4p5_SQ_OS_dR_Max1p2; 
bool L1_DoubleMu0er1p5_SQ_OS_dR_Max1p4; 
bool L1_DoubleMu0er1p4_SQ_OS_dR_Max1p4;

    // vector<float> MuonId_chi2LocalPosition;
    // vector<float> MuonId_glbNormChi2;
    // vector<float> MuonId_glbTrackProbability;
    // vector<float> MuonId_match1_dX;
    // vector<float> MuonId_match1_dY;
    // vector<float> MuonId_match1_pullDxDz;
    // vector<float> MuonId_match1_pullDyDz;
    // vector<float> MuonId_match1_pullX;
    // vector<float> MuonId_match1_pullY;
    // vector<float> MuonId_match2_dX;
    // vector<float> MuonId_match2_dY;
    // vector<float> MuonId_match2_pullDxDz;
    // vector<float> MuonId_match2_pullDyDz;
    // vector<float> MuonId_match2_pullX;
    // vector<float> MuonId_match2_pullY;
    // vector<float> MuonId_trkKink;
    // vector<float> MuonId_trkValidFrac;
    // vector<int> MuonId_nLostHitsInner;
    // vector<int> MuonId_nLostHitsOn;
    // vector<int> MuonId_nLostHitsOuter;
    // vector<int> MuonId_nPixels;
    // vector<int> MuonId_nValidHits;

    float Mm_docatrk;
    float Mm_iso;
    float Mm_kin_alpha;
    float Mm_kin_alphaBS;
    float Mm_kin_l3d;
    float Mm_kin_lxy;
    float Mm_kin_pt;
    float Mm_kin_mass;
    float Mm_kin_eta;
    float Mm_kin_pvip;
    float Mm_kin_pvlip;
    float Mm_kin_sl3d;
    float Mm_kin_slxy;
    float Mm_kin_spvip;
    float Mm_kin_vtx_chi2dof;
    float Mm_kin_vtx_prob;
    float Mm_m1iso;
    float Mm_m2iso;
    float Mm_mass;
    float Mm_mva;
    float Mm_otherVtxMaxProb;
    float Mm_otherVtxMaxProb1;
    float Mm_otherVtxMaxProb2;
    int Mm_closetrk;
    int Mm_closetrks1;
    int Mm_closetrks2;
    int Mm_closetrks3;
    int Mm_kin_valid;
    int Mm_nDisTrks;
    
    float Mm_mu1_pt;
    float Mm_mu1_eta;
    float Mm_mu1_phi;
    float Mm_mu2_pt;
    float Mm_mu2_eta;
    float Mm_mu2_phi;

    float Muon_softMva1;
    float Muon_softMva2;
    float Muon_Dxy1;
    float Muon_Dxy2;
    float Muon_Dz1;
    float Muon_Dz2;
    float Muon_charge1;
    float Muon_charge2;

    bool Muon_looseId1;
    bool Muon_looseId2;
    bool Muon_mediumId1;
    bool Muon_mediumId2;
    bool Muon_mediumPromptId1;
    bool Muon_mediumPromptId2;
    bool Muon_isGlobal1;
    bool Muon_isGlobal2;
    bool Muon_softMvaId1;
    bool Muon_softMvaId2;
    bool Muon_softId1;
    bool Muon_softId2;
    bool Muon_isStandalone1;
    bool Muon_isStandalone2;
    bool Muon_tightId1;
    bool Muon_tightId2;
    bool Muon_triggerIdLoose1;
    bool Muon_triggerIdLoose2;
    bool Muon_isTracker1;
    bool Muon_isTracker2;


    outtree->Branch("run"   , &run   , "run/i");
    outtree->Branch("lumSec", &luminosityBlock);
    outtree->Branch("Event", &Event);
    outtree->Branch("nMuonId", &NMuonId);


    outtree->Branch("HLT_Dimuon0_LowMass",&HLT_Dimuon0_LowMass, 32000,0);
    outtree->Branch("HLT_DoubleMu4_3_LowMass", &HLT_DoubleMu4_3_LowMass);
    outtree->Branch("HLT_DoubleMu4_3_Jpsi", &HLT_DoubleMu4_3_Jpsi);
    outtree->Branch("HLT_Dimuon10_Upsilon_y1p4", &HLT_Dimuon10_Upsilon_y1p4);
    outtree->Branch("HLT_Dimuon12_Upsilon_y1p4", &HLT_Dimuon12_Upsilon_y1p4);
    outtree->Branch("MuonId_HLT_DoubleMu4_3_LowMass1", &MuonId_HLT_DoubleMu4_3_LowMass1);
    outtree->Branch("MuonId_HLT_DoubleMu4_3_LowMass2", &MuonId_HLT_DoubleMu4_3_LowMass2);
    outtree->Branch("Prescale_HLT_DoubleMu4_3_LowMass", &Prescale_HLT_DoubleMu4_3_LowMass);

    outtree->Branch("L1_DoubleMu0er2p0_SQ_OS_dEta_Max1p6", &L1_DoubleMu0er2p0_SQ_OS_dEta_Max1p6);
    outtree->Branch("L1_DoubleMu0er2p0_SQ_OS_dEta_Max1p5", &L1_DoubleMu0er2p0_SQ_OS_dEta_Max1p5);
    outtree->Branch("L1_DoubleMu4_SQ_OS_dR_Max1p2", &L1_DoubleMu4_SQ_OS_dR_Max1p2);
    outtree->Branch("L1_DoubleMu4p5_SQ_OS_dR_Max1p2", &L1_DoubleMu4p5_SQ_OS_dR_Max1p2);
    outtree->Branch("L1_DoubleMu0er1p5_SQ_OS_dR_Max1p4", &L1_DoubleMu0er1p5_SQ_OS_dR_Max1p4);
    outtree->Branch("L1_DoubleMu0er1p4_SQ_OS_dR_Max1p4", &L1_DoubleMu0er1p4_SQ_OS_dR_Max1p4);
    

    // outtree->Branch("MuonId_chi2LocalPosition",&MuonId_chi2LocalPosition,32000,0);
    // outtree->Branch("MuonId_glbNormChi2",&MuonId_glbNormChi2,32000,0);
    // outtree->Branch("MuonId_glbTrackProbability",&MuonId_glbTrackProbability,32000,0);
    // outtree->Branch("MuonId_match1_dX",&MuonId_match1_dX,32000,0);
    // outtree->Branch("MuonId_match1_dY",&MuonId_match1_dY,32000,0);
    // outtree->Branch("MuonId_match1_pullDxDz",&MuonId_match1_pullDxDz,32000,0);
    // outtree->Branch("MuonId_match1_pullDyDz",&MuonId_match1_pullDyDz,32000,0);
    // outtree->Branch("MuonId_match1_pullX",&MuonId_match1_pullX,32000,0);
    // outtree->Branch("MuonId_match1_pullY",&MuonId_match1_pullY,32000,0);
    // outtree->Branch("MuonId_match2_dX",&MuonId_match2_dX,32000,0);
    // outtree->Branch("MuonId_match2_dY",&MuonId_match2_dY,32000,0);
    // outtree->Branch("MuonId_match2_pullDxDz",&MuonId_match2_pullDxDz,32000,0);
    // outtree->Branch("MuonId_match2_pullDyDz",&MuonId_match2_pullDyDz,32000,0);
    // outtree->Branch("MuonId_match2_pullX",&MuonId_match2_pullX,32000,0);
    // outtree->Branch("MuonId_match2_pullY",&MuonId_match2_pullY,32000,0);
    // outtree->Branch("MuonId_trkKink",&MuonId_trkKink,32000,0);
    // outtree->Branch("MuonId_trkValidFrac",&MuonId_trkValidFrac,32000,0);
    // outtree->Branch("MuonId_nLostHitsInner",&MuonId_nLostHitsInner,32000,0);
    // outtree->Branch("MuonId_nLostHitsOn",&MuonId_nLostHitsOn,32000,0);
    // outtree->Branch("MuonId_nLostHitsOuter",&MuonId_nLostHitsOuter,32000,0);
    // outtree->Branch("MuonId_nPixels",&MuonId_nPixels,32000,0);
    // outtree->Branch("MuonId_nValidHits",&MuonId_nValidHits,32000,0);
    
    outtree->Branch("Mm_docatrk",&Mm_docatrk,32000,0);
    outtree->Branch("Mm_iso",&Mm_iso,32000,0);
    outtree->Branch("Mm_kin_alpha",&Mm_kin_alpha,32000,0);
    outtree->Branch("Mm_kin_alphaBS",&Mm_kin_alphaBS,32000,0);
    outtree->Branch("Mm_kin_l3d",&Mm_kin_l3d,32000,0);
    outtree->Branch("Mm_kin_lxy",&Mm_kin_lxy,32000,0);
    outtree->Branch("Mm_kin_pt",&Mm_kin_pt,32000,0);
    outtree->Branch("Mm_kin_mass",&Mm_kin_mass,32000,0);
    outtree->Branch("Mm_kin_eta",&Mm_kin_eta,32000,0);
    outtree->Branch("Mm_kin_pvip",&Mm_kin_pvip,32000,0);
    outtree->Branch("Mm_kin_pvlip",&Mm_kin_pvlip,32000,0);
    outtree->Branch("Mm_kin_sl3d",&Mm_kin_sl3d,32000,0);
    outtree->Branch("Mm_kin_slxy",&Mm_kin_slxy,32000,0);
    outtree->Branch("Mm_kin_spvip",&Mm_kin_spvip,32000,0);
    outtree->Branch("Mm_kin_vtx_chi2dof",&Mm_kin_vtx_chi2dof,32000,0);
    outtree->Branch("Mm_kin_vtx_prob",&Mm_kin_vtx_prob,32000,0);
    outtree->Branch("Mm_m1iso",&Mm_m1iso,32000,0);
    outtree->Branch("Mm_m2iso",&Mm_m2iso,32000,0);
    outtree->Branch("Mm_mass",&Mm_mass,32000,0);
    outtree->Branch("Mm_mva",&Mm_mva,32000,0);
    outtree->Branch("Mm_otherVtxMaxProb",&Mm_otherVtxMaxProb,32000,0);
    outtree->Branch("Mm_otherVtxMaxProb1",&Mm_otherVtxMaxProb1,32000,0);
    outtree->Branch("Mm_otherVtxMaxProb2",&Mm_otherVtxMaxProb2,32000,0);
    outtree->Branch("Mm_closetrk",&Mm_closetrk,32000,0);
    outtree->Branch("Mm_closetrks1",&Mm_closetrks1,32000,0);
    outtree->Branch("Mm_closetrks2",&Mm_closetrks2,32000,0);
    outtree->Branch("Mm_closetrks3",&Mm_closetrks3,32000,0);
    outtree->Branch("Mm_kin_valid",&Mm_kin_valid,32000,0);
    outtree->Branch("Mm_nDisTrks",&Mm_nDisTrks,32000,0);


    outtree->Branch("Mm_mu1_pt",&Mm_mu1_pt,32000,0);
    outtree->Branch("Mm_mu1_eta",&Mm_mu1_eta,32000,0);
    outtree->Branch("Mm_mu1_phi",&Mm_mu1_phi,32000,0);
    outtree->Branch("Mm_mu2_pt",&Mm_mu2_pt,32000,0);
    outtree->Branch("Mm_mu2_eta",&Mm_mu2_eta,32000,0);
    outtree->Branch("Mm_mu2_phi",&Mm_mu2_phi,32000,0);


    outtree->Branch("Muon_softMva1",&Muon_softMva1,32000,0);
    outtree->Branch("Muon_softMva2",&Muon_softMva2,32000,0);
    outtree->Branch("Muon_charge1",&Muon_charge1,32000,0);
    outtree->Branch("Muon_charge2",&Muon_charge2,32000,0);
    outtree->Branch("Muon_Dxy1",&Muon_Dxy1,32000,0);
    outtree->Branch("Muon_Dxy2",&Muon_Dxy2,32000,0);
    outtree->Branch("Muon_Dz1",&Muon_Dz1,32000,0);
    outtree->Branch("Muon_Dz2",&Muon_Dz2,32000,0);


    
    outtree->Branch("Muon_looseId1",&Muon_looseId1,32000,0);
    outtree->Branch("Muon_looseId2",&Muon_looseId2,32000,0);
    outtree->Branch("Muon_mediumId1",&Muon_mediumId1,32000,0);
    outtree->Branch("Muon_mediumId2",&Muon_mediumId2,32000,0);
    outtree->Branch("Muon_mediumPromptId1",&Muon_mediumPromptId1,32000,0);
    outtree->Branch("Muon_mediumPromptId2",&Muon_mediumPromptId2,32000,0);
    outtree->Branch("Muon_isGlobal1",&Muon_isGlobal1,32000,0);
    outtree->Branch("Muon_isGlobal2",&Muon_isGlobal2,32000,0);
    outtree->Branch("Muon_softMvaId1",&Muon_softMvaId1,32000,0);
    outtree->Branch("Muon_softMvaId2",&Muon_softMvaId2,32000,0);
    outtree->Branch("Muon_softId1",&Muon_softId1,32000,0);
    outtree->Branch("Muon_softId2",&Muon_softId2,32000,0);
    outtree->Branch("Muon_isStandalone1",&Muon_isStandalone1,32000,0);
    outtree->Branch("Muon_isStandalone2",&Muon_isStandalone2,32000,0);
    outtree->Branch("Muon_tightId1",&Muon_tightId1,32000,0);
    outtree->Branch("Muon_tightId2",&Muon_tightId2,32000,0);
    outtree->Branch("Muon_triggerIdLoose1",&Muon_triggerIdLoose1,32000,0);
    outtree->Branch("Muon_triggerIdLoose2",&Muon_triggerIdLoose2,32000,0);
    outtree->Branch("Muon_isTracker1",&Muon_isTracker1,32000,0);
    outtree->Branch("Muon_isTracker2",&Muon_isTracker2,32000,0);

    int counter=0;

    while(reader.Next()) {
        counter+=1;
        if (counter%10000==0) cout << counter << " events parsed" << endl;
        if (counter%event_fraction!=0) continue;

        if (*nMuon<2) continue;
        // MuonId_chi2LocalPosition.clear();
        // MuonId_glbNormChi2.clear();
        // MuonId_glbTrackProbability.clear();
        // MuonId_match1_dX.clear();
        // MuonId_match1_dY.clear();
        // MuonId_match1_pullDxDz.clear();
        // MuonId_match1_pullDyDz.clear();
        // MuonId_match1_pullX.clear();
        // MuonId_match1_pullY.clear();
        // MuonId_match2_dX.clear();
        // MuonId_match2_dY.clear();
        // MuonId_match2_pullDxDz.clear();
        // MuonId_match2_pullDyDz.clear();
        // MuonId_match2_pullX.clear();
        // MuonId_match2_pullY.clear();
        // MuonId_trkKink.clear();
        // MuonId_trkValidFrac.clear();
        // MuonId_nLostHitsInner.clear();
        // MuonId_nLostHitsOn.clear();
        // MuonId_nLostHitsOuter.clear();
        // MuonId_nPixels.clear();
        // MuonId_nValidHits.clear();

        vector<unsigned> goodmuons;
        
        for (int i = 0; i < *nMuon; i++) {
            
            if(mpt[i]<4 || abs(meta[i])>1.9) continue;

            bool i_is_dimuon = false;
            for (int j = 0; j < end(mm_mu1_index) - begin(mm_mu1_index); j++){
                if (mm_mu1_index[j] == i || mm_mu2_index[j] == i) {
                    i_is_dimuon = true;
                    break;
                }
            }
            //muons in nMuon : 
            // (pt > 2 || (pt > 2 && (passed('CutBasedIdLoose') || passed('SoftCutBasedId') || passed('SoftMvaId') || passed('CutBasedIdGlobalHighPt') || passed('CutBasedIdTrkHighPt'))))
            // equivalent to just pt>2
            if (i_is_dimuon) goodmuons.push_back(i);
        }
        
        
        if (goodmuons.size() < 2 || *nmm < 1) continue;

        unsigned idx1 = goodmuons[0];
        for(int i = 0; i<goodmuons.size(); i++){
            unsigned k = goodmuons[i];
            if (mpt[k] > mpt[idx1]) {
                idx1 = k;
            }
        }//Now idx1 is the highest pt muon among goodmuons
        
        unsigned idx2 = 99;
        unsigned mm_idx;
        for(int i = 0; i < *nmm; i++){
            unsigned i1 = mm_mu1_index[i];
            unsigned i2 = mm_mu2_index[i];
            if (i1 == idx1 && (idx2 == 99 || mpt[i2] > mpt[idx2])) {
                idx2 = i2;
                mm_idx = i;
            }
            else if (i2 == idx1 && (idx2 == 99 || mpt[i1] > mpt[idx2])) {
                idx2 = i1;
                mm_idx = i;
            }
        }// now idx2 is the highest pt muon among those paired with idx1 in a dimuon

        if(mpt[idx2]<4 || abs(meta[idx2])>1.9 || mcharge[idx1]*mcharge[idx2] ==1) continue;
        
        Muon_softMva1 = muon_softMva[idx1];
        Muon_softMva2 = muon_softMva[idx2];
        Muon_charge1 = mcharge[idx1];
        Muon_charge2 = mcharge[idx2];
        Muon_Dxy1 = Dxy[idx1];
        Muon_Dxy2 = Dxy[idx2];
        Muon_Dz1 = Dz[idx1];
        Muon_Dz2 = Dz[idx2];
        Muon_looseId1 = muon_looseId[idx1];
        Muon_looseId2 = muon_looseId[idx2];
        Muon_mediumId1 = muon_mediumId[idx1];
        Muon_mediumId2 = muon_mediumId[idx2];
        Muon_mediumPromptId1 = muon_mediumPromptId[idx1];
        Muon_mediumPromptId2 = muon_mediumPromptId[idx2]; 
        Muon_isGlobal1 = muon_isGlobal[idx1];
        Muon_isGlobal2 = muon_isGlobal[idx2];
        Muon_softMvaId1 = muon_softMvaId[idx1];
        Muon_softMvaId2 = muon_softMvaId[idx2];  
        Muon_softId1 = muon_softId[idx1];
        Muon_softId2 = muon_softId[idx2];  
        Muon_isStandalone1 = muon_isStandalone[idx1];
        Muon_isStandalone2 = muon_isStandalone[idx2];  
        Muon_tightId1 = muon_tightId[idx1];
        Muon_tightId2 = muon_tightId[idx2];  
        Muon_triggerIdLoose1 = muon_triggerIdLoose[idx1];
        Muon_triggerIdLoose2 = muon_triggerIdLoose[idx2]; 
        Muon_isTracker1 = muon_isTracker[idx1];
        Muon_isTracker2 = muon_isTracker[idx2];

        MuonId_HLT_DoubleMu4_3_LowMass1 = MuonId_hlt_DoubleMu4_3_LowMass[idx1];
        MuonId_HLT_DoubleMu4_3_LowMass2 = MuonId_hlt_DoubleMu4_3_LowMass[idx2];


        // for (int j=0; j<2; j++) {
        //     int i = idx1;
        //     if (j == 1){i = idx2;}

            // MuonId_chi2LocalPosition.push_back((muonId_chi2LocalPosition)[i]);
            // MuonId_glbNormChi2.push_back((muonId_glbNormChi2)[i]);
            // MuonId_glbTrackProbability.push_back((muonId_glbTrackProbability)[i]);
            // MuonId_match1_dX.push_back((muonId_match1_dX)[i]);
            // MuonId_match1_dY.push_back((muonId_match1_dY)[i]);
            // MuonId_match1_pullDxDz.push_back((muonId_match1_pullDxDz)[i]);
            // MuonId_match1_pullDyDz.push_back((muonId_match1_pullDyDz)[i]);
            // MuonId_match1_pullX.push_back((muonId_match1_pullX)[i]);
            // MuonId_match1_pullY.push_back((muonId_match1_pullY)[i]);
            // MuonId_match2_dX.push_back((muonId_match2_dX)[i]);
            // MuonId_match2_dY.push_back((muonId_match2_dY)[i]);
            // MuonId_match2_pullDxDz.push_back((muonId_match2_pullDxDz)[i]);
            // MuonId_match2_pullDyDz.push_back((muonId_match2_pullDyDz)[i]);
            // MuonId_match2_pullX.push_back((muonId_match2_pullX)[i]);
            // MuonId_match2_pullY.push_back((muonId_match2_pullY)[i]);
            // MuonId_trkKink.push_back((muonId_trkKink)[i]);
            // MuonId_trkValidFrac.push_back((muonId_trkValidFrac)[i]);
            // MuonId_nLostHitsInner.push_back((muonId_nLostHitsInner)[i]);
            // MuonId_nLostHitsOn.push_back((muonId_nLostHitsOn)[i]);
            // MuonId_nLostHitsOuter.push_back((muonId_nLostHitsOuter)[i]);
            // MuonId_nPixels.push_back((muonId_nPixels)[i]);
            // MuonId_nValidHits.push_back((muonId_nValidHits)[i]);

        // }
        
        Mm_docatrk = mm_docatrk[mm_idx];
        Mm_iso = mm_iso[mm_idx];
        Mm_kin_alpha = mm_kin_alpha[mm_idx];
        Mm_kin_alphaBS = mm_kin_alphaBS[mm_idx];
        Mm_kin_l3d = mm_kin_l3d[mm_idx];
        Mm_kin_lxy = mm_kin_lxy[mm_idx];
        Mm_kin_pt = mm_kin_pt[mm_idx];
        Mm_kin_pvip = mm_kin_pvip[mm_idx];
        Mm_kin_pvlip = mm_kin_pvlip[mm_idx];
        Mm_kin_sl3d = mm_kin_sl3d[mm_idx];
        Mm_kin_slxy = mm_kin_slxy[mm_idx];
        Mm_kin_spvip = mm_kin_spvip[mm_idx];
        Mm_kin_vtx_chi2dof = mm_kin_vtx_chi2dof[mm_idx];
        Mm_kin_vtx_prob = mm_kin_vtx_prob[mm_idx];
        Mm_m1iso = mm_m1iso[mm_idx];
        Mm_m2iso = mm_m2iso[mm_idx];
        Mm_mass = mm_mass[mm_idx];
        Mm_mva = mm_mva[mm_idx];
        Mm_otherVtxMaxProb = mm_otherVtxMaxProb[mm_idx];
        Mm_otherVtxMaxProb1 = mm_otherVtxMaxProb1[mm_idx];
        Mm_otherVtxMaxProb2 = mm_otherVtxMaxProb2[mm_idx];
        Mm_closetrk = mm_closetrk[mm_idx];
        Mm_closetrks1 = mm_closetrks1[mm_idx];
        Mm_closetrks2 = mm_closetrks2[mm_idx];
        Mm_closetrks3 = mm_closetrks3[mm_idx];
        Mm_nDisTrks = mm_nDisTrks[mm_idx];


        Mm_mu1_pt = mm_mu1_pt[mm_idx];
        Mm_mu1_eta = mm_mu1_eta[mm_idx];
        Mm_mu2_pt = mm_mu2_pt[mm_idx];
        Mm_mu2_eta = mm_mu2_eta[mm_idx];
        Mm_kin_eta = mm_kin_eta[mm_idx];

        run = *Run;
        luminosityBlock = *LumSec;
        Event = *event;
        NMuonId = *nMuonId;

        HLT_DoubleMu4_3_LowMass = *hlt_DoubleMu4_3_LowMass;
        HLT_DoubleMu4_3_Jpsi = *hlt_DoubleMu4_3_Jpsi;
        HLT_Dimuon10_Upsilon_y1p4 = *hlt_Dimuon10_Upsilon_y1p4;
        HLT_Dimuon12_Upsilon_y1p4 = *hlt_Dimuon12_Upsilon_y1p4;
        Prescale_HLT_DoubleMu4_3_LowMass = *prescale_HLT_DoubleMu4_3_LowMass;

L1_DoubleMu0er2p0_SQ_OS_dEta_Max1p6 = *l1_DoubleMu0er2p0_SQ_OS_dEta_Max1p6;
L1_DoubleMu0er2p0_SQ_OS_dEta_Max1p5 = *l1_DoubleMu0er2p0_SQ_OS_dEta_Max1p5;
L1_DoubleMu4_SQ_OS_dR_Max1p2 = *l1_DoubleMu4_SQ_OS_dR_Max1p2;
L1_DoubleMu4p5_SQ_OS_dR_Max1p2 = *l1_DoubleMu4p5_SQ_OS_dR_Max1p2;
L1_DoubleMu0er1p5_SQ_OS_dR_Max1p4 = *l1_DoubleMu0er1p5_SQ_OS_dR_Max1p4;
L1_DoubleMu0er1p4_SQ_OS_dR_Max1p4 = *l1_DoubleMu0er1p4_SQ_OS_dR_Max1p4;

        
	outtree->Fill(); 
   }
     
   outfile->cd();
   outtree->Write();
   outfile->Close();
}


