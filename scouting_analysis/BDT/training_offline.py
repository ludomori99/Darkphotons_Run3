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
import subprocess
import yaml
from numba_stats import expon # crystalball,crystalball_ex, norm, expon, voigt, truncexpon
from iminuit.cost import LeastSquares #,  ExtendedBinnedNLL,ExtendedUnbinnedNLL,
from iminuit import Minuit
from sklearn.metrics import roc_auc_score
from numba import jit,njit,vectorize

"""
Define imports, variables, and functions for later use

"""

HOME_USER = os.environ.get("HOMELUDO", None)
DP_USER = os.environ.get("DPUSER", None)
OFFLINE_FOLDER = DP_USER+"/pull_data/offline/" if DP_USER else None

def load_analysis_config():
    try:
        with open(os.path.join(DP_USER,"config/analysis_config.yml"), "r") as f:
            config = yaml.load(f, Loader=yaml.FullLoader)
        return config

    except Exception as e: 
        raise e

config = load_analysis_config()


class Trainer:

    def __init__(self, particle, modelname = None) -> None:
        self.particle = particle #the particle on which training was performed
        self.particle_config = config["BDT_training"][particle]
        if modelname is not None: self.modelname = modelname
        else: self.modelname = self.particle_config["BDT_training"]["modelname"]
        return
    
    def load_data(self, data_particle = None,include_MC=False,signal_indices=None,**kwargs):
        print("\n\n\nStart loading data")
        if data_particle: self.off_dir = config["locations"]["offline"][data_particle]
        else :  self.off_dir = config["locations"]["offline"][self.particle]
        self.filename=self.off_dir+"merged_A.root"
        self.full_mass_range = up.open(self.filename + ":tree").arrays(library = 'pd')
        self.mass = self.full_mass_range["Mm_mass"]
        print(f"Successfully imported data file {self.filename} to memory")

        if include_MC: 
            print("Start loading MC data")
            if data_particle: self.MC_dir = config["locations"]["MC_InclusiveMinBias"][data_particle]
            else :  self.MC_dir = config["locations"]["MC_InclusiveMinBias"][self.particle]
            peak_index = ""
            if signal_indices is not None: 
                peak_index="Y"
                for i in signal_indices:
                    peak_index += str(i)
            self.MC_filename=self.MC_dir+"merged"+peak_index+"_A.root"
            self.MC_full_mass_range = up.open(self.MC_filename + ":tree").arrays(library = 'pd')
            self.MC_mass = self.MC_full_mass_range["Mm_mass"]
            print(f"Successfully imported data file {self.MC_filename} to memory")
        return
    
    def prepare_training_set(self,data_particle = None, prompt_reweight=False, w_frac_bkg=0.1, signal_indices=None, plot_reweight=False, **kwargs):
        """
        data_particle: if want to evaluate on particle different from particle from training given in __init__
        signal_indices: array like [1,2] to say to get just Y1 and Y2 
        """

        #Define signal and background
        if signal_indices is not None: 
            sig_lims = np.array(self.particle_config["limits"]["signal"])[[i-1 for i in signal_indices]] if not data_particle else np.array(config["BDT_training"][data_particle]["limits"]["signal"])[[i-1 for i in signal_indices]]
        else:
            sig_lims = self.particle_config["limits"]["signal"] if not data_particle else config["BDT_training"][data_particle]["limits"]["signal"]
        bkg_lims = self.particle_config["limits"]["background"] if not data_particle else config["BDT_training"][data_particle]["limits"]["background"] 

        print("sig_lims:", sig_lims)

        if self.modelname is None: self.modelname = self.particle_config["modelname"]
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

        sig = pd.DataFrame(self.full_mass_range[sig_cut]).reset_index(drop=True)
        bkg = pd.DataFrame(self.full_mass_range[bkg_cut]).reset_index(drop=True)


        #Define data and compute weights
        if prompt_reweight:
            lxy_cutoff = self.particle_config["models"][self.modelname]["reweighing"]["lxy_cutoff"]
            nbins_fit = self.particle_config["models"][self.modelname]["reweighing"]["nbins_fit"]
            nbins_corrections = self.particle_config["models"][self.modelname]["reweighing"]["nbins_corrections"]

            sig = sig[sig["Mm_kin_lxy"]<lxy_cutoff].reset_index(drop=True)
            sig_prompt_w = self.compute_reweight(sig,'Mm_kin_lxy', nonPrompt_tail,nbins_fit=nbins_fit,nbins_corrections=nbins_corrections,fit_range=(0.1,0.5),fitting_limits=[(0,1)],plot_reweight=plot_reweight,**kwargs)


        sig['Score'] = 1
        bkg['Score'] = 0
        self.sig_frac = -1 if (len(sig)+len(bkg)) == 0 else len(sig)/(len(sig)+len(bkg))

        def slice_df(df,N,seed=42):
            if len(df)<N:
                return df
            return df.sample(n=N, random_state=seed)

        #discard random events to equalize sig and bkg 
        if len(sig)>len(bkg):
            print(f"Threw {(len(sig)-len(bkg))/len(sig)} of signal events")
            sig = slice_df(sig,len(bkg))
        else:
            print(f"Threw {-(len(sig)-len(bkg))/len(bkg)} of bkg events")
            bkg = slice_df(bkg,len(sig))

        sampled_indices = sig.index

        self.sig=sig #added these two lines in second moment just to look at masses, maybe will remove
        self.bkg=bkg

        self.trainData = pd.concat([sig,bkg]).reset_index(drop=True)
        self.trainData_skinny = self.trainData[self.train_vars]
        self.full_mass_range_skinny = self.full_mass_range[self.train_vars]

        if prompt_reweight:
            sig_weights = sig_prompt_w[sampled_indices]
            bkg_weights = w_frac_bkg*np.ones((len(bkg),))
            weights = np.concatenate([sig_weights,bkg_weights])     
            assert len(weights) == len(self.trainData)
            self.weights=weights
            self.sig_weight_frac=np.round(np.sum(sig_weights)/(np.sum(sig_weights)+np.sum(bkg_weights)),2)
            print(f"Total signal events: {len(sig)}\nTotal signal weight: {np.sum(sig_weights)}, {self.sig_weight_frac} of total weight")
        else: weights = None

        
        #NB: train_test_split conserves row indexing throughout splitting. Indexing range remains that of trainData (meaning X_train and X_val will be smaller than trainData but retain the corresponding indices)

        X_temp, self.X_test, y_temp, self.y_test = train_test_split(self.trainData_skinny, self.trainData["Score"], test_size=0.05, shuffle=True, random_state=random_seed)
        self.X_train, self.X_val, self.y_train, self.y_val = train_test_split(X_temp, y_temp, test_size=0.3, shuffle=True, random_state=random_seed)
        self.dtrain = xgb.DMatrix(self.X_train, label=self.y_train, weight = weights[self.X_train.index] if weights is not None else None) 
        self.dval = xgb.DMatrix(self.X_val, label=self.y_val, weight = weights[self.X_val.index] if weights is not None else None)
        print(f"Defined training and evaluation datasets")

    def train_model(self,name_extra="",**kwargs):
        s=np.sum(self.y_train==1)
        b=np.sum(self.y_train==0)
        print(f"Train on {len(self.X_train)} events, of which an approximate fraction {s/(s+b)} is signal, and {len(self.train_vars)} variables")
        num_round = self.particle_config["models"][self.modelname]["num_rounds"]
        evallist = [(self.dtrain, 'train'), (self.dval, 'eval')]
        self.bst = xgb.train(self.hyperpars, self.dtrain, num_round, evals=evallist)
        self.bst.save_model(os.path.join(DP_USER,"BDT/trained_models/", self.modelname+"_"+self.particle+name_extra+".json"))
        print("Training successful, model saved to file " + os.path.join(DP_USER,"BDT/trained_models/", self.modelname+"_"+self.particle+name_extra+".json"))
        return

    def training_metadata(self):
        #not used 
        self.test_bkg=self.bst.predict(xgb.DMatrix(self.X_test[self.y_test==0]))
        self.val_sig=self.bst.predict(xgb.DMatrix(self.X_val[self.y_val==1]))
        d = {}
        return 

    def load_model(self,name_extra="",**kwargs):
        self.bst = xgb.Booster()
        print("loading model ",os.path.join(DP_USER,"BDT/trained_models/", self.modelname+"_"+self.particle+name_extra+".json"))
        try:
            self.bst.load_model(os.path.join(DP_USER,"BDT/trained_models/", self.modelname+"_"+self.particle+name_extra+".json"))
            print("loading successful")
        except Exception as e:
            print(e)
        return 
    
    def complete_train(self,include_MC=False,**kwargs):
        #out-of-the-box for training
        self.load_data(include_MC=include_MC,**kwargs)
        self.prepare_training_set(**kwargs)
        self.train_model(**kwargs)

    def complete_load(self, data_particle = None, include_MC=False, **kwargs):
        self.load_data(data_particle,include_MC=include_MC,**kwargs)
        self.prepare_training_set(data_particle,**kwargs)
        self.load_model(**kwargs)

    def plot_model(self,plot_training=False,plot_MC=False, apply_weights=False,density=True,compute_optimal_cut=True,**kwargs):

        if plot_MC and 'MC_full_mass_range' not in dir(self): 
            print("ERROR: Cannot plot MC, must load by using load_data(include_MC=True)")
            plot_MC=False

        self.val_bkg=self.bst.predict(xgb.DMatrix(self.X_val[self.y_val==0]))
        self.val_sig=self.bst.predict(xgb.DMatrix(self.X_val[self.y_val==1]))
        self.train_bkg=None
        self.train_sig=None
        self.val_MC=None
        if plot_training:
            self.train_bkg=self.bst.predict(xgb.DMatrix(self.X_train[self.y_train==0]))
            self.train_sig=self.bst.predict(xgb.DMatrix(self.X_train[self.y_train==1]))
        if plot_MC:
            self.val_MC=self.bst.predict(xgb.DMatrix(self.MC_full_mass_range[self.train_vars]))

        data_to_plot = [self.val_bkg,self.val_sig]+plot_training*[self.train_bkg,self.train_sig]+plot_MC*[self.val_MC]
        labels = ["Bkg.","Sig."]+plot_training*["Training bkg.","Train sig."]+plot_MC*["MC sig."]
        weights=None

        s=np.sum(self.y_train==1)
        b=np.sum(self.y_train==0)
        text=f"N={len(self.X_train)}\nSig. frac.={np.round(s/(s+b),2)}" 

        if apply_weights:
            weights = [self.weights[self.X_val.index[self.y_val==0]],self.weights[self.X_val.index[self.y_val==1]]] + plot_training*[self.weights[self.X_train.index[self.y_train==0]],self.weights[self.X_train.index[self.y_train==1]]]
            if plot_MC: 
                weights+=[self.MC_full_mass_range["weights_prompt"]]

        if compute_optimal_cut:
            if apply_weights:
                    def significance(dis_lim):
                        sig = self.weights[self.X_val.index[self.y_val==1]][self.val_sig > dis_lim]
                        s = np.sum(sig)
                        bkg = self.weights[self.X_val.index[self.y_val==0]][self.val_bkg > dis_lim]
                        b = np.sum(bkg)
                        return s/np.sqrt(b) if b!= 0 else 0
            else: 
                def significance(dis_lim):
                        sig = self.val_sig > dis_lim
                        s = np.sum(sig)
                        bkg = self.val_bkg > dis_lim
                        b = np.sum(bkg)
                        return s/np.sqrt(b) if b!= 0 else 0
            vtx_vals= np.linspace(0.3,0.9,500)
            significance_vals = np.vectorize(significance)(vtx_vals)
            max_idx = np.argmax(significance_vals)
            max_cut = vtx_vals[max_idx]
            # max_significance = significance_vals[max_idx]
            if apply_weights:
                sig_eff = np.sum(self.weights[self.X_val.index[self.y_val==1]][self.val_sig > max_cut])/np.sum(self.weights[self.X_val.index[self.y_val==1]])
                bkg_rej = 1-np.sum(self.weights[self.X_val.index[self.y_val==0]][self.val_bkg > max_cut])/np.sum(self.weights[self.X_val.index[self.y_val==0]])
                ROC_score = roc_auc_score(np.concatenate([self.y_val[self.y_val==0],self.y_val[self.y_val==1]]),
                                          np.concatenate([self.val_bkg,self.val_sig]),
                                          sample_weight=np.concatenate([self.weights[self.X_val.index[self.y_val==0]],self.weights[self.X_val.index[self.y_val==1]]]))
            else:
                sig_eff = np.sum(self.val_sig > max_cut)/len(self.val_sig)
                bkg_rej = 1-np.sum(self.val_bkg > max_cut)/len(self.val_bkg)
                ROC_score = roc_auc_score(np.concatenate([self.y_val[self.y_val==0],self.y_val[self.y_val==1]]),np.concatenate([self.val_bkg,self.val_sig]))
            text+=f"\nmax cut={round(max_cut,3)}\nsig. eff.={round(sig_eff,3)}\nbkg. rej.={round(bkg_rej,3)}\nROC area={round(ROC_score,3)}"

        self.plot_hist(data_to_plot,labels,weights = weights,density=density, text=text, xlabel="BDT score",**kwargs)
        
        if compute_optimal_cut and apply_weights: return ROC_score, sig_eff, bkg_rej

    def plot_mass(self,plot_training=False,plot_MC=False,apply_weights=False,density=True,**kwargs):
        if plot_MC and 'MC_full_mass_range' not in dir(self):
            print("requested to plot MC but have not loaded it before. please include flag include_MC=True in load_data or complete_load.")
            return
 
        data_to_plot = [self.bkg,self.sig]+plot_MC*[self.MC_mass]
        labels = ["Bkg.","Sig."]+plot_MC*["MC sig."]
        weights=None
        if apply_weights:
            weights = ([self.weights[self.X_val.index[self.y_val==0]],self.weights[self.X_val.index[self.y_val==1]]] + 
                        plot_MC*[self.MC_full_mass_range["weights_prompt"]])

        self.plot_hist(data_to_plot,labels,weights = weights,density=density, xlabel="masses",**kwargs)
       

    @staticmethod
    def compute_reweight(data,variable,fitting_func, nbins_fit = 100, nbins_corrections=100, fit_range = None, fitting_limits=None, plot_reweight=False,saveas=None, nonnegative=True, xrange=(0,0.5), plot_logscale=False):
        """
        data: ak array, dic-like, containing all events
        variable: string, name of var to compute the reweighing. e.g.: Mm_kin_lxy
        fitting_func: function to fit to histogram in range fit_range. signature f(variable,N,*args) w/ N normalization
        nbins: int, obv
        range: range we compute the histogram
        fitting_limits: array of 2-tuples, with length #args - 2
        """

        n_vars_fit_func = fitting_func.__code__.co_argcount   
        assert  type(n_vars_fit_func)==int and n_vars_fit_func > 1 , "fit function has too few arguments"
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

        if nonnegative: weights = np.where(weights<0, 0, weights)

        print(f"""\nSome General infos:\n
                Total number of events considered: {len(data)} \n
                Sum of weights: {np.sum(weights)} \n
                Fitted prompt: {prompt}\n
                Sum of weights>0 {np.sum(weights[weights>=0])}\n\n""")

        if plot_reweight:
            hep.style.use("CMS")
            fig, ax = plt.subplots(figsize=(10,8))
            x = np.linspace(0,xe_corr[-1], 1000)
            ax.plot(x,fitting_func(x,*mData.values), label = "Nonprompt tail fit")
            # ax.errorbar(0.5*(xe_corr[:-1] + xe_corr[1:]),histSlxy-fitting_func(0.5*(xe_corr[:-1] + xe_corr[1:]),*mData.values)/dx_fit*dx_corr,xerr = dx_corr/2, label="$J/\psi$ prompt data", color = "green", zorder=0,marker = '.')
            ax.hist(data["Mm_kin_lxy"], nbins_corrections,   label="$J/\psi$ data", color = "orange", zorder=0, histtype='step', linewidth = 1.8)
            ax.hist(data["Mm_kin_lxy"], nbins_corrections,  weights=weights, label="$J/\psi$ data reweighed", color = "brown", zorder=0, histtype='step', linewidth = 1.8)
            # ax.hist(data["Mm_kin_lxy"], nbins_corrections,  weights = (data["Mm_kin_lxy"]>fit_range[0]), color = "purple", zorder=0, histtype='bar', linewidth = 1.8, alpha = .2)
            # ax.text(0.2,0.9, f"J/$\psi$ data, sPlot-unfolded $l_{{xy}}$ distribution \nShaded area is nonprompt tail", fontsize = 12, transform=ax.transAxes)
            ax.grid()
            ax.legend(fontsize=13,frameon=True,edgecolor='black',fancybox=False)    
            if xrange is not None: ax.set_xlim(xrange)
            # ax.set_ylim(1e-2,1e6)
            ax.set_xlabel("$l_{xy}$")
            # ax.set_ylim(0,20000)
            if plot_logscale: ax.set_yscale('log')
            ax.set_ylabel("Frequency")    
            plt.show()
            if saveas is not None: 
                plt.savefig(saveas)
                print(f"saved {saveas}")
        
        return weights
    
    @staticmethod
    def save_weight_to_tree(data_path,weights,weight_name):
        if not os.path.isabs(data_path):
            raise ValueError("Input path is not a full path.")
        data_file=up.open(data_path)["tree"].arrays()
        len_tree=len(data_file["Mm_mass"])
        if(len_tree != len(weights)): 
            print("error: weights and data have a different shape")
            return
        data_file[weight_name]=weights
        with up.recreate(data_path) as output_file:
            output_file["tree"] = data_file 
        return
    

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
        if text!=None: ax.text(0.02, .6, text, fontsize=13, bbox=dict(facecolor='white', edgecolor='black'), transform=ax.transAxes) 
        if int_xticks: ax.xaxis.get_major_locator().set_params(integer=True)
        ax.set_ylabel('Normalized frequency')
        ax.set_xlim(xlim)
        ax.legend(fontsize=13,frameon=True,edgecolor='black',fancybox=False)
        ax.grid(True)
        if saveas: 
            plt.savefig(saveas)
            print(f"saved figure as {saveas}")
        return
    
    @staticmethod
    def plot_scatter(xs,ys,names, xlabel = None, ylabel=None, saveas=None, text = None, xlim=None, log = False, int_xticks = False):
        hep.style.use("CMS")
        colors = plt.cm.tab10.colors
        fig, ax = plt.subplots(figsize=(12,9))
        hep.cms.text("Preliminary")
        for x,y,name,c in zip(xs,ys,names,colors[:len(xs)]):
                ax.scatter(x,y, label=name, color=c)
                ax.plot(x,y, color=c,alpha=0.5)
        if (xlabel): ax.set_xlabel(xlabel)
        if (ylabel): ax.set_ylabel(ylabel)
        if text!=None: ax.text(0.02, .8, text, fontsize=13, bbox=dict(facecolor='white', edgecolor='black'), transform=ax.transAxes) 
        if int_xticks: ax.xaxis.get_major_locator().set_params(integer=True)
        ax.set_xlim(xlim)
        ax.legend(fontsize=13,frameon=True,edgecolor='black',fancybox=False)
        ax.grid(True)
        if saveas: 
            plt.savefig(saveas)
            print(f"saved figure as {saveas}")
        return
    
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
        ax.scatter(sig_eff_test, bkg_rej_test, color =c1, zorder=0, label = l)
        ax.plot(sig_eff_test, bkg_rej_test, lw=1.3, color = c1)
    
    ax.set_xlabel('Signal efficiency')
    ax.set_ylabel('Background rejection')
    ax.grid(True)

    if tmva: 
        ax.scatter(tmva["sig_eff_test_tmva"], tmva["bkg_rej_test_tmva"], color = "blue", zorder=0, label = "TMVA test")
        ax.plot(tmva["sig_eff_test_tmva"], tmva["bkg_rej_test_tmva"], lw=1.3, color = "blue")
        ax.scatter(tmva["sig_eff_train_tmva"], tmva["bkg_rej_train_tmva"], color ="red", zorder=0, label = "TMVA train")
        ax.plot(tmva["sig_eff_train_tmva"], tmva["bkg_rej_train_tmva"], lw=1.3, color = "red")
    
    ax.legend(fontsize=13,frameon=True,edgecolor='black',fancybox=False)
    plt.show()
    return 

def nonPrompt_tail(x,N,b):
    return  N*expon.pdf(x,0,b)

def train_prompt_Jpsi():
    """
    Just to keep the main clean. Essentially the same as a complete_train call, however taking the prompt corrections into account 
    (and storing weights into tree file)
    """
    modelname = "forest_prompt"
    particle = "Jpsi"

    Jpsi_trainer = Trainer(particle, modelname)
    Jpsi_trainer.load_data()

    #Perform training and plot with different w parameters
    w_bkg = np.linspace(0.1,1,21)
    roc_scores = []
    sig_eff_at_wp = []
    bkg_rej_at_wp = []
    for w in w_bkg:   
        Jpsi_trainer.prepare_training_set(prompt_reweight=True, w_frac_bkg=w)
        Jpsi_trainer.train_model(name_extra=f"_w={np.round(w,3)}")
        roc_score, sig_eff, bkg_rej = Jpsi_trainer.plot_model(saveas=config["locations"]["public_html"]+f"BDTs/{particle}/{particle}_{modelname}_w={np.round(w,3)}.png",apply_weights=True,compute_optimal_cut=True)
        roc_scores.append(roc_score)
        sig_eff_at_wp.append(sig_eff)
        bkg_rej_at_wp.append(bkg_rej)
    Jpsi_trainer.plot_scatter([w_bkg,w_bkg,w_bkg],[roc_scores,sig_eff_at_wp,bkg_rej_at_wp], ["ROC AUC", "Sig. eff. at WP", "Bkg. rej. at WP"],xlabel="Bkg. weight at training",saveas=config["locations"]["public_html"]+"BDTs/w_ROC_Jpsi_"+modelname+".png")
    return 


def save_Jpsi_weights():

    ##### First process data. use lxy cutoff ######

    modelname = "forest_prompt"
    particle = "Jpsi"

    Jpsi_trainer = Trainer(particle, modelname)
    Jpsi_trainer.load_data()

    #import reweighing parameters
    lxy_cutoff = Jpsi_trainer.particle_config["models"][modelname]["reweighing"]["lxy_cutoff"]
    nbins_fit = Jpsi_trainer.particle_config["models"][modelname]["reweighing"]["nbins_fit"]
    nbins_corrections = Jpsi_trainer.particle_config["models"][modelname]["reweighing"]["nbins_corrections"]

    #Define data and compute weights
    data_prompt = Jpsi_trainer.full_mass_range[Jpsi_trainer.full_mass_range["Mm_kin_lxy"]<lxy_cutoff]
    weights = Jpsi_trainer.compute_reweight(data_prompt,
                                            'Mm_kin_lxy', 
                                            nonPrompt_tail,
                                            nbins_fit=nbins_fit,nbins_corrections=nbins_corrections,
                                            fit_range=(0.1,0.5),fitting_limits=[(0,1)],nonnegative=True,plot_reweight=True,saveas=config["locations"]["public_html"]+f"BDTs/lxy_reweight.png")
    weights_extended = np.zeros_like(Jpsi_trainer.full_mass_range["Mm_mass"])
    weights_extended[Jpsi_trainer.full_mass_range["Mm_kin_lxy"]<lxy_cutoff] = weights
    Jpsi_trainer.save_weight_to_tree(Jpsi_trainer.filename, weights_extended,"weights_prompt")

    ##### Now for MC ######

    MC_file_name = os.path.join(config["locations"]["MC_InclusiveMinBias"]["Jpsi"], "merged_A.root")
    MC_file=up.open(MC_file_name)
    MC_data = MC_file["tree"].arrays(library = 'pd')    #import reweighing parameters

    lxy_cutoff = config["BDT_training"]["Jpsi"]["models"]["forest_prompt"]["reweighing"]["lxy_cutoff"]
    nbins_fit = config["BDT_training"]["Jpsi"]["models"]["forest_prompt"]["reweighing"]["nbins_fit"]
    nbins_corrections = config["BDT_training"]["Jpsi"]["models"]["forest_prompt"]["reweighing"]["nbins_corrections"]

    #Define data and compute weights
    MC_data_prompt=MC_data[MC_data["Mm_kin_lxy"]<lxy_cutoff]
    weights = Trainer.compute_reweight(MC_data_prompt,
                                            'Mm_kin_lxy', 
                                            nonPrompt_tail,
                                            nbins_fit=nbins_fit,nbins_corrections=nbins_corrections,
                                            fit_range=(0.1,0.5),fitting_limits=[(0,1)],nonnegative=True,plot_reweight=False)
    
    weights_extended = np.zeros_like(MC_data["Mm_mass"])
    weights_extended[MC_data["Mm_kin_lxy"]<lxy_cutoff] = np.array(weights)
    Trainer.save_weight_to_tree(MC_file_name, weights_extended,"weights_prompt")
    return

def add_branch_weights_prompt():
    """
    Add branch "weights_prompt" to Y data and MC. quick fix to be able to work with sPlot consistently
    Effectively adding constant weights to Y events (since Y already prompt) to have the name in 
    """
    MC_file_name = os.path.join(config["locations"]["MC_InclusiveMinBias"]["Y"], "merged_A.root")
    MC_file=up.open(MC_file_name)
    MC_data = MC_file["tree"].arrays(library = 'pd')    #import reweighing parameters
    weights_extended = np.ones_like(MC_data["Mm_mass"])
    Trainer.save_weight_to_tree(MC_file_name, weights_extended,"weights_prompt")

    off_file_name = os.path.join(config["locations"]["offline"]["Y"], "merged_A.root")
    off_file=up.open(off_file_name)
    off_data = off_file["tree"].arrays(library = 'pd')    #import reweighing parameters
    weights_extended = np.ones_like(off_data["Mm_mass"])
    Trainer.save_weight_to_tree(off_file_name, weights_extended,"weights_prompt")
    return



if __name__ == "__main__":
    print("Executing training block")
    # train_prompt_Jpsi()
    # save_Jpsi_weights()
    # add_branch_weights_prompt()

    # Y_trainer = Trainer("Y", 'forest_ID')
    # Y_trainer.complete_load(signal_indices=[1],include_MC=True)
    # Y_trainer.plot_model(plot_MC=True, saveas=config["locations"]["public_html"]+"BDTs/Y/Y_forest_standard_ID.png")

    # Y_trainer = Trainer("Y", 'tree_standard')
    # Y_trainer.complete_load(name_extra='1',signal_indices=[1],include_MC=True)
    # Y_trainer.plot_model(plot_MC=True, saveas=config["locations"]["public_html"]+"BDTs/Y/Y1_tree_standard.png")

    # Y_trainer = Trainer("Y", 'forest_standard')
    # Y_trainer.complete_load(name_extra='1',signal_indices=[1],include_MC=True)
    # Y_trainer.plot_model(plot_MC=True, saveas=config["locations"]["public_html"]+"BDTs/Y/Y1_forest_standard.png")

    # Y_trainer = Trainer("Y", 'forest_standard')
    # Y_trainer.complete_load(name_extra='2',signal_indices=[2],include_MC=True)
    # Y_trainer.plot_model(plot_MC=True, saveas=config["locations"]["public_html"]+"BDTs/Y/Y2_forest_standard.png")

    # Y_trainer = Trainer("Y", 'forest_standard')
    # Y_trainer.complete_load(name_extra='3',signal_indices=[2],include_MC=True)
    # Y_trainer.plot_model(plot_MC=True, saveas=config["locations"]["public_html"]+"BDTs/Y/Y3_forest_standard.png")

    # Y_trainer = Trainer("Y", 'forest_standard')
    # Y_trainer.complete_load(name_extra='12',signal_indices=[1,2],include_MC=True)
    # Y_trainer.plot_model(plot_MC=True, saveas=config["locations"]["public_html"]+"BDTs/Y/Y12_forest_standard.png")

    # Y_trainer = Trainer("Y", 'forest_standard')
    # Y_trainer.complete_load(name_extra='13',signal_indices=[1,3],include_MC=True)
    # Y_trainer.plot_model(plot_MC=True, saveas=config["locations"]["public_html"]+"BDTs/Y/Y13_forest_standard.png")

    # Y_trainer = Trainer("Y", 'forest_standard')
    # Y_trainer.complete_load(name_extra='123',signal_indices=[1,2,3],include_MC=True)
    # Y_trainer.plot_model(plot_MC=True, saveas=config["locations"]["public_html"]+"BDTs/Y/Y123_forest_standard.png")

    # Jpsi_trainer = Trainer("Jpsi",'forest_standard')
    # Jpsi_trainer.complete_load()
    # Jpsi_trainer.plot_model(plot_MC=True, saveas=config["locations"]["public_html"]+"BDTs/Jpsi/Jpsi_forest_standard.png")

    # Jpsi_prompt_trainer = Trainer("Jpsi",'forest_prompt')
    # Jpsi_prompt_trainer.complete_load(prompt_reweight=True, w_frac_bkg=0.325, include_MC=True)
    # Jpsi_prompt_trainer.plot_model(plot_MC=True, saveas=config["locations"]["public_html"]+"BDTs/Jpsi/Jpsi_forest_prompt.png",apply_weights=True,compute_optimal_cut=True)

    # Jpsi_trainer = Trainer("Jpsi",'forest_prompt_noPromptCut')
    # Jpsi_trainer.complete_train(prompt_reweight=True, w_frac_bkg=0.325,include_MC=True)
    # Jpsi_trainer.plot_model(plot_MC=True, saveas=config["locations"]["public_html"]+"BDTs/Jpsi/Jpsi_forest_prompt_noPromptCut.png",apply_weights=True,compute_optimal_cut=True)
    