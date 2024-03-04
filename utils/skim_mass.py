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


#directory to extract ntuples. dump dir has slightly different definition
off_data_dir = config["locations"]["offline"]
MC_data_dir = config["locations"]["MC_InclusiveMinBias"]

def write_tree_data(particle, data_dir):

    #particle = "Y" or "Jpsi"

    #Extract config infos: mass range, branches to be used and included in the trees
    mass_range = config["BDT_training"][particle]["limits"]["inclusive"]
    out_dir = data_dir[particle]
    reduction_factor = config["BDT_training"][particle]["reduction_factor"]
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

def prepareTP(particle, data_dir,tag="tightId",probe="softMvaId",reduction_factor=0.05,isMC=False):

    #extract tree suited for tagnprobe

    mass_range = config["BDT_training"][particle]["limits"]["inclusive"]
    out_dir = data_dir[particle]
    branches = ["Mm_mass", "Probe_pt", "Probe_eta","PassingProbeSoftId"]
    types = ['float','float','float','int']
    branch_dic = {branch: t for branch,t in zip(branches,types)}

    with up.recreate(os.path.join(out_dir, f"TP_samples_{particle}.root")) as outfile:
        outfile.mktree("tree",branch_dic)
        error_indices = []
        if isMC: nfiles=config["extraction"]["MC_InclusiveMinBias"]["njobs"]
        else: nfiles=config["extraction"]["offline"]["njobs"]
        for i in range(nfiles):
            if i and i%10 == 0: 
                print(f"Processing {i}/{nfiles}", end="\r")
                sys.stdout.flush()
            try: 
                filename = data_dir["dump"]+"DimuonTree"+isMC*particle+str(i)+".root:tree"
                intree = up.open(filename)

                #define mass cut 
                mass = intree["Mm_mass"].array()
                mass_cut = ((mass>mass_range[0])&(mass<mass_range[1]))

                len_before=np.sum(mass_cut)
                num_events_surviving = int(len_before*reduction_factor)
                cut_reduction_idc = random.sample(range(len_before), num_events_surviving)

                working_point_id=0.223
                softId1 = intree["Muon_softMva1"].array(library='np')>working_point_id
                softId2 = intree["Muon_softMva2"].array(library='np')>working_point_id

                #define global tag cut
                if tag=="tightId":
                    tag_cut1= intree["Muon_tightId1"].array(library='np')[mass_cut][cut_reduction_idc] 
                    tag_cut2= intree["Muon_tightId2"].array(library='np')[mass_cut][cut_reduction_idc] 
                #else ... 

                if probe=="softMvaId":
                    probe_cut1= softId1[mass_cut][cut_reduction_idc] 
                    probe_cut2= softId2[mass_cut][cut_reduction_idc] 
                #else ... 
                    
                isEven = intree["Event"].array(library='np')[mass_cut][cut_reduction_idc]%2==0
                charge1 = intree["Muon_charge1"].array(library='np')[mass_cut][cut_reduction_idc]==1

                #idea: if even, check positive charge. randomized is (muon 1 == positive). 
                denom = ((isEven & (charge1&tag_cut1 | (~charge1)&tag_cut2)) | 
                        (~isEven & (charge1&tag_cut2 | (~charge1)&tag_cut1)))

                num = ((isEven & (charge1&tag_cut1&probe_cut2 | (~charge1)&tag_cut2&probe_cut1)) |
                        (~isEven & (charge1&tag_cut2&probe_cut1 | (~charge1)&tag_cut1&probe_cut2)))
                
                assert(len(denom) == len(num))

                #initialize empty dictionary tree
                tree = {}    
                tree["Mm_mass"] = intree["Mm_mass"].array()[mass_cut][cut_reduction_idc][denom]
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
                tree["Probe_eta"]=np.abs(eta[denom])
                
                tree["PassingProbeSoftId"]=num[denom]

                #extend the outfile with the dictionary for current file
                outfile["tree"].extend(tree)

            except Exception as e: 
                print(e, " error at ", i)
                error_indices.append(i)
                continue
                
        print("Error extracting files with indices: ", error_indices)

    return

if __name__ == "__main__":
    # print("please unindent a function in the main of skim_mass.py")
    write_tree_data("Y",off_data_dir)
    write_tree_data("Jpsi",off_data_dir)
    # prepareTP('Y',off_data_dir)
    prepareTP('Jpsi',off_data_dir)
    prepareTP('Jpsi',MC_data_dir,reduction_factor=1,isMC=True)