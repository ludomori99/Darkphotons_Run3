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
    
    int num_mass_regions = 470;
    float m = 1;

    TH1F* massforLimitFull = new TH1F("massforLimitFull","massforLimitFull",4000,0., 120.);
    TH1F* massforLimitUpsilon = new TH1F("massforLimitUpsilon", "massforLimitUpsilon", 100 , 8.5, 11.4);
    TH1D* massforLimit_CatA[num_mass_regions];
    TH1D* massforLimit_CatB[num_mass_regions];

    for(int j=0; j<num_mass_regions; j++){
      m = m+(m*0.01);
      massforLimit_CatA[j] = new TH1D(Form("massforLimit_CatA%d",j),Form("massforLimit_CatA%d",j),100,m-(m*0.013*5.),m+(m*0.013*5.));  massforLimit_CatA[j]->Sumw2();
      massforLimit_CatB[j] = new TH1D(Form("massforLimit_CatB%d",j),Form("massforLimit_CatB%d",j),100,m-(m*0.013*5.),m+(m*0.013*5.));  massforLimit_CatB[j]->Sumw2();
    }

    TChain* chain = new TChain("tree");
    chain->Add(dump); 
    // TH1F *htotal = (TH1F*)chain->Get("htotal");    

    TTreeReader reader(chain);

    TTreeReaderValue<double>          mass  (reader, "Mm_mass"    );
    TTreeReaderValue<double>          forest_prompt_mva  (reader, "forest_prompt_mva"    );
    TTreeReaderValue<double>          soft1  (reader, "Muon_softMva1"    );
    TTreeReaderValue<double>          soft2  (reader, "Muon_softMva2"    );
    TTreeReaderValue<double>          m1eta  (reader, "Mm_mu1_eta"    );
    TTreeReaderValue<double>          m2eta (reader, "Mm_mu2_eta"    );

    int j=0;
    int count[4]={0};
    int weight=1;
    while(reader.Next()) {
        count[0]++;
	j++;
	if (j%1000000==0){
	   cout << j << "events" <<endl;
	}


	if (*forest_prompt_mva <0.9445454545454546) continue;
	if (*soft1 < 0.3836734693877551 || *soft2 < 0.3836734693877551  ) continue;	

        count[2]++;
        count[3]++;

        massforLimitFull->Fill(*mass, weight);
	if (*mass>=8.5 && *mass<=11.2) {
	    massforLimitUpsilon->Fill(*mass, weight);
	}	

	float maxEta=TMath::Max(abs(*m1eta),abs(*m2eta));
	
	float ma=1;
        for(int j=0; j<num_mass_regions; j++){
            ma = ma+(ma*0.01);
            if(*mass > ma-(ma*0.013*5.) && *mass < ma+(ma*0.013*5.)) {
		//cout << "filling catAB "<< j<< " with *mass " << mass << endl;
		if(maxEta<0.9){ massforLimit_CatA[j]->Fill(*mass,weight); }
		if(maxEta>=0.9 && maxEta<1.9 ){ massforLimit_CatB[j]->Fill(*mass,weight); }
	    }
	}

    }
    
    cout << "final showdown " << "count1=" << count[0] << " count2=" << count[1] << " count3=" << count[2] << " count4=" << count[3] << endl;

    outfile->cd();
    massforLimitFull->Write();
    massforLimitUpsilon->Write();
    for(int j=0; j<num_mass_regions;j++){
        massforLimit_CatA[j]->Write();
        massforLimit_CatB[j]->Write();
    }
    // htotal->Write();
    outfile->Close();
}