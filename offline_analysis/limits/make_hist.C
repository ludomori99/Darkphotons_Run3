#include "TFileCollection.h"
#include "TChain.h"
#include "TFile.h"
#include <TTreeReader.h>
#include "TH1D.h"
#include "TH2D.h"
#include <TTreeReaderValue.h>
#include "TLorentzVector.h"

using namespace std;

void make_hist(TString dump = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/dump_post_BDT/*.root", const char* outfilename = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/histograms/offline/output_histogram.root") {

    TFile* outfile = new TFile(outfilename, "RECREATE");
    TTree* outtree = new TTree("tree", "tree");
    
    int num_mass_regions = 278;
    float growth_factor = 0.005;
    float m = 2;

	double id_wp = 0.4;
	double mva_wp = 0.8;

    TH1F* massforLimitFull = new TH1F("massforLimitFull","massforLimitFull",1000,0., 8.);
    TH1D* massforLimit_CatA[num_mass_regions];
    TH1D* massforLimit_CatB[num_mass_regions];

    float m_tmp = m;
    for(int j=0; j<num_mass_regions; j++){
      m_tmp = m_tmp+(m_tmp*growth_factor);
      massforLimit_CatA[j] = new TH1D(Form("massforLimit_CatA%d",j),Form("massforLimit_CatA%d",j),100,m_tmp-(m_tmp*0.013*5.),m_tmp+(m_tmp*0.013*5.));  massforLimit_CatA[j]->Sumw2();
      massforLimit_CatB[j] = new TH1D(Form("massforLimit_CatB%d",j),Form("massforLimit_CatB%d",j),100,m_tmp-(m_tmp*0.013*5.),m_tmp+(m_tmp*0.013*5.));  massforLimit_CatB[j]->Sumw2();
    }

    TChain* chain = new TChain("tree");
    chain->Add(dump); 
    // TH1F *htotal = (TH1F*)chain->Get("htotal");    

    TTreeReader reader(chain);

    TTreeReaderValue<double>          mass  (reader, "Mm_mass"    );
    TTreeReaderValue<double>             HLT_DoubleMu4_3_LowMass  (reader, "HLT_DoubleMu4_3_LowMass"    );
    TTreeReaderValue<double>          forest_prompt_Jpsi_mva  (reader, "forest_prompt_Jpsi_mva"    );
    TTreeReaderValue<double>          soft1  (reader, "Muon_softMva1"    );
    TTreeReaderValue<double>          soft2  (reader, "Muon_softMva2"    );
    TTreeReaderValue<double>          m1eta  (reader, "Mm_mu1_eta"    );
    TTreeReaderValue<double>          m2eta (reader, "Mm_mu2_eta"    );

    int j=0;
    int count[2]={0};
    int weight=1;
    while(reader.Next()) {
        count[0]++;
        j++;
        if (j%1000000==0){
        cout << j << "events" <<endl;
        }

        if (*HLT_DoubleMu4_3_LowMass !=1) continue;
        if (*forest_prompt_Jpsi_mva <mva_wp) continue;
        if (*soft1 <id_wp || *soft2 <id_wp  ) continue;	

        count[1]++;

        massforLimitFull->Fill(*mass, weight);
        float maxEta=TMath::Max(abs(*m1eta),abs(*m2eta));
        
        m_tmp = m;
        for(int j=0; j<num_mass_regions; j++){
            m_tmp = m_tmp+(m_tmp*growth_factor);
            if(*mass > m_tmp-(m_tmp*0.013*5.) && *mass < m_tmp+(m_tmp*0.013*5.)) {
                //cout << "filling catAB "<< j<< " with *mass " << mass << endl;
                if(maxEta<0.9){ massforLimit_CatA[j]->Fill(*mass,weight); }
                if(maxEta>=0.9 && maxEta<1.9 ){ massforLimit_CatB[j]->Fill(*mass,weight); }
            }
        }

    }
    
    cout << "final showdown: \n " << "total count=" << count[0] << " selected count=" << count[1] << " efficiency= " << count[1]/count[0] << endl;

    outfile->cd();
    massforLimitFull->Write();
    for(int j=0; j<num_mass_regions;j++){
        massforLimit_CatA[j]->Write();
        massforLimit_CatB[j]->Write();
    }
    // htotal->Write();
    outfile->Close();
}