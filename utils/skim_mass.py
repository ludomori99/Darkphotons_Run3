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

#directory to extract ntuples. dump dir has slightly different definition
off_dump_dir = "/data/submit/"+config["locations"]["offline"]["dump"]

def write_tree_data(particle):

    #particle = "Y" or "Jpsi"

    #Extract config infos: mass range, branches to be used and included in the trees
    mass_range = config["BDT_training"][particle]["limits"]["inclusive"]
    off_dir = config["locations"]["offline"][particle]
    reduction_factor = config["BDT_training"][particle]["reduction_factor"]
    branches = config["ntuple_branches"]
    branch_dic = {branch: 'float' for branch in branches}

    with up.recreate(off_dir + "merged_A.root") as outfile:
        outfile.mktree("tree",branch_dic)
        error_indices = []
        for i in range(config["condor"]["njobs"]):
            try: intree = up.open(off_dump_dir+"DimuonTree"+str(i)+".root:tree")
            except Exception as e: 
                print(e, " error at ", i)
                error_indices.append(i)
                continue
                
            #define mass cut 
            mass = intree["Mm_mass"].array()
            mass_cut = ((mass>mass_range[0])&(mass<mass_range[1]))

            #define reduction factor cut
            num_events_surviving = int(np.sum(mass_cut)*reduction_factor)
            cut_reduction_idc = random.sample(range(np.sum(mass_cut)), num_events_surviving)
            cut_reduction = np.zeros((np.sum(mass_cut),))
            cut_reduction[cut_reduction_idc]=1

            #initialize empty dictionary tree
            tree = {}

            #fill tree with infile with mass cuts
            for branch in branch_dic.keys(): 
                tree[branch] = intree[branch].array()[mass_cut][cut_reduction.astype(int)]

            #extend the outfile with the dictionary for current file
            outfile["tree"].extend(tree)
            if i%10 == 0: print("done file #", i)
        print("Error extracting files with indices: ", error_indices)

    return


if __name__ == "__main__":
    write_tree_data("Y")
    write_tree_data("Jpsi")

