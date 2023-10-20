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
import os
import yaml

"""
Define imports, variables, and functions for later use

"""

HOME_USER = os.environ["HOMELUDO"] or None
DP_USER = os.environ["DPLUDO"] or None
OFFLINE_FOLDER = DP_USER+"/pull_data/offline/" or None

def load_analysis_config():
    try:
        with open(os.path.join(DP_USER,"config/analysis_config.yml"), "r") as f:
            config = yaml.load(f, Loader=yaml.FullLoader)
        return config

    except Exception as e: 
        raise e

config = load_analysis_config()


"""
Load data : 0.5 test, 0.25 val, 0.25 train
"""


class Trainer:

    def __init__(self, particle, modelname = None) -> None:
        self.particle = particle
        self.modelname = modelname
        self.train_config = config["BDT_training"][particle]
        return
    
    def load_data(self):
        print("Start loading data")
        self.off_dir = config["locations"]["offline"][self.particle]
        self.full_mass_range = up.open(self.off_dir+"merged_A.root:tree").arrays(library = 'pd')
        self.mass = self.full_mass_range["Mm_mass"]
        print("Successfully imported data file to memory")
        return
    
    def prepare_training_set(self):
        #Define signal and background
        sig_lims = self.train_config["limits"]["signal"]
        bkg_lims = self.train_config["limits"]["background"]

        if self.modelname == None: self.modelname = self.train_config["modelname"]
        self.hyperpars = self.train_config["models"][self.modelname]["hyperpars"]
        self.train_vars = self.train_config["models"][self.modelname]["train_vars"] 
        random_seed = self.train_config["models"][self.modelname]["random_seed"]

        #for loop over pair of extremals of signal/bkg regions, to keep as general as possible in case of multiple signal/bkg intervals
        sig_cut = (self.mass < -1e3)
        for lims in sig_lims:
            sig_cut = sig_cut | ((self.mass>lims[0])&(self.mass<lims[1]))

        bkg_cut = (self.mass < -1e3)
        for lims in bkg_lims:
            bkg_cut = bkg_cut | ((self.mass>lims[0])&(self.mass<lims[1]))

        sig = self.full_mass_range[sig_cut]
        bkg = self.full_mass_range[bkg_cut]
        sig['Score'] = 1
        bkg['Score'] = 0
        sig_frac = len(sig)/(len(sig)+len(bkg))

        self.trainData = pd.concat([sig,bkg])
        self.trainData_skinny = self.trainData[self.train_vars]
        X_temp, self.X_test, y_temp, self.y_test = train_test_split(self.trainData_skinny, self.trainData["Score"], test_size=0.5, shuffle=True, random_state=random_seed)
        self.X_train, self.X_val, self.y_train, self.y_val = train_test_split(X_temp, y_temp, test_size=0.5, shuffle=True, random_state=random_seed)
        self.dtrain = xgb.DMatrix(self.X_train, label=self.y_train)
        self.dval = xgb.DMatrix(self.X_val, label=self.y_val)
        print(f"Defined training and evaluation datasets\nTrain on {len(self.X_train)} events, of which a fraction {sig_frac} is signal, and {len(self.train_vars)} variables")

    def train_model(self):
        num_round = self.train_config["models"][self.modelname]["num_rounds"]
        evallist = [(self.dtrain, 'train'), (self.dval, 'eval')]
        self.bst = xgb.train(self.hyperpars, self.dtrain, num_round, evallist)
        self.bst.save_model(os.path.join(DP_USER,"BDT/trained_models/", self.modelname,"_",self.particle,".json"))
        print("Training successful, model saved to file " + os.path.join(DP_USER,"BDT/trained_models/", self.modelname,"_",self.particle,".json"))
        return

    def load_model(self):
        self.bst = xgb.Booster()
        print("loading model ",os.path.join(DP_USER,"BDT/trained_models/", self.modelname,"_",self.particle,".json"))
        try:
            self.bst.load_model(os.path.join(DP_USER,"BDT/trained_models/", self.modelname,"_",self.particle,".json"))
            print("loading successful")
        except Exception as e:
            print(e)
        return 
    
    def complete_train(self):
        self.load_data()
        self.prepare_training_set()
        self.train_model()

    def complete_load(self):
        self.load_data()
        self.prepare_training_set()
        self.load_model()

    @staticmethod
    def plot_hist(data,names,nbins=100,xlabel = None, saveas=None, text = None, xlim=None, log = False, density=False, int_xticks = False):
        hep.style.use("CMS")
        colors = plt.cm.tab10.colors
        fig, ax = plt.subplots(figsize=(12,9))
        hep.cms.text("Preliminary")
        for d,name,c in zip(data,names,colors[:len(data)]):
            ax.hist(d, bins = nbins, range = xlim, label=name, color=c, density = density, log=log, histtype='step', linewidth=2)
            # ax.hist(d, bins = nbins, range = xlim, color=c, density = density, log=log, alpha = 0.5)# hatch = '*',
        if (xlabel): ax.set_xlabel(xlabel)
        if text!=None: ax.text(0.02, .8, text, fontsize=11, bbox=dict(facecolor='white', edgecolor='black'), transform=ax.transAxes) 
        if int_xticks: ax.xaxis.get_major_locator().set_params(integer=True)
        ax.set_ylabel('Normalized frequency')
        ax.set_xlim(xlim)
        ax.legend()
        ax.grid(True)
        if saveas: plt.savefig(saveas)
        return
    
    def plot_model(self,**kwargs):
        self.val_bkg=self.bst.predict(xgb.DMatrix(self.X_val[self.y_val==0]))
        self.val_sig=self.bst.predict(xgb.DMatrix(self.X_val[self.y_val==1]))
        self.train_bkg=self.bst.predict(xgb.DMatrix(self.X_train[self.y_train==0]))
        self.train_sig=self.bst.predict(xgb.DMatrix(self.X_train[self.y_train==1]))
        self.plot_hist([self.val_bkg,self.val_sig,self.train_bkg,self.train_sig],["Bkg.","Sig.","Training bkg.","Train sig."],nbins =100, density=True, xlabel="BDT score",**kwargs)



def plot_ROC(train,test,bsts,labels, tmva=None, log = False):
    hep.style.use("CMS")
    colors = plt.cm.tab10.colors
    fig, ax = plt.subplots(figsize=(9,9))
    hep.cms.text("Preliminary")

    true_test_sig = test.get_label()==1
    true_test_bkg = ~true_test_sig

    true_train_sig = train.get_label()==1
    true_train_bkg = ~true_train_sig

    dis = np.linspace(0.01,0.99,20)

    for bst,c1,c2,l in zip(bsts,colors,colors[3:],labels):
        sig_eff_test = []
        bkg_rej_test = []
        sig_eff_train = []
        bkg_rej_train = []
        preds_test = bst.predict(test)
        preds_train = bst.predict(train)
        for d in dis:
            pred_test_sig = preds_test>d
            pred_test_bkg = ~pred_test_sig
            sig_eff_test.append(np.sum(true_test_sig & pred_test_sig)/np.sum(true_test_sig))
            bkg_rej_test.append(np.sum(true_test_bkg & pred_test_bkg)/np.sum(true_test_bkg))

            pred_train_sig = preds_train>d
            pred_train_bkg = ~pred_train_sig
            sig_eff_train.append(np.sum(true_train_sig & pred_train_sig)/np.sum(true_train_sig))
            bkg_rej_train.append(np.sum(true_train_bkg & pred_train_bkg)/np.sum(true_train_bkg))
        
        ax.scatter(sig_eff_test, bkg_rej_test, color =c1, zorder=0, label = l+" test")
        ax.plot(sig_eff_test, bkg_rej_test, lw=1.3, color = c1)
        ax.scatter(sig_eff_train, bkg_rej_train, color =c2, zorder=0, label = l+" train")
        ax.plot(sig_eff_train, bkg_rej_train, lw=1.3, color = c2)
    
    ax.set_xlabel('Signal efficiency')
    ax.set_ylabel('Background rejection')
    ax.grid(True)

    if tmva: 
        ax.scatter(tmva["sig_eff_test_tmva"], tmva["bkg_rej_test_tmva"], color = "blue", zorder=0, label = "TMVA test")
        ax.plot(tmva["sig_eff_test_tmva"], tmva["bkg_rej_test_tmva"], lw=1.3, color = "blue")
        ax.scatter(tmva["sig_eff_train_tmva"], tmva["bkg_rej_train_tmva"], color ="red", zorder=0, label = "TMVA train")
        ax.plot(tmva["sig_eff_train_tmva"], tmva["bkg_rej_train_tmva"], lw=1.3, color = "red")
    
    ax.legend()
    plt.show()
    return 


if __name__ == "__main__":
    Y_trainer = Trainer("Y", 'tree_standard')
    Y_trainer.complete_train()
    Y_trainer.plot_model(saveas=config["locations"]["public_html"]+"BDTs/Y_tree_standard.png")

    # Jpsi_trainer = Trainer("Jpsi")
    # Jpsi_trainer.complete_train()
    