import numpy as np
import xgboost as xgb
import uproot as up
import awkward as ak
import subprocess
import os
from config_loader import load_analysis_config,load_test_config

#Script to evaluate the trained BDTs on known mesons in configured mass ranges

HOME_USER = os.environ["HOMEUSER"]
DP_USER = os.environ["DPUSER"]
OFFLINE_FOLDER = os.path.join(DP_USER,"pull_data/offline/") 

config = load_analysis_config()


def evaluate_BDT(meson, model):
    """
    Meson (data): Jpsi or Y
    model (name of BDT to evaluate): forest_standard_Y ...  
    
    """

    # extract offline data
    off_file_name = os.path.join(config["locations"]["offline"][meson], "merged_A.root")
    off_file=up.open(off_file_name)["tree"]
    print(f'successfully extracted offline data: {off_file_name}')
    
    #extract MC
    MC_file_name = os.path.join(config["locations"]["MC_InclusiveMinBias"][meson], f"merged{'Y123' if meson=='Y' else ''}_A.root")
    MC_file=up.open(MC_file_name)
    MC_data = MC_file["tree"].arrays()#,library = 'pd')
    print(f'successfully extracted MC data: {MC_file_name}')


    #load model
    method = model[:model.rfind("_")]
    train_meson = model[1+model.rfind("_"):]
    vars = config["BDT_training"][train_meson]["models"][method]["train_vars"] 
    bst = xgb.Booster()
    bst.load_model(DP_USER + "BDT/trained_models/" + model+".json")
    print(f"Model {model} loaded")

    # extend tree using BDT prediction
    with up.create(os.path.join(config["locations"]["offline"][meson], "tmp.root")) as output_file:
        dic_mm = {branch: "float" for branch in off_file.keys()}
        dic = {**dic_mm, model+"_mva" : "float"}
        output_file.mktree("tree", dic)

        len_tree=len(off_file["Mm_mass"].array())
        step_size = 100000
        n_chunks = int(np.ceil(len_tree/step_size))
        i=0

        print(f"Begin processing {n_chunks} chunks of data from meson {meson}")

        for chunk in up.iterate(off_file, step_size = step_size):
            pred_off=bst.predict(xgb.DMatrix(ak.to_dataframe(chunk[vars])))
            chunk[model+"_mva"] = pred_off
            output_file["tree"].extend(chunk)
            i+=1
            print(f"Finished chunk {i}/{n_chunks} from {meson} meson", end = '\r')
        print("Finished processing data")

    subprocess.run('mv '+os.path.join(config["locations"]["offline"][meson], "tmp.root") + ' ' + os.path.join(config["locations"]["offline"][meson], "merged_A.root"), shell=True)

    #extend MC tree
    print("\nBegin processing MC")
    pred_MC=bst.predict(xgb.DMatrix(ak.to_dataframe(MC_data[vars])))
    MC_data[model+"_mva"] = pred_MC
    with up.recreate(MC_file_name) as output_file:
        output_file["tree"] = MC_data
    print("Finished processing MC\n\n")

    return


def evaluate_BDT_lmDY(model):
    
    #extract MC lmDY
    MC_file_name = os.path.join(config["locations"]["MC_lmDY"]["inclusive"], "merged_A.root")
    MC_file=up.open(MC_file_name)
    MC_data = MC_file["tree"].arrays()#,library = 'pd')
    print(f'successfully extracted MC data: {MC_file_name}')


    #load model
    method = model[:model.rfind("_")]
    train_meson = model[1+model.rfind("_"):]
    vars = config["BDT_training"][train_meson]["models"][method]["train_vars"] 
    bst = xgb.Booster()
    bst.load_model(DP_USER + "BDT/trained_models/" + model+".json")
    print(f"Model {model} loaded")

    #extend MC tree
    print("\nBegin processing MC")
    pred_MC=bst.predict(xgb.DMatrix(ak.to_dataframe(MC_data[vars])))
    MC_data[model+"_mva"] = pred_MC
    with up.recreate(MC_file_name) as output_file:
        output_file["tree"] = MC_data
    print("Finished processing MC\n\n")

    return

def evaluate_dump(njob,dataset,model,verbose=False):
    #to be used in batch system, njob is the job number

    if dataset not in ["MC_lmDY","MC_InclusiveMinBias","offline"]:
        raise ValueError(f"dataset {dataset} not recognized")
    
    #extract file
    infile_name = os.path.join(config["locations"][dataset]["dump"], f"DimuonTree{njob}.root")
    infile=up.open(infile_name)
    intree = infile["tree"]
    indata = intree.arrays()
    if verbose: print(f'successfully extracted MC data: {infile_name}')

    #load model
    method = model[:model.rfind("_")]
    train_meson = model[1+model.rfind("_"):]
    vars = config["BDT_training"][train_meson]["models"][method]["train_vars"] 
    bst = xgb.Booster()
    bst.load_model(DP_USER + "BDT/trained_models/" + model+".json")
    if verbose: print(f"Model {model} loaded")

    #Create new file 
    outfile_name = os.path.join(config["locations"][dataset]["dump_post_BDT"], f"DimuonTree{njob}.root")

    if verbose: print(f"\nBegin processing file {infile_name}")
    pred=bst.predict(xgb.DMatrix(ak.to_dataframe(indata[vars])))
    indata[model+"_mva"] = pred
    with up.recreate(outfile_name) as output_file:
        dic_mm = {branch: "float" for branch in intree.keys()}
        dic = {**dic_mm, model+"_mva" : "float"}
        output_file.mktree("tree", dic)
        output_file["tree"].extend(indata)
    if verbose: print(f"Finished processing data, output file is {outfile_name}\n\n")

    return

def evaluate_dumps(model = "forest_prompt_Jpsi"):

    #Switch for each type of dataset
    bools = "111"

    num = int(bools, 2)
    do_off =     bool(num & 0b100)
    do_lmDY =    bool(num & 0b010)
    do_MinBias = bool(num & 0b001)

    njobs_offline = config["extraction"]["offline"]["njobs"]
    njobs_MC_lmDY = config["extraction"]["MC_lmDY"]["njobs"]
    njobs_MC_InclusiveMinBias = config["extraction"]["MC_InclusiveMinBias"]["njobs"]

    if do_off:
        print(f"\nBegin processing offline")
        for i in range(njobs_offline):
            print(f"Job {i}/{njobs_offline}", end = '\r')
            evaluate_dump(i,"offline",model, verbose=False)
        print("Finished processing offline\n\n")
    if do_lmDY:
        print(f"\nBegin processing MC_lmDY")
        for i in range(njobs_MC_lmDY):
            print(f"Job {i}/{njobs_MC_lmDY}", end = '\r')
            evaluate_dump(i,"MC_lmDY",model, verbose=False)
        print("Finished processing MC_lmDY\n\n")
    if do_MinBias:
        print(f"\nBegin processing MC_InclusiveMinBias")
        for i in range(njobs_MC_InclusiveMinBias):
            print(f"Job {i}/{njobs_MC_InclusiveMinBias}", end = '\r')
            evaluate_dump(i,"MC_InclusiveMinBias",model, verbose=False)
        print("Finished processing MC_InclusiveMinBias\n\n")
    return

if __name__=="__main__":
    # print("")
    # evaluate_BDT("Y", "forest_standard_Y")
    # evaluate_BDT("Jpsi", "forest_standard_Y")
    evaluate_BDT("Y", "forest_prompt_Jpsi")
    evaluate_BDT("Jpsi", "forest_prompt_Jpsi")
    # evaluate_BDT_lmDY("forest_prompt_Jpsi")
    # evaluate_dumps()