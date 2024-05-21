import awkward as ak
import uproot as up
import numpy as np
import os
import sys
import random 

from config_loader import load_analysis_config, load_test_config

test_or_depl = sys.argv[1] if len(sys.argv)==2 else 'test'
if test_or_depl == 'test': 
    print("loading test config in mass skim")
    config = load_test_config()
elif test_or_depl == 'depl' : 
    print("loading depl config in mass skim")
    config = load_analysis_config()
else:
    print("Error: no configuration available")

trigger = "dimuon" #or inclusive



def write_tree_data(particle,MC=False,reduction_factor = 0.05):

    #particle = "Psi2" or "Psi" or "Y" or "Jpsi"
    data_dir = config["locations"]["offline"]

    #Extract config infos: mass range, branches to be used and included in the trees
    mass_range = config["signal_regions"][particle][0]
    out_dir = data_dir[particle]
    branches = config["ntuple_branches"]
    branch_dic = {branch: 'float' for branch in branches}

    with up.recreate(os.path.join(out_dir, "merged_A.root")) as outfile:
        print("Start processing ",os.path.join(out_dir, "merged_A.root"))
        outfile.mktree("tree",branch_dic)
        error_indices = []
        nfiles=config["extraction"]["offline"]["njobs"]
        for i in range(nfiles):
            if i and i%10 == 0: 
                print(f"Processing {i}/{nfiles}", end="\r")
                sys.stdout.flush()
            try: 
                filename = data_dir["dump"]+"DimuonTree"+str(i)+".root:tree"
                # print(filename)
                intree = up.open(filename)

                #define mass cut 
                mass = intree["Mm_mass"].array()
                mass_cut = ((mass>mass_range[0])&(mass<mass_range[1]))
                print

                #define reduction factor cut
                num_events_surviving = int(np.sum(mass_cut)*reduction_factor)
                cut_reduction_idc = random.sample(range(np.sum(mass_cut)), num_events_surviving)

                #initialize empty dictionary tree
                tree = {}

                #fill tree with infile with mass cuts
                for branch in branch_dic.keys(): 
                    tree[branch] = intree[branch].array()[mass_cut][cut_reduction_idc]

                #extend the outfile with the dictionary for current file
                outfile["tree"].extend(tree)

            except Exception as e: 
                print(e, " error at ", i)
                error_indices.append(i)
                continue
                
        print("Error extracting files with indices: ", error_indices)

    return

def prepareTP(particle, dataset,tag="tightId",reduction_factor=0.5):

    #extract tree suited for tagnprobe
    if dataset=="Run3":
        data_dir = config["locations"]["offline"]
        nfiles=config["extraction"]["offline"]["njobs"]
    elif dataset=="MC_InclusiveMinBias": 
        data_dir = config["locations"]["MC_InclusiveMinBias"]
        nfiles=config["extraction"]["MC_InclusiveMinBias"]["njobs"]
    elif dataset =="MC_lmDY":
        data_dir = config["locations"]["MC_lmDY"]
        nfiles=config["extraction"]["MC_lmDY"]["njobs"]
    else: 
        raise ValueError("dataset not recognized")

    mass_ranges = config["signal_regions"][particle]
    out_dir = data_dir[particle]
    branches = ["Mm_mass", "Mm_dR", "Probe_pt", "Probe_eta","Probe_abs_eta","isBarrelMuon","PassingProbeSoftId","PassingProbeLooseId","HLT_DoubleMu4_3_LowMass"]
    types = ['float','float','float','float','float','int','int','int','int']
    branch_dic = {branch: t for branch,t in zip(branches,types)}
    outfilename = os.path.join(out_dir, f"TP_samples_{particle}.root")

    with up.recreate(outfilename) as outfile:
        outfile.mktree("tree",branch_dic)
        error_indices = []
        for i in range(nfiles):
            if i and i%10 == 0: 
                print(f"Processing {i}/{nfiles}", end="\r")
                sys.stdout.flush()
            try: 
                filename = data_dir["dump"]+"DimuonTree"+(dataset=="MC_InclusiveMinBias")*(particle+"_")+str(i)+".root:tree"
                intree = up.open(filename)

                #define mass cut 
                mass = intree["Mm_mass"].array()
                mass_cut = np.zeros(len(mass),dtype=bool)
                for mass_range in mass_ranges:
                    mass_cut = mass_cut | ((mass>mass_range[0])&(mass<mass_range[1]))

                len_before=np.sum(mass_cut)
                num_events_surviving = int(len_before*reduction_factor)
                cut_reduction_idc = random.sample(range(len_before), num_events_surviving)

                working_point_id=config["working_points"]["MuonId"]
                softId1 = intree["Muon_softMva1"].array(library='np')>working_point_id
                softId2 = intree["Muon_softMva2"].array(library='np')>working_point_id

                looseId1 = intree["Muon_looseId1"].array(library='np')
                looseId2 = intree["Muon_looseId2"].array(library='np')

                #define global tag cut
                if tag=="tightId":
                    tag_cut1= intree["Muon_tightId1"].array(library='np')[mass_cut][cut_reduction_idc] 
                    tag_cut2= intree["Muon_tightId2"].array(library='np')[mass_cut][cut_reduction_idc] 
               
                softProbe_cut1= softId1[mass_cut][cut_reduction_idc] 
                softProbe_cut2= softId2[mass_cut][cut_reduction_idc] 

                looseProbe_cut1= looseId1[mass_cut][cut_reduction_idc] 
                looseProbe_cut2= looseId2[mass_cut][cut_reduction_idc] 
                    
                isEven = intree["Event"].array(library='np')[mass_cut][cut_reduction_idc]%2==0
                charge1 = intree["Muon_charge1"].array(library='np')[mass_cut][cut_reduction_idc]==1

                #idea: if even, check positive charge. randomized is (muon 1 == positive). 
                denom = ((isEven & (charge1&tag_cut1 | (~charge1)&tag_cut2)) | 
                        (~isEven & (charge1&tag_cut2 | (~charge1)&tag_cut1)))

                softNum = ((isEven & (charge1&tag_cut1&softProbe_cut2 | (~charge1)&tag_cut2&softProbe_cut1)) |
                        (~isEven & (charge1&tag_cut2&softProbe_cut1 | (~charge1)&tag_cut1&softProbe_cut2)))
                
                looseNum = ((isEven & (charge1&tag_cut1&looseProbe_cut2 | (~charge1)&tag_cut2&looseProbe_cut1)) |
                        (~isEven & (charge1&tag_cut2&looseProbe_cut1 | (~charge1)&tag_cut1&looseProbe_cut2)))
                
                assert(len(denom) == len(softNum))

                #initialize empty dictionary tree
                tree = {}    
                tree["Mm_mass"] = intree["Mm_mass"].array()[mass_cut][cut_reduction_idc][denom]
                tree["HLT_DoubleMu4_3_LowMass"] = intree["HLT_DoubleMu4_3_LowMass"].array()[mass_cut][cut_reduction_idc][denom]
                tree["Mm_dR"] = np.sqrt((intree["Mm_mu1_eta"].array()-intree["Mm_mu2_eta"].array())**2 + (intree["Mm_mu1_phi"].array()-intree["Mm_mu2_phi"].array())**2)[mass_cut][cut_reduction_idc][denom]

                """
                I look at events that satisfy either  isEven&charge1 or (~isEven)&(~charge1) to avoid double counting 
                in the first case I tag on muon 1 . hence probe is muon2 .  
                """
                pt = np.zeros(len(isEven))
                pt[((isEven& (~charge1) & tag_cut2) | ((~isEven)&(charge1)&tag_cut2))] = intree["Mm_mu1_pt"].array(library='np')[mass_cut][cut_reduction_idc][((isEven& (~charge1) & tag_cut2) | ((~isEven)&(charge1)&tag_cut2))]
                pt[((isEven& charge1 & tag_cut1) | ((~isEven)&(~charge1)&tag_cut1))] = intree["Mm_mu2_pt"].array(library='np')[mass_cut][cut_reduction_idc][((isEven& charge1 & tag_cut1) | ((~isEven)&(~charge1)&tag_cut1))] 
                tree["Probe_pt"]=pt[denom]

                eta = np.zeros(len(isEven))
                eta[((isEven& (~charge1) & tag_cut2) | ((~isEven)&(charge1)&tag_cut2))] = intree["Mm_mu1_eta"].array(library='np')[mass_cut][cut_reduction_idc][((isEven& (~charge1) & tag_cut2) | ((~isEven)&(charge1)&tag_cut2))] 
                eta[((isEven& charge1 & tag_cut1) | ((~isEven)&(~charge1)&tag_cut1))] = intree["Mm_mu2_eta"].array(library='np')[mass_cut][cut_reduction_idc][((isEven& charge1 & tag_cut1) | ((~isEven)&(~charge1)&tag_cut1))] 
                tree["Probe_abs_eta"]=np.abs(eta[denom])
                tree["Probe_eta"]=eta[denom]

                barrel_cut = eta<1.46

                tree["isBarrelMuon"] = barrel_cut[denom]
                tree["PassingProbeSoftId"]=softNum[denom]
                tree["PassingProbeLooseId"]=looseNum[denom]

                #extend the outfile with the dictionary for current file
                outfile["tree"].extend(tree)

            except Exception as e: 
                print(e, " error at ", i)
                error_indices.append(i)
                continue
                
        print("Error extracting files with indices: ", error_indices)
        print(f"Generated outfile {outfilename}")

    return

if __name__ == "__main__":
    # print("please unindent a function in the main of skim_mass.py")
    # write_tree_data("Y",1)
    # write_tree_data("Jpsi",0.05)
    write_tree_data("Phi",0.08)
    write_tree_data("Psi2",0.1)
    # prepareTP('DP',"MC_lmDY",reduction_factor=1)
    # prepareTP('Jpsi',"MC_InclusiveMinBias",reduction_factor=1)
    # prepareTP('Jpsi',"Run3",reduction_factor=0.2)