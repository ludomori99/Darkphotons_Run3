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
        nfiles=10#config["condor_off"]["njobs"]
        for i in range(nfiles):
            if i and i%10 == 0: 
                print(f"Processing {i}/{nfiles}", end="\r")
                sys.stdout.flush()
            try: 
                filename = data_dir["dump"]+"DimuonTree"+str(i)+".root:tree"
                print(filename)
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


if __name__ == "__main__":
    write_tree_data("Y",off_data_dir)
    write_tree_data("Jpsi",off_data_dir)
