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
    MC_file_name = os.path.join(config["locations"]["MCRun3"][meson], "merged_A.root")
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

def evaluate_dump(model):

    return


if __name__=="__main__":
    print("")
    # evaluate_BDT("Y", "forest_standard_Y")
    # evaluate_BDT("Jpsi", "forest_standard_Y")
    evaluate_BDT("Y", "forest_prompt_Jpsi")
    evaluate_BDT("Jpsi", "forest_prompt_Jpsi")
