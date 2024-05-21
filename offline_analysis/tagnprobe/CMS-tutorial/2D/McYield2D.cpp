
double* McYield2D(const char* filepath, string condition, string MuonID_str, string quantx, string quanty)
{
    TFile *file0    = TFile::Open(filepath);
    TTree *DataTree = (TTree*)file0->Get(("tree"));
    
    double _mmin = 2.6;  double _mmax = 3.56;

    RooRealVar MuonID(MuonID_str.c_str(), MuonID_str.c_str(), 0, 1); //Muon_Id
    RooRealVar isBarrelMuon("isBarrelMuon","isBarrelMuon",0,1);
    RooRealVar Mm_mass("Mm_mass", "Mm_mass", _mmin, _mmax);
    RooRealVar trigger("HLT_DoubleMu4_3_LowMass", "HLT_DoubleMu4_3_LowMass", 0,1);


    RooRealVar xquantity((quantx).c_str(), (quantx).c_str(),-100,100);
    RooRealVar yquantity((quanty).c_str(), (quanty).c_str(),-100,100);
    
    RooFormulaVar* reduce = new RooFormulaVar("PPTM", condition.c_str(), RooArgList(trigger, xquantity,yquantity));
    RooFormulaVar* cutvar = new RooFormulaVar("PPTM", (condition + "&&" + MuonID_str + "==1").c_str() , RooArgList(trigger,xquantity,yquantity,MuonID));
    RooDataSet *Data_ALL    = new RooDataSet("DATA", "DATA", DataTree,RooArgSet(trigger,xquantity,yquantity,MuonID,Mm_mass),*reduce);
    RooDataSet *Data_PASSING = new RooDataSet("DATA_PASS", "DATA_PASS", DataTree, RooArgSet(trigger,xquantity,yquantity,MuonID,Mm_mass), *cutvar);//
    double* output = new double[4];
    output[0] = Data_ALL->sumEntries();
    output[1] = Data_PASSING->sumEntries();
    output[2] = std::sqrt(output[0]);
    output[3] = std::sqrt(output[1]);
    cout<<condition<<":\n"<<"all: "<<output[0]<<"\npass: "<<output[1];
    return output;
}

