import numpy as np
import xgboost as xgb
import uproot as up
import awkward as ak
import yaml
import os,sys
# from config_loader import load_analysis_config,load_test_config

# Script to write the bash scripts with proper current configuration 

os.environ["HOMELUDO"]="/home/submit/mori25/"
os.environ["DPUSER"]="/work/submit/mori25/Darkphotons_ludo/offline_analysis/"
HOME_USER = os.environ["HOMELUDO"]
DP_USER = os.environ["DPUSER"]

def load_analysis_config():
    try:
        with open(os.path.join(os.environ["DPUSER"],"config/analysis_config.yml"), "r") as f:
            config = yaml.load(f, Loader=yaml.FullLoader)
        return config

    except Exception as e: 
        raise e
    
config = load_analysis_config()
njob = sys.argv[1]

# extract MC data
MC_file_name = os.path.join(config["locations"]["MC_lmDY"]["dump"], f"DimuonTree{njob}.root")
MC_file=up.open(MC_file_name)["tree"]

#load model
meson = "Jpsi"
model = "forest_prompt"
vars = config["BDT_training"][meson]["models"][model]["train_vars"]  # choice of model hardcoded for now
bst = xgb.Booster()
bst.load_model(os.path.join(DP_USER, f"BDT/trained_models/{model}_{meson}.json"))

# extend tree using BDT prediction
with up.create(os.path.join(config["locations"]["MC_lmDY"]["dump_post_BDT"], f"DimuonTree{njob}_BDT.root")) as output_file:
    dic_mm = {branch: "float" for branch in MC_file.keys()}
    dic = {**dic_mm, model+"_mva" : "float"}
    output_file.mktree("tree", dic)

    len_tree=len(MC_file["Mm_mass"].array())
    step_size = 100000
    n_chunks = int(np.ceil(len_tree/step_size))
    i=0

    for chunk in up.iterate(MC_file, step_size = step_size):
        pred_off=bst.predict(xgb.DMatrix(ak.to_dataframe(chunk[vars])))
        chunk[model+"_mva"] = pred_off
        output_file["tree"].extend(chunk)
        i+=1
        print(f"Finished chunk {i}/{n_chunks} ")
    print("Finished processing data")

# subprocess.run('mv '+os.path.join(config["locations"]["offline"]["dump"], "tmp.root") + ' ' + os.path.join(config["locations"]["offline"][meson], "merged_A.root"), shell=True)
