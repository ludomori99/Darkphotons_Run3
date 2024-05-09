import numpy as np
import xgboost as xgb
import uproot as up
import awkward as ak
import yaml
import os,sys
# from config_loader import load_analysis_config,load_test_config

HOME_USER = os.environ["HOMEUSER"]
DP_USER = os.environ["DPUSER"]

model = "forest_prompt_Jpsi"
###### Choose which model you want


def load_analysis_config():
    try:
        with open(os.path.join(os.environ["DPUSER"],"config/analysis_config.yml"), "r") as f:
            config = yaml.load(f, Loader=yaml.FullLoader)
        return config

    except Exception as e: 
        raise e
    
config = load_analysis_config()
njob = int(sys.argv[1])

def evaluate_dump(njob,dataset,model,meson=None,verbose=False):
    #to be used in batch system, njob is the job number

    if dataset not in ["MC_lmDY","MC_InclusiveMinBias","offline"] :
        raise ValueError(f"dataset {dataset} not recognized")
    if dataset != "MC_InclusiveMinBias" and meson is not None:
        raise ValueError(f"Indicate meson only for MinBias")
    if dataset == "MC_InclusiveMinBias" and meson not in ["Jpsi","Y1","Y2","Y3","Inclusive"]:
        raise ValueError(f"meson {meson} not recognized")
    
    #extract file
    infile_name = os.path.join(config["locations"][dataset]["dump"], f"""DimuonTree{njob if dataset!="MC_InclusiveMinBias" else meson+"_"+str(njob)}.root""")
    infile=up.open(infile_name)
    intree = infile["tree"]
    indata = intree.arrays()
    if verbose: print(f'successfully extracted {dataset} data: {infile_name}')

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


njobs_offline = config["extraction"]["offline"]["njobs"]
njobs_MC_lmDY = config["extraction"]["MC_lmDY"]["njobs"]
njobs_MC_InclusiveMinBias = config["extraction"]["MC_InclusiveMinBias"]["njobs"]

if njob < njobs_MC_lmDY: evaluate_dump(njob,"MC_lmDY",model,verbose=True)
if njob < njobs_MC_InclusiveMinBias: 
    evaluate_dump(njob,"MC_InclusiveMinBias",model,meson="Inclusive",verbose=True)
    evaluate_dump(njob,"MC_InclusiveMinBias",model,meson="Jpsi",verbose=True)
    evaluate_dump(njob,"MC_InclusiveMinBias",model,meson="Y1",verbose=True)
if njob < njobs_offline: evaluate_dump(njob,"offline",model,verbose=True)

# subprocess.run('mv '+os.path.join(config["locations"]["offline"]["dump"], "tmp.root") + ' ' + os.path.join(config["locations"]["offline"][meson], "merged_A.root"), shell=True)
