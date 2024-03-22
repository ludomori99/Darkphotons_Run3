
double* McYield(string condition, string quant, string MuonID_str = "PassingProbeSoftId")
{
    TFile *file0    = TFile::Open("/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Jpsi/TP_samples_Jpsi.root");
    TTree *DataTree = (TTree*)file0->Get(("tree"));
    
    double _mmin = 2.6;  double _mmax = 3.56;

    RooRealVar MuonID(MuonID_str.c_str(), MuonID_str.c_str(), 0, 1); //Muon_Id
    
    RooRealVar Mm_mass("Mm_mass", "Mm_mass", _mmin, _mmax);

    double* limits = new double[2];
    if (quant == "Probe_pt") {
        limits[0] = 0;
        limits[1] = 40;
    }
    if (quant == "Probe_eta") {
        limits[0] = -3;
        limits[1] = 3;
    }

    RooRealVar quantity((quant).c_str(), (quant).c_str(), limits[0], limits[1]);
    
    RooFormulaVar* redeuce = new RooFormulaVar("PPTM", condition.c_str(), RooArgList(quantity));
    RooDataSet *Data_ALL    = new RooDataSet("DATA", "DATA", DataTree, RooArgSet(quantity,MuonID,Mm_mass),*redeuce);
    RooFormulaVar* cutvar = new RooFormulaVar("PPTM", (condition + " && " + MuonID_str + " == 1").c_str() , RooArgList(quantity,MuonID));

    RooDataSet *Data_PASSING = new RooDataSet("DATA_PASS", "DATA_PASS", DataTree, RooArgSet(quantity, MuonID, Mm_mass), *cutvar);//
    
    double* output = new double[2];
    output[0] = Data_ALL->sumEntries();
    output[1] = Data_PASSING->sumEntries();
    return output;
}

