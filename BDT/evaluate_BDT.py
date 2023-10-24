import numpy as np
import mplhep as hep
import xgboost as xgb
import matplotlib.pyplot as plt
from sklearn import datasets
from sklearn.model_selection import train_test_split
from sklearn.metrics import precision_score, accuracy_score
import uproot as up
import pandas as pd 
import awkward as ak
import subprocess

import os, sys
from config_loader import load_analysis_config,load_test_config

# Script to write the bash scripts with proper current configuration 

HOME_USER = os.environ["HOMELUDO"]
DP_USER = os.environ["DPLUDO"]
OFFLINE_FOLDER = os.path.join(DP_USER,"pull_data/offline/") 

config = load_analysis_config()


def evaluate_BDT(meson, model):
    """
    Meson: Jpsi or Y
    model: forest_standard_Y ...  
    
    """

    # extract offline data
    off_file_name = os.path.join(config["locations"]["offline"][meson], "merged_A.root")
    off_file=up.open(off_file_name)["tree"]
    print('successfully extracted offline data')
    
    #extract MC
    MC_file_name = os.path.join(config["locations"]["MC"][meson], "merged.root")
    MC_file=up.open(MC_file_name)
    MC_data = MC_file["tree"].arrays()#,library = 'pd')


    #load model
    method = model[:model.rfind("_")]
    train_meson = model[1+model.rfind("_"):]
    vars = config["BDT_training"][train_meson]["models"][method]["train_vars"] 
    all_branches = off_file.keys()
    bst = xgb.Booster()
    bst.load_model(DP_USER + "BDT/trained_models/" + model+".json")
    print("Model loaded")

    # extend tree using BDT prediction
    with up.create(os.path.join(config["locations"]["offline"][meson], "tmp.root")) as output_file:
        dic_mm = {branch: "float" for branch in all_branches}
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
            print(f"Finished chunk {i}/{n_chunks} in from {meson} meson", end = '\r')
        print("Finished processing data")

    subprocess.run('mv '+os.path.join(config["locations"]["offline"][meson], "tmp.root") + ' ' + os.path.join(config["locations"]["offline"][meson], "merged_A.root"), shell=True)

    #extend MC tree
    print("\nBegin processing MC")
    pred_MC=bst.predict(xgb.DMatrix(ak.to_dataframe(MC_data[vars])))
    MC_data[model+"_mva"] = pred_MC
    with up.recreate(MC_file_name) as output_file:
        output_file["tree"] = MC_data
    print("Finished processing MC")

    return


if __name__=="__main__":
    evaluate_BDT("Y", "forest_standard_Y")
    evaluate_BDT("Jpsi", "forest_standard_Y")
