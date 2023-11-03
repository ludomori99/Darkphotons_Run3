import numpy as np
import mplhep as hep
import xgboost as xgb
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.metrics import precision_score, accuracy_score
import uproot as up
import pandas as pd 
import awkward as ak
import os
import yaml
from numba_stats import expon # crystalball,crystalball_ex, norm, expon, voigt, truncexpon
from iminuit.cost import LeastSquares #,  ExtendedBinnedNLL,ExtendedUnbinnedNLL,
from iminuit import Minuit
from numba import jit,njit,vectorize

"""
Define imports, variables, and functions for later use

"""

HOME_USER = os.environ.get("HOMELUDO", None)
DP_USER = os.environ.get("DPLUDO", None)
OFFLINE_FOLDER = DP_USER+"/pull_data/offline/" if DP_USER else None

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
        self.particle = particle #the particle on which training was performed
        self.particle_config = config["BDT_training"][particle]
        if modelname is not None: self.modelname = modelname
        else: self.modelname = self.particle_config["BDT_training"]["modelname"]
        return
    
    def load_data(self, data_particle = None):
        print("Start loading data")
        if data_particle: self.off_dir = config["locations"]["offline"][data_particle]
        else :  self.off_dir = config["locations"]["offline"][self.particle]
        filename=self.off_dir+"merged_A.root:tree"
        self.full_mass_range = up.open(filename).arrays(library = 'pd')
        self.mass = self.full_mass_range["Mm_mass"]
        print(f"Successfully imported data file {filename} to memory")
        return
    
    def prepare_training_set(self,data_particle = None, data_override=None, weights = None,w_frac_bkg=0.1):
        """
        data_particle: if want to evaluate on particle different from particle from training given in __init__
        """
        if data_override is not None: 
            self.full_mass_range = data_override
            self.mass = self.full_mass_range["Mm_mass"]
        #Define signal and background
        sig_lims = self.particle_config["limits"]["signal"] if not data_particle else config["BDT_training"][data_particle]["limits"]["signal"] 
        bkg_lims = self.particle_config["limits"]["background"] if not data_particle else config["BDT_training"][data_particle]["limits"]["background"] 

        if self.modelname == None: self.modelname = self.particle_config["modelname"]
        self.hyperpars = self.particle_config["models"][self.modelname]["hyperpars"]
        self.train_vars = self.particle_config["models"][self.modelname]["train_vars"] 
        random_seed = self.particle_config["models"][self.modelname]["random_seed"]

        #for loop over pair of extremals of signal/bkg regions, to keep as general as possible in case of multiple signal/bkg intervals
        sig_cut = (self.mass < -1e3)
        for lims in sig_lims:
            sig_cut = sig_cut | ((self.mass>lims[0])&(self.mass<lims[1]))

        bkg_cut = (self.mass < -1e3)
        for lims in bkg_lims:
            bkg_cut = bkg_cut | ((self.mass>lims[0])&(self.mass<lims[1]))

        sig = pd.DataFrame(self.full_mass_range[sig_cut])
        bkg = pd.DataFrame(self.full_mass_range[bkg_cut])

        sig['Score'] = 1
        bkg['Score'] = 0
        self.sig_frac = -1 if (len(sig)+len(bkg)) == 0 else len(sig)/(len(sig)+len(bkg))

        self.trainData = pd.concat([sig,bkg]).reset_index(drop=True)
        self.trainData_skinny = self.trainData[self.train_vars]

        if weights is not None:
            sig_weights = weights[sig_cut]
            bkg_weights = w_frac_bkg*np.ones((len(bkg),))
            weights = np.concatenate([sig_weights,bkg_weights])     
            self.weights=weights
            print(f"Total signal events: {np.sum(sig_cut)}\n Total signal weight: {np.sum(sig_weights)}, {np.round(np.sum(sig_weights)/(np.sum(sig_weights)+np.sum(bkg_weights)),2)} of total weight")
        
        #NB: train_test_split conserves row indexing throughout splitting. Indexing range remains that of trainData (meaning X_train and X_val will be smaller than trainData but retain the corresponding indices)

        X_temp, self.X_test, y_temp, self.y_test = train_test_split(self.trainData_skinny, self.trainData["Score"], test_size=0.1, shuffle=True, random_state=random_seed)
        self.X_train, self.X_val, self.y_train, self.y_val = train_test_split(X_temp, y_temp, test_size=0.5, shuffle=True, random_state=random_seed)
        self.dtrain = xgb.DMatrix(self.X_train, label=self.y_train, weight = weights[self.X_train.index] if weights is not None else None) 
        self.dval = xgb.DMatrix(self.X_val, label=self.y_val, weight = weights[self.X_val.index] if weights is not None else None)
        print(f"Defined training and evaluation datasets\n")

    def train_model(self):
        print(f"Train on {len(self.X_train)} events, of which an approximate fraction {self.sig_frac} is signal, and {len(self.train_vars)} variables")
        num_round = self.particle_config["models"][self.modelname]["num_rounds"]
        evallist = [(self.dtrain, 'train'), (self.dval, 'eval')]
        self.bst = xgb.train(self.hyperpars, self.dtrain, num_round, evals=evallist)
        self.bst.save_model(os.path.join(DP_USER,"BDT/trained_models/", self.modelname+"_"+self.particle+".json"))
        print("Training successful, model saved to file " + os.path.join(DP_USER,"BDT/trained_models/", self.modelname+"_"+self.particle+".json"))
        return

    def load_model(self):
        self.bst = xgb.Booster()
        print("loading model ",os.path.join(DP_USER,"BDT/trained_models/", self.modelname+"_"+self.particle+".json"))
        try:
            self.bst.load_model(os.path.join(DP_USER,"BDT/trained_models/", self.modelname+"_"+self.particle+".json"))
            print("loading successful")
        except Exception as e:
            print(e)
        return 
    
    def complete_train(self):
        #out-of-the-box for training
        self.load_data()
        self.prepare_training_set()
        self.train_model()

    def complete_load(self, data_particle = None):
        self.load_data(data_particle)
        self.prepare_training_set(data_particle)
        self.load_model()

    @staticmethod
    def compute_reweight(data,variable,fitting_func, nbins_fit = 100, nbins_corrections=100, fit_range = None, fitting_limits=None, plot=False, xrange=None, plot_logscale=False):
        """
        data: ak array, dic-like, containing all events
        variable: string, name of var to compute the reweighing. e.g.: Mm_kin_lxy
        fitting_func: function to fit to histogram in range fit_range. signature f(variable,N,*args) w/ N normalization
        nbins: int, obv
        range: range we compute the histogram
        fitting_limits: array of 2-tuples, with length #args - 2
        """

        n_vars_fit_func = fitting_func.__code__.co_argcount   
        assert n_vars_fit_func > 1 and type(n_vars_fit_func)==int, "fit function has too few arguments"
        if fit_range == None: fit_range = (min(data[variable]),max(data[variable]))

        ### Fitting 
        print("Start fitting function ",fitting_func,"to variable ",variable)
        costMethod = LeastSquares
        n_fit,xe_fit = np.histogram(ak.to_numpy(data[variable]), nbins_fit)
        x_fit = 0.5*(xe_fit[1:] + xe_fit[:-1])
        dx_fit = xe_fit[1]-xe_fit[0]
        mask = (x_fit > fit_range[0]) & (x_fit < fit_range[1])
        cf_data = costMethod(x_fit[mask],n_fit[mask], n_fit[mask]**0.5, fitting_func)
        ivData = [len(data[variable])] + [1 for i in range(n_vars_fit_func - 2)] #we assume the first variable to always be the normalization 
        mData = Minuit(cf_data,*ivData)
        mData.limits = [(0,len(data[variable]))] + fitting_limits if fitting_limits else [(0,len(data[variable]))] + [None for i in range(n_vars_fit_func - 2)] 
        mData.migrad()
        mData.hesse()
        print("Done fitting. Parameters: ", *mData.values)

        histSlxy,xe_corr = np.histogram(ak.to_numpy(data[variable]),nbins_corrections)
        x_corr = 0.5*(xe_corr[1:] + xe_corr[:-1])
        dx_corr = xe_corr[1]-xe_corr[0]
        weight_hist = np.where(histSlxy==0,0, (histSlxy-fitting_func(x_corr,*mData.values)/dx_fit*dx_corr)/histSlxy)
        weights = weight_hist[np.digitize(data[variable], xe_corr, right=True) - 1]   

        import scipy
        Nonprompt_in_prompt,_ = scipy.integrate.quad(lambda x: fitting_func(x,*mData.values)/dx_fit, 0, xe_corr[-1])
        prompt = np.sum(histSlxy) - Nonprompt_in_prompt

        print(f"""\nSome General infos:\n
                Total number of events considered: {len(data)} \n
                Sum of weights: {np.sum(weights)} \n
                Fitted prompt: {prompt}\n
                Sum of weights>0 {np.sum(weights[weights>=0])}\n\n""")

        if plot:
            hep.style.use("CMS")
            fig, ax = plt.subplots(figsize=(10,6))
            x = np.linspace(0,xe_corr[-1], 1000)
            ax.plot(x,fitting_func(x,*mData.values), label = "Nonprompt tail fit")
            # ax.errorbar(0.5*(xe_corr[:-1] + xe_corr[1:]),histSlxy-fitting_func(0.5*(xe_corr[:-1] + xe_corr[1:]),*mData.values)/dx_fit*dx_corr,xerr = dx_corr/2, label="$J/\psi$ prompt data", color = "green", zorder=0,marker = '.')
            ax.hist(data["Mm_kin_lxy"], nbins_corrections,   label="$J/\psi$ data", color = "orange", zorder=0, histtype='step', linewidth = 1.8)
            ax.hist(data["Mm_kin_lxy"], nbins_corrections,  weights=weights, label="$J/\psi$ data reweighed", color = "brown", zorder=0, histtype='step', linewidth = 1.8)
            # ax.hist(data["Mm_kin_lxy"], nbins_corrections,  weights = (data["Mm_kin_lxy"]>fit_range[0]), color = "purple", zorder=0, histtype='bar', linewidth = 1.8, alpha = .2)
            # ax.text(0.2,0.9, f"J/$\psi$ data, sPlot-unfolded $l_{{xy}}$ distribution \nShaded area is nonprompt tail", fontsize = 12, transform=ax.transAxes)
            ax.grid()
            ax.legend()    
            if xrange is not None: ax.set_xlim(xrange)
            ax.set_ylim(1e-2,1e6)
            ax.set_xlabel("$l_{xy}$")
            # ax.set_ylim(0,20000)
            if plot_logscale: ax.set_yscale('log')
            ax.set_ylabel("Frequency")    
            plt.show()
    
        return weights
    

    @staticmethod
    def plot_hist(data,names,nbins=100, weights = None, xlabel = None, saveas=None, text = None, xlim=None, log = False, density=False, int_xticks = False):
        hep.style.use("CMS")
        colors = plt.cm.tab10.colors
        fig, ax = plt.subplots(figsize=(12,9))
        hep.cms.text("Preliminary")
        if weights is not None:
            for d,w,name,c in zip(data,weights,names,colors[:len(data)]):
                ax.hist(d, bins = nbins, weights=w, range = xlim, label=name, color=c, density = density, log=log, histtype='step', linewidth=2)
                # ax.hist(d, bins = nbins, range = xlim, color=c, density = density, log=log, alpha = 0.5)# hatch = '*',
        else: 
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
        if saveas: 
            plt.savefig(saveas)
            print(f"saved figure as {saveas}")
        return
    
    def plot_model(self,plot_training=False, apply_weights=False,density=True,**kwargs):
        self.val_bkg=self.bst.predict(xgb.DMatrix(self.X_val[self.y_val==0]))
        self.val_sig=self.bst.predict(xgb.DMatrix(self.X_val[self.y_val==1]))
        self.train_bkg=self.bst.predict(xgb.DMatrix(self.X_train[self.y_train==0]))
        self.train_sig=self.bst.predict(xgb.DMatrix(self.X_train[self.y_train==1]))

        j=4 if plot_training else 2
        
        if apply_weights: 
            self.plot_hist([self.val_bkg,self.val_sig,self.train_bkg,self.train_sig][:j],
                       ["Bkg.","Sig.","Training bkg.","Train sig."][:j],
                       weights = [self.weights[self.X_val.index[self.y_val==0]],self.weights[self.X_val.index[self.y_val==1]],self.weights[self.X_train.index[self.y_train==0]],self.weights[self.X_train.index[self.y_train==1]]][:j],
                       nbins =100, density=density, xlabel="BDT score",**kwargs)
        else : 
            self.plot_hist([self.val_bkg,self.val_sig,self.train_bkg,self.train_sig][:j],
                       ["Bkg.","Sig.","Training bkg.","Train sig."][:j],
                       nbins =100, density=density, xlabel="BDT score",**kwargs)

#deprecated
def plot_ROC_train_test(trainers,labels, tmva=None, log = False,n_points=20):
    hep.style.use("CMS")
    colors = plt.cm.tab10.colors
    fig, ax = plt.subplots(figsize=(9,9))
    hep.cms.text("Preliminary")

    dis = np.linspace(0.01,0.99,n_points)

    for trainer,c1,c2,l in zip(trainers,colors,colors[3:],labels):

        if len(trainer.dval.get_weight())==0: trainer.dval.set_weight(np.ones((len(trainer.dval.get_label()))))
        if len(trainer.dtrain.get_weight())==0: trainer.dtrain.set_weight(np.ones((len(trainer.dtrain.get_label()),)))

        true_test_sig = trainer.dval.get_label()==1
        true_test_bkg = ~true_test_sig
        true_train_sig = trainer.dtrain.get_label()==1
        true_train_bkg = ~true_train_sig

        sig_eff_test = []
        bkg_rej_test = []
        sig_eff_train = []
        bkg_rej_train = []
        preds_test = trainer.bst.predict(trainer.dval)
        preds_train = trainer.bst.predict(trainer.dtrain)
        for d in dis:
            pred_test_sig = preds_test>d
            pred_test_bkg = ~pred_test_sig
            sig_eff_test.append(np.sum(trainer.dval.get_weight()[true_test_sig & pred_test_sig])/np.sum(trainer.dval.get_weight()[true_test_sig]))
            bkg_rej_test.append(np.sum(trainer.dval.get_weight()[true_test_bkg & pred_test_bkg])/np.sum(trainer.dval.get_weight()[true_test_bkg]))

            pred_train_sig = preds_train>d
            pred_train_bkg = ~pred_train_sig
            sig_eff_train.append(np.sum(trainer.dtrain.get_weight()[true_train_sig & pred_train_sig])/np.sum(trainer.dtrain.get_weight()[true_train_sig]))
            bkg_rej_train.append(np.sum(trainer.dtrain.get_weight()[true_train_bkg & pred_train_bkg])/np.sum(trainer.dtrain.get_weight()[true_train_bkg]))
        
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

def plot_ROC(trainers,labels,evals, n_points = 50, tmva=None, log = False):
    hep.style.use("CMS")
    colors = plt.cm.tab10.colors
    fig, ax = plt.subplots(figsize=(9,9))
    hep.cms.text("Preliminary")

    dis = np.linspace(0.001,0.999,n_points)

    for trainer,eval,c1,l in zip(trainers,evals,colors,labels):

        if len(eval.get_weight())==0: eval.set_weight(np.ones((len(eval.get_label()),)))
        true_test_sig = eval.get_label()==1
        true_test_bkg = ~true_test_sig

        sig_eff_test = []
        bkg_rej_test = []
        preds_test = trainer.bst.predict(eval)

        w = eval.get_weight()
        w_sig = np.sum(eval.get_weight()[true_test_sig])
        w_bkg=np.sum(eval.get_weight()[true_test_bkg])

        @jit(nopython=True,parallel=True)
        def compute_ROC_point(d):
            pred_test_sig = preds_test>d
            pred_test_bkg = ~pred_test_sig
            return (np.sum(w[true_test_sig & pred_test_sig])/w_sig,
                    np.sum(w[true_test_bkg & pred_test_bkg])/w_bkg)

        sig_eff_test,bkg_rej_test = np.vectorize(compute_ROC_point)(dis)        
        ax.scatter(sig_eff_test, bkg_rej_test, color =c1, zorder=0, label = l+" test")
        ax.plot(sig_eff_test, bkg_rej_test, lw=1.3, color = c1)
    
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


def train_prompt_Jpsi():
    """
    Just to keep the main clean
    """
    def nonPrompt_tail(x,N,b):
        return  N*expon.pdf(x,0,b)

    modelname = "forest_prompt"

    Jpsi_trainer = Trainer("Jpsi", modelname)
    Jpsi_trainer.load_data()

    #import reweighing parameters
    lxy_cutoff = Jpsi_trainer.particle_config["models"][modelname]["reweighing"]["lxy_cutoff"]
    nbins_fit = Jpsi_trainer.particle_config["models"][modelname]["reweighing"]["nbins_fit"]
    nbins_corrections = Jpsi_trainer.particle_config["models"][modelname]["reweighing"]["nbins_corrections"]

    #Define data and compute weights
    data_prompt = Jpsi_trainer.full_mass_range[Jpsi_trainer.full_mass_range["Mm_kin_lxy"]<lxy_cutoff]
    weights = Jpsi_trainer.compute_reweight(data_prompt,'Mm_kin_lxy', nonPrompt_tail,nbins_fit=nbins_fit,nbins_corrections=nbins_corrections,fit_range=(0.1,0.5),fitting_limits=[(0,1)],plot=True)
    weights = np.where(weights<0, 0, weights)

    #Perform training and plot
    Jpsi_trainer.prepare_training_set(data_override=data_prompt, weights=weights)
    Jpsi_trainer.train_model()
    Jpsi_trainer.plot_model(saveas=config["locations"]["public_html"]+"BDTs/Jpsi_"+modelname+".png")

    return 

if __name__ == "__main__":
    train_prompt_Jpsi()

    # Y_trainer = Trainer("Y", 'forest_ID')
    # Y_trainer.complete_train()
    # Y_trainer.plot_model() #saveas=config["locations"]["public_html"]+"BDTs/Y_forest_standard.png"

    # Y_trainer = Trainer("Y", 'tree_standard')
    # Y_trainer.complete_train()
    # Y_trainer.plot_model() #saveas=config["locations"]["public_html"]+"BDTs/Y_forest_standard.png"

    # Jpsi_trainer = Trainer("Jpsi")
    # Jpsi_trainer.complete_train()
    