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
from numba import jit,njit,vectorize
from typing import Literal

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


class Systematics:

    def __init__(self,data,MC,particle_name="") -> None:
        self.data=data
        self.MC=MC
        self.particle_name=particle_name
        self.corrections={"data":{},"MC":{}}
        return
    
    def define_cuts(self, BDT_cut,ID_cut):
        self.BDT_cut=BDT_cut
        self.ID_cut=ID_cut

    def compute_corrections_MC(self,variable,hist_range=None,plot=False):
        """
        e.g. to correct MC based on eta distribution
        """
        nbins_correction = 20
        histSvar,_ = np.histogram(self.data[variable],nbins_correction,weights=self.data["sigYield_sw"],range=hist_range,density=True) #range=(-8.38,6.9),-
        histMCvar,edges = np.histogram(self.MC[variable],nbins_correction, weights = self.MC["sigYield_sw"],range=hist_range,density=True)
        weight_hist = np.where(histMCvar == 0, 0, histSvar/histMCvar)  

        if plot:
            hep.style.use("CMS")
            fig, ax = plt.subplots(figsize=(10,6))
            ax.scatter(0.5*(edges[:-1] + edges[1:]),histSvar, label = "Signal variable" )
            ax.scatter(0.5*(edges[:-1] + edges[1:]),histMCvar, label = "MC variable")
            ax.plot(0.5*(edges[:-1] + edges[1:]),histSvar,)
            ax.plot(0.5*(edges[:-1] + edges[1:]),histMCvar,)
            # ax.scatter(0.5*(edges[:-1] + edges[1:]),weight_hist,)
            ax.legend()

        corrections_var = weight_hist[np.digitize(self.MC[variable], edges) - 1]
        corrections_var = corrections_var/np.sum(corrections_var)
        self.corrections['MC'][variable]=corrections_var
    
    def study_efficiency(self,BDT_model,MC_correction_var=None,nbins=100):
        self.nbins=nbins
        self.BDT_model=BDT_model
        print(f"Study efficiency of the BDT {BDT_model} on the {self.particle_name} peak\nID cut: {self.ID_cut}\nBDT cut: {self.BDT_cut}")
        #The BDT results before the cut
        self.scoreData = self.data[BDT_model+"_mva"]
        self.scoreMC = self.MC[BDT_model+"_mva"]

        #The total signal yields as for the sPlot weights before cuts
        dataTotalYield = np.sum(self.data["sigYield_sw"])
        MCTotalYield = np.sum(self.MC["sigYield_sw"])
        print("Total data signal yield: ", dataTotalYield)
        print("Total MC signal yield: ", MCTotalYield, "\n\n")

        #compute corrected weights
        self.normalized_sweights_data_bkg = self.data["bkgYield_sw"]/np.sum(self.data["bkgYield_sw"])
        self.normalized_sweights_data_signal = self.data["sigYield_sw"]/np.sum(self.data["sigYield_sw"])
        self.normalized_sweights_MC = self.MC["sigYield_sw"]/np.sum(self.MC["sigYield_sw"])

        # combined_weights_mean = 0.5*(self.corrections*MCTotalYield + self.MC["sigYield_sw"]) #Arithmetic mean as alternative
        
        # combine sweights with prompt corrections. Normalize to 1
        self.combined_weights_data = (self.data["weights_prompt"]*self.normalized_sweights_data_signal)/np.sum(self.data["weights_prompt"]*self.normalized_sweights_data_signal) #Normalized product
        self.combined_weights_MC = (self.MC["weights_prompt"]*self.normalized_sweights_MC)/np.sum(self.MC["weights_prompt"]*self.normalized_sweights_MC) #Normalized product
        
        if MC_correction_var is not None:
            self.combined_weights_MC=self.combined_weights_MC*self.corrections["MC"][MC_correction_var]/np.sum(self.combined_weights_MC*self.corrections["MC"][MC_correction_var])

        #build histograms with score distributions before cuts
        histYB,_ = np.histogram(self.scoreData,nbins,(0,1),False,self.data["bkgYield_sw"])
        histYS,self.xe = np.histogram(self.scoreData,nbins,(0,1),False,self.data["sigYield_sw"])
        histYMC,_ = np.histogram(self.scoreMC,nbins,(0,1),False, self.MC["sigYield_sw"])
        histYMC_corrected,_ = np.histogram(self.scoreMC,nbins,(0,1),False,self.combined_weights_MC)

        self.dx = self.xe[1]-self.xe[0]
        self.x = 0.5*(self.xe[:-1]+self.xe[1:])

        #The event cuts on the ID
        dataCutOnID = (self.data["Muon_softMva1"] > self.ID_cut) & (self.data["Muon_softMva2"]> self.ID_cut)  
        MCCutOnID = (self.MC["Muon_softMva1"] > self.ID_cut) & (self.MC["Muon_softMva2"] > self.ID_cut)     

        print("Efficiency data ID cut: ", np.sum(self.normalized_sweights_data_signal[dataCutOnID]))
        print("Efficiency prompt data ID cut (only applies to Jpsi): ", np.sum(self.combined_weights_data[dataCutOnID]))
        print("Efficiency MC ID cut: ", np.sum(self.normalized_sweights_MC[MCCutOnID]))
        print("Efficiency corrected MC ID cut (prompt and possible further correction): ", np.sum(self.combined_weights_MC[MCCutOnID]), "\n\n")

        histB_IDcut,_ = np.histogram(self.scoreData[dataCutOnID],nbins,(0,1),False,self.normalized_sweights_data_bkg[dataCutOnID])
        histS_IDcut,_ = np.histogram(self.scoreData[dataCutOnID],nbins,(0,1),False,self.normalized_sweights_data_signal[dataCutOnID])
        histS_corr_IDcut,_ = np.histogram(self.scoreData[dataCutOnID],nbins,(0,1),False,self.combined_weights_data[dataCutOnID])
        histMC_IDcut,_ = np.histogram(self.scoreMC[MCCutOnID],nbins,(0,1),False,self.normalized_sweights_MC[MCCutOnID])
        histMC_corr_IDcut,_ = np.histogram(self.scoreMC[MCCutOnID],nbins,(0,1),False,self.combined_weights_MC[MCCutOnID])

        bkgYield_after_cuts = np.sum(histB_IDcut*(self.x>self.BDT_cut))
        dataYield_after_cuts = np.sum(histS_IDcut*(self.x>self.BDT_cut) )
        data_corr_Yield_after_cuts = np.sum(histS_corr_IDcut*(self.x>self.BDT_cut) )
        MCYield_after_cuts = np.sum(histMC_IDcut*(self.x>self.BDT_cut) )
        MC_corr_Yield_after_cuts = np.sum(histMC_corr_IDcut*(self.x>self.BDT_cut) )

        print(f"dataYield_after_cuts: {dataYield_after_cuts:.2f}")
        print(f"MCYield_after_cuts: {MCYield_after_cuts:.2f} \n\n")
        print(f"MC corr Yield_after_cuts {MC_corr_Yield_after_cuts:.2f} \n\n")

        self.effData =  dataYield_after_cuts
        self.effData_corr =  data_corr_Yield_after_cuts
        self.effMC = MCYield_after_cuts
        self.effMC_corr = MC_corr_Yield_after_cuts

        print(f"BDT efficiency on Y peak: \n On data: {self.effData} \n On corrected data: {self.effData_corr} \n On MC: {self.effMC} \n On corrected MC: {self.effMC_corr}")
        print(f"Efficiency difference: {self.effMC_corr - self.effData_corr}")

    def plot(self,plot_bkg=False,plot_corr=True,plot_nonPrompt=True):
        bin_cut=np.digitize(self.BDT_cut, self.xe)
        hep.style.use("CMS")
        fig, ax = plt.subplots(figsize=(10,8))
        if plot_bkg: ax.hist(self.scoreData, self.nbins, (0,1),  False, weights =self.normalized_sweights_data_bkg*self.nbins, label="Background", color = "brown", zorder=0, histtype='step', linewidth = 1.8)
        if plot_nonPrompt:
            ax.hist(self.scoreData, self.nbins, (0,1),  False, weights = self.normalized_sweights_data_signal*self.nbins, label="Signal", color = "orange", zorder=0, histtype='step', linewidth = 1.8)
            ax.hist(self.scoreData, self.nbins, (0,1),  False, weights = self.normalized_sweights_data_signal*(self.scoreData>self.xe[bin_cut])*self.nbins, color = "orange", zorder=0, histtype='bar', linewidth = 1.8, alpha = .1)
            ax.hist(self.scoreMC, self.nbins, (0,1),  False, weights =  self.normalized_sweights_MC*self.nbins, label="MC", color = "red", zorder=0, histtype='step', linewidth = 1.8)
            ax.hist(self.scoreMC, self.nbins, (0,1),  False, weights =  self.normalized_sweights_MC*(self.scoreMC>self.xe[bin_cut])*self.nbins,  color = "red", label ='Selection', zorder=0, histtype='bar', linewidth = 1.8 , alpha=.1)
        if plot_corr:
            ax.hist(self.scoreData, self.nbins, (0,1),  False, weights = self.combined_weights_data*self.nbins, label="Signal corrected", color = "blue", zorder=0, histtype='step', linewidth = 1.8)
            ax.hist(self.scoreData, self.nbins, (0,1),  False, weights = self.combined_weights_data*(self.scoreData>self.xe[bin_cut])*self.nbins, color = "blue", zorder=0, histtype='bar', linewidth = 1.8, alpha = .1)
            ax.hist(self.scoreMC, self.nbins, (0,1),  False, weights = self.combined_weights_MC*self.nbins, label="MC corrected", color = "green", zorder=0, histtype='step', linewidth = 1.8)
            ax.hist(self.scoreMC, self.nbins, (0,1),  False, weights = self.combined_weights_MC*(self.scoreMC>self.xe[bin_cut])*self.nbins,  color = "green", zorder=0, histtype='bar', linewidth = 1.8 , alpha=.1)
        text=f"""
            sPlot unfolded BDT distribution 
            Model: {self.BDT_model}
            Data: {self.particle_name}"""+plot_nonPrompt*f"""
            $\epsilon_{{data}} = ${round(self.effData,3)} 
            $\epsilon_{{MC}} = ${round(self.effMC,3)}"""+plot_corr*f"""
            $\epsilon_{{data}}^{{corr}} = ${round(self.effData_corr,3)} 
            $\epsilon_{{MC}}^{{corr}} = ${round(self.effMC_corr,3)}"""
        ax.text(0.25,0.3, text, fontsize=12,  transform=ax.transAxes)
        ax.grid()
        ax.legend()    
        ax.set_xlabel("BDT score")
        ax.set_ylabel("Normalized frequency")

        
    def plot_ROC(self, plot_nonPrompt=True,plot_corr=True, n_points = 50, log = False):
        hep.style.use("CMS")
        colors = ["orange","red","blue","green"]
        fig, ax = plt.subplots(figsize=(9,9))
        hep.cms.text("Preliminary")

        dis = np.linspace(0.001,0.999,n_points)
        bkg = {"data":self.scoreData, "name":"bkg", "weights":self.normalized_sweights_data_bkg}
        nonPrompt=[{"data":self.scoreData, "name":"non-prompt data","weights":self.normalized_sweights_data_signal},
                   {"data":self.scoreMC, "name":"non-prompt MC","weights":self.normalized_sweights_MC}]
        corr=[{"data":self.scoreData, "name":"corrected data","weights":self.combined_weights_data},
              {"data":self.scoreMC, "name":"corrected MC","weights":self.combined_weights_MC}]
        
        signal=plot_nonPrompt*nonPrompt+plot_corr*corr

        output = {}

        for dic,c in zip(signal,colors):
            sig_data = dic["data"]
            sig_data_w = dic["weights"]
            bkg_data = bkg["data"]
            bkg_w = bkg["weights"]
            # @jit(nopython=True,parallel=True)
            def compute_ROC_point(d):
                pred_test_sig = sig_data>d
                pred_test_bkg = bkg_data<d
                return (np.sum(sig_data_w[pred_test_sig]),
                        np.sum(bkg_w[pred_test_bkg]))

            sig_eff_test,bkg_rej_test = np.vectorize(compute_ROC_point)(dis)
            output[dic["name"]]=[sig_eff_test,bkg_rej_test] 
            ax.scatter(sig_eff_test, bkg_rej_test, color =c, zorder=0, label = dic["name"])
            ax.plot(sig_eff_test, bkg_rej_test, lw=1.3, color = c)
        
        ax.set_xlabel('Signal efficiency')
        ax.set_ylabel('Background rejection')
        ax.grid(True)
        ax.legend()
        plt.show()
        return output