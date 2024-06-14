#include <TFile.h>
#include <TH1.h>
#include <TKey.h>
#include <TClass.h>

void add_histos() {

    const char* inputFileName = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/histograms/offline/output_histogram_full_bkp.root";
    const char* outFileName = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/histograms/offline/output_histogram_full.root";
    // Open the input file
    TFile *inputFile = TFile::Open(inputFileName, "READ");
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error opening input file: " << inputFileName << std::endl;
        return;
    }
    TFile *outputFile = TFile::Open(outFileName, "RECREATE");
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error opening output file: " << outFileName << std::endl;
        return;
    }

    TH1 *histFull = (TH1*)inputFile->Get("massforLimitFull");
    outputFile->cd();
    histFull->Write();
    
    int N = 139; 

    for (int n = 0; n <N ; ++n) {
        std::ostringstream histAName, histBName, histCName;
        histAName << "massforLimit_CatA" << n;
        histBName << "massforLimit_CatB" << n;
        histCName << "massforLimit_CatC" << n;

        // Get histograms A and B
        TH1 *histA = (TH1*)inputFile->Get(histAName.str().c_str());
        TH1 *histB = (TH1*)inputFile->Get(histBName.str().c_str());

        if (!histA || !histB) {
            std::cerr << "Error: Histogram " << histAName.str() << " or " << histBName.str() << " not found in the input file." << std::endl;
            continue;
        }

        // Clone histA to create histC
        TH1 *histC = (TH1*)histA->Clone(histCName.str().c_str());
        histC->SetDirectory(0);
        histC->SetTitle(histCName.str().c_str());
        histC->Add(histB);

        // Write histC to the output file
        histA->Write();
        histB->Write();
        histC->Write();

        // Clean up
        delete histC;
    }
    
    inputFile->Close();
    delete inputFile;

}
