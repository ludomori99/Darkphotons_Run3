
double* McYield(const char* data_filepath, string condition, string MuonID_str, string quant,bool isBarrel, bool isEndcap)
{
    TFile *file0    = TFile::Open(data_filepath);
    if (!file0 || file0->IsZombie()) {
        cout << "Error: Could not open file " << data_filepath << endl;
    }
    TTree *DataTree = (TTree*)file0->Get(("tree"));
    
    double _mmin = 2.6;  double _mmax = 3.56;

    RooRealVar MuonID(MuonID_str.c_str(), MuonID_str.c_str(), 0, 1); //Muon_Id
    RooRealVar isBarrelMuon("isBarrelMuon","isBarrelMuon",0,1);
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
    if (quant == "Probe_abs_eta") {
        limits[0] = 0;
        limits[1] = 3;
    }
    if (quant == "Mm_dR") {
        limits[0] = 0;
        limits[1] = 2;
    }

    RooRealVar quantity((quant).c_str(), (quant).c_str(), limits[0], limits[1]);
    
    RooFormulaVar* reduce = new RooFormulaVar("PPTM", condition.c_str(), ((isBarrel||isEndcap) ? RooArgList(isBarrelMuon,quantity) : RooArgList(quantity)));
    RooFormulaVar* cutvar = new RooFormulaVar("PPTM", (condition + "&&" + MuonID_str + "==1").c_str() , ((isBarrel||isEndcap) ? RooArgList(isBarrelMuon,quantity,MuonID) : RooArgList(quantity,MuonID)));

    RooDataSet *Data_ALL    = new RooDataSet("DATA", "DATA", DataTree, ((isBarrel||isEndcap) ? RooArgSet(isBarrelMuon,quantity,MuonID,Mm_mass) : RooArgSet(quantity,MuonID,Mm_mass)),*reduce);
    RooDataSet *Data_PASSING = new RooDataSet("DATA_PASS", "DATA_PASS", DataTree, ((isBarrel||isEndcap) ? RooArgSet(isBarrelMuon,quantity,MuonID,Mm_mass) : RooArgSet(quantity,MuonID,Mm_mass)), *cutvar);//
    
    double* output = new double[2];
    output[0] = Data_ALL->sumEntries();
    output[1] = Data_PASSING->sumEntries();

    cout<<condition<<":\n"<<"all: "<<output[0]<<"\npass: "<<output[1];
    return output;
}

