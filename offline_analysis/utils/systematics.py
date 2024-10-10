import numpy as np
import mplhep as hep
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import os
import yaml
# from numba import jit,njit,vectorize
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

    def __init__(self,data,MC,BDT_limit,ID_limit,particle_name="",apply_ID=True) -> None:
        self.data=data
        self.MC=MC
        self.ID_cut_MC = (MC["Muon_softMva1"] > ID_limit) & (MC["Muon_softMva2"]> ID_limit) 
        self.ID_cut_data = (data["Muon_softMva1"] > ID_limit) & (data["Muon_softMva2"]> ID_limit) 
        if apply_ID: 
            self.data = data[self.ID_cut_data]
            self.MC = MC[self.ID_cut_MC]
        self.BDT_limit=BDT_limit
        self.ID_limit=ID_limit
        self.particle_name=particle_name
        self.corrections={"data":{},"MC":{}}
        return

    def compute_corrections_MC(self,variable,hist_range=None,plot=False):
        """
        e.g. to correct MC based on eta distribution
        """
        nbins_correction = 20
        histSvar,_ = np.histogram(self.data[variable],nbins_correction,weights=self.data["sigYield_sw"],range=hist_range,density=True) #range=(-8.38,6.9),-
        histMCvar,edges = np.histogram(self.MC[variable],nbins_correction, weights = np.ones_like(self.MC["Mm_mass"]),range=hist_range,density=True)
        weight_hist = np.where(histMCvar == 0, 0, histSvar/histMCvar)  

        if plot:
            hep.style.use("CMS")
            fig, ax = plt.subplots(figsize=(10,6))
            hep.cms.label("Preliminary",data=True,lumi=config["lumi"]["offline"])
            ax.scatter(0.5*(edges[:-1] + edges[1:]),histSvar, label = "Signal variable" )
            ax.scatter(0.5*(edges[:-1] + edges[1:]),histMCvar, label = "MC variable")
            ax.plot(0.5*(edges[:-1] + edges[1:]),histSvar,)
            ax.plot(0.5*(edges[:-1] + edges[1:]),histMCvar,)
            ax.set_ylabel("Relative frequency")
            if variable == "Mm_kin_eta":
                ax.set_xlabel("$\eta$")
            # ax.scatter(0.5*(edges[:-1] + edges[1:]),weight_hist,)
            ax.legend()

        corrections_var = weight_hist[np.digitize(self.MC[variable], edges) - 1]
        corrections_var = corrections_var/np.sum(corrections_var)
        self.corrections['MC'][variable]=corrections_var
    
    def study_efficiency(self,BDT_model,MC_correction_var=None,nbins=100):
        self.nbins=nbins
        self.BDT_model=BDT_model
        print(f"Study efficiency of the BDT {BDT_model} on the {self.particle_name} peak\nID cut: {self.ID_limit}\nBDT cut: {self.BDT_limit}")
        #The BDT results before the cut
        self.scoreData = self.data[BDT_model+"_mva"]
        self.scoreMC = self.MC[BDT_model+"_mva"]

        #The total signal yields as for the sPlot weights before cuts
        dataTotalYield = np.sum(self.data["sigYield_sw"])
        MCTotalYield = np.sum(len(self.MC["Mm_mass"])) #stopped using sweights for MC
        print("Total data signal yield: ", dataTotalYield)
        print("Total MC signal yield: ", MCTotalYield, "\n\n")

        #compute corrected weights
        self.normalized_sweights_data_bkg = np.where(self.data["bkgYield_sw"]<0,0,self.data["bkgYield_sw"])
        self.normalized_sweights_data_bkg = self.normalized_sweights_data_bkg/np.sum(self.normalized_sweights_data_bkg)
        self.normalized_sweights_data_signal = self.data["sigYield_sw"]/np.sum(self.data["sigYield_sw"])
        self.normalized_sweights_MC = np.ones_like(self.MC["Mm_mass"])/len(self.MC["Mm_mass"])#self.MC["sigYield_sw"]/np.sum(self.MC["sigYield_sw"])

        # combined_weights_mean = 0.5*(self.corrections*MCTotalYield + self.MC["sigYield_sw"]) #Arithmetic mean as alternative
        
        # combine sweights with prompt corrections. Normalize to 1
        self.combined_weights_data = (self.data["weights_prompt"]*self.normalized_sweights_data_signal)/np.sum(self.data["weights_prompt"]*self.normalized_sweights_data_signal) #Normalized product
        self.combined_weights_MC = (self.MC["weights_prompt"]*self.normalized_sweights_MC)/np.sum(self.MC["weights_prompt"]*self.normalized_sweights_MC) #Normalized product
        
        self.prompt_signal_norm_data = np.sum(self.data["weights_prompt"]*self.data["sigYield_sw"])
        self.prompt_signal_norm_MC = np.sum(self.MC["weights_prompt"])

        self.MC_label = "MinBias simulation (prompt)"
        self.MC_label2 = "prompt"
        if MC_correction_var is not None:
            self.MC_correction_var=MC_correction_var
            self.combined_weights_MC=self.combined_weights_MC*self.corrections["MC"][MC_correction_var]/np.sum(self.combined_weights_MC*self.corrections["MC"][MC_correction_var])
            self.MC_label = f"MinBias simulation (prompt), $\eta$-reshaped"
            self.MC_label2 = f"prompt\ \eta-reshaped"

        # #The event cuts on the ID
        # dataCutOnID = (self.data["Muon_softMva1"] > self.ID_cut) & (self.data["Muon_softMva2"]> self.ID_cut)  
        # MCCutOnID = (self.MC["Muon_softMva1"] > self.ID_cut) & (self.MC["Muon_softMva2"] > self.ID_cut)     

        # print("Efficiency data ID cut: ", np.sum(self.normalized_sweights_data_signal[dataCutOnID]))
        # print("Efficiency prompt data ID cut (only applies to Jpsi): ", np.sum(self.combined_weights_data[dataCutOnID]))
        # print("Efficiency MC ID cut: ", np.sum(self.normalized_sweights_MC[MCCutOnID]))
        # print("Efficiency corrected MC ID cut (prompt and possible further correction): ", np.sum(self.combined_weights_MC[MCCutOnID]), "\n\n")

        self.histB,_ = np.histogram(self.scoreData,nbins,(0,1),False,self.normalized_sweights_data_bkg)
        self.histS,self.xe = np.histogram(self.scoreData,nbins,(0,1),False,self.normalized_sweights_data_signal)
        self.histS_prompt_corr,_ = np.histogram(self.scoreData,nbins,(0,1),False,self.combined_weights_data)
        self.histMC,_ = np.histogram(self.scoreMC,nbins,(0,1),False,self.normalized_sweights_MC)
        self.histMC_corr,_ = np.histogram(self.scoreMC,nbins,(0,1),False,self.combined_weights_MC)

        self.dx = self.xe[1]-self.xe[0]
        self.x = 0.5*(self.xe[:-1]+self.xe[1:])
        self.bin_cut=np.digitize(self.BDT_limit, self.xe)


        bkgYield_after_cut = np.sum(self.histB*(self.x>self.BDT_limit))
        sigYield_after_cut = np.sum(self.histS*(self.x>self.BDT_limit) )
        sig_prompt_Yield_after_cut = np.sum(self.histS_prompt_corr*(self.x>self.BDT_limit) )
        MCYield_after_cut = np.sum(self.histMC*(self.x>self.BDT_limit) )
        MC_corr_Yield_after_cut = np.sum(self.histMC_corr*(self.x>self.BDT_limit) )

        print(f"bkg Yield_after_cut: {bkgYield_after_cut:.2f}")
        print(f"signal Yield_after_cut: {sigYield_after_cut:.2f}")
        print(f"prompt signal Yield_after_cut: {sig_prompt_Yield_after_cut:.2f}")
        print(f"MCYield_after_cut: {MCYield_after_cut:.2f} \n\n")
        print(f"MC corr Yield_after_cut {MC_corr_Yield_after_cut:.2f} \n\n")

        self.effbkg =  bkgYield_after_cut
        self.effsig =  sigYield_after_cut
        self.effsig_corr =  sig_prompt_Yield_after_cut
        self.effMC = MCYield_after_cut
        self.effMC_corr = MC_corr_Yield_after_cut

        self.splot_unc_data = self.data["sigYield_sw"]
        self.bin_unc_data = np.sqrt(self.effsig_corr*(1-self.effsig_corr)/ self.prompt_signal_norm_data)
        self.bin_unc_MC = np.sqrt(self.effMC_corr*(1-self.effMC_corr)/ self.prompt_signal_norm_MC)
        self.tot_unc = self.effsig_corr/self.effMC_corr*np.sqrt((self.bin_unc_data/self.effsig_corr)**2 + (self.bin_unc_MC/self.effMC_corr)**2)

        print(f"BDT efficiency on Y peak: \n On unfolded signal: {self.effsig} \n On prompt corrected, unfolded signal: {self.effsig_corr} \n On MC: {self.effMC} \n On corrected MC: {self.effMC_corr}")
        print(f"Efficiency difference: {self.effMC_corr - self.effsig_corr}")
        print(f"Binomial uncertainty data : {self.bin_unc_data}")
        print(f"Binomial uncertainty MC : {self.bin_unc_MC}")
        print(f"total uncertainty : {self.tot_unc}")

    def plot(self,plot_bkg=False,plot_corr=True,plot_nonPrompt=True,text=None):
        hep.style.use("CMS")
        fig, ax = plt.subplots(figsize=(10,8))
        hep.cms.label("Preliminary",data=True,lumi=config["lumi"]["offline"], com=config["com"])
        colors = plt.cm.tab10.colors
        bkg_color = colors[0]
        sig_color = colors[1]
        MC_color = colors[2]
        if plot_bkg: ax.hist(self.scoreData, self.nbins, (0,1),  False, weights =self.normalized_sweights_data_bkg*self.nbins, label="Unfolded background", color = bkg_color, zorder=0, histtype='step', linewidth = 1.8)
        if plot_nonPrompt:
            ax.hist(self.scoreData, self.nbins, (0,1),  False, weights = self.normalized_sweights_data_signal*self.nbins, label="Unfolded signal", color = colors[3], zorder=0, histtype='step', linewidth = 1.8)
            ax.hist(self.scoreData, self.nbins, (0,1),  False, weights = self.normalized_sweights_data_signal*(self.scoreData>self.xe[self.bin_cut])*self.nbins, color = colors[3], zorder=0, histtype='bar', linewidth = 1.8, alpha = .1)
            ax.hist(self.scoreMC, self.nbins, (0,1),  False, weights =  self.normalized_sweights_MC*self.nbins, label="MinBias simulated", color = "red", zorder=0, histtype='step', linewidth = 1.8)
            ax.hist(self.scoreMC, self.nbins, (0,1),  False, weights =  self.normalized_sweights_MC*(self.scoreMC>self.xe[self.bin_cut])*self.nbins,  color = "red", label ='Selection', zorder=0, histtype='bar', linewidth = 1.8 , alpha=.1)
        if plot_corr:
            ax.hist(self.scoreData, self.nbins, (0,1),  False, weights = self.combined_weights_data*self.nbins, label="Unfolded signal (prompt)", color = sig_color, zorder=0, histtype='step', linewidth = 1.8)
            ax.hist(self.scoreData, self.nbins, (0,1),  False, weights = self.combined_weights_data*(self.scoreData>self.xe[self.bin_cut])*self.nbins, color = sig_color, zorder=0, histtype='bar', linewidth = 1.8, alpha = .1)
            ax.hist(self.scoreMC, self.nbins, (0,1),  False, weights = self.combined_weights_MC*self.nbins, label=self.MC_label, color = MC_color, zorder=0, histtype='step', linewidth = 1.8)
            ax.hist(self.scoreMC, self.nbins, (0,1),  False, weights = self.combined_weights_MC*(self.scoreMC>self.xe[self.bin_cut])*self.nbins,  color = MC_color, zorder=0, histtype='bar', linewidth = 1.8 , alpha=.1)
        if text is None:
            text=f"""
            sPlot unfolded BDT distribution 
            Model: {self.BDT_model}
            Data: {self.particle_name} trigger
            Cut: {self.BDT_limit}"""+plot_nonPrompt*f"""
            $\epsilon_{{data}} = ${round(self.effsig,3)} 
            $\epsilon_{{MC}} = ${round(self.effMC,3)}"""+plot_corr*f"""
            $\epsilon_{{data}}^{{prompt}} = ${round(self.effsig_corr,3)} $\pm$ {round(self.bin_unc_data,3)} 
            $\epsilon_{{MC}}^{{{self.MC_label2}}} = ${round(self.effMC_corr,3)} $\pm$ {round(self.bin_unc_MC,3)} 
            $\zeta_{{MVA}} = ${round(self.effsig_corr/self.effMC_corr,3)}  $\pm$ {round(self.tot_unc,3)}  """
            # $\Delta\epsilon = ${round(np.abs(self.effMC_corr - self.effsig_corr),3)}
        ax.text(0.25,0.3, text, fontsize=14,  transform=ax.transAxes)
            # ax.grid()

        handles, labels = plt.gca().get_legend_handles_labels()
        gray_box = mpatches.Patch(color='green', alpha=0.1, label='Selection')
        handles.append(gray_box)
        labels.append('Selection')
        print(labels)
        ax.legend(handles=handles, labels=labels)    
        ax.set_xlabel("BDT score")
        ax.set_ylabel("Normalized frequency")
        return

    def plot_quantities(self,quantities,nbins=200,density=False,log=False,plot_selection=False,ylim=None,xlabel=None):
        c = plt.cm.tab10.colors
        for d in quantities.keys():
            hep.style.use("CMS")
            fig, ax = plt.subplots(figsize=(10,8))
            hep.cms.label("Preliminary",data=True,lumi=config["lumi"]["offline"],com=config["com"])
            print(d," done")
            ax.hist(self.data[d], bins =nbins,weights=self.combined_weights_data, range = quantities[d]['l'], label=fr"Unfolded data signal $J/\psi$", color="blue", density = density, log=log, histtype='step', linewidth=2)
            ax.hist(self.data[d], bins =nbins,weights=self.normalized_sweights_data_bkg, range = quantities[d]['l'], label=fr"Unfolded data background $J/\psi$", color='orange', density = density, log=log, histtype='step', linewidth=2)
            # ax.hist(self.MC[d], bins = nbins, weights = self.combined_weights_MC, range = quantities[d]['l'], label=f"MinBias simulation ({self.particle_name})", color=c[2], density = density, log=log, histtype='step', linewidth=2)
            if plot_selection:
                ax.hist(self.data[d], bins = nbins, weights=self.combined_weights_data*(self.scoreData>self.BDT_limit), range = quantities[d]['l'], label=f"Unfolded data signal selection ({self.particle_name})", color=c[3], density = density, log=log, histtype='step', linewidth=2)
                ax.hist(self.MC[d], bins = nbins, weights=self.combined_weights_MC*(self.scoreMC>self.BDT_limit), range = quantities[d]['l'], label=f"MinBias simulation selection ({self.particle_name})", color=c[4], density = density, log=log, histtype='step', linewidth=2)
            # ax.hist(d, bins = nbins, range = xlim, color=c, density = density, log=log, alpha = 0.5)# hatch = '*',
            ax.set_xlabel(d)
            if xlabel is not None:
                ax.set_xlabel(xlabel)
            # if text!=None: ax.text(0.02, .8, text, fontsize=13, bbox=dict(facecolor='white'), transform=ax.transAxes) 
            if quantities[d]['t']=='i': ax.xaxis.get_major_locator().set_params(integer=True)
            l = density*'Normalized f'+ (density==False)*'F'+'requency'
            ax.set_ylabel(l)
            # ax.set_xlim(xlim)
            ax.set_ylim(ylim)
            ax.legend()
            ax.grid(True)
            plt.show()
        return

        
    def plot_ROC(self, plot_nonPrompt=True,plot_corr=True, n_points = 50, log = False):
        hep.style.use("CMS")
        colors = ["orange","red","blue","green"]
        fig, ax = plt.subplots(figsize=(9,9))
        hep.cms.label("Preliminary",data=True,lumi=config["lumi"]["offline"],com=config["com"])

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

    def study_significance(self, mva_or_ID: Literal["mva","ID"]):
        
        def significance_mva(dis_lim):
            s = np.sum(self.combined_weights_data[self.scoreData>dis_lim])
            b = np.sum(self.normalized_sweights_data_bkg[self.scoreData>dis_lim]) 
            r =  0 if b== 0 else s/np.sqrt(b)
            return r,s,b
        
        def significance_id(dis_lim):
            s = np.sum(self.combined_weights_data[(self.data["Muon_softMva1"] > dis_lim) &  (self.data["Muon_softMva2"]>dis_lim)])
            b = np.sum(self.normalized_sweights_data_bkg[(self.data["Muon_softMva1"] > dis_lim) &  (self.data["Muon_softMva2"]>dis_lim)]) 
            r =  0 if b== 0 else s/np.sqrt(b)
            return r,s,b

        stot = np.sum(self.combined_weights_data)
        btot = np.sum(self.normalized_sweights_data_bkg)

        xlims = [0.6,0.98] if mva_or_ID=="mva" else [0.2,0.6]
        BDT_vals= np.linspace(*xlims,1000)
        significance_vals = np.vectorize(significance_id if mva_or_ID=="ID" else significance_mva)(BDT_vals)
        max_idx = np.argmax(significance_vals[0])
        max_cut = BDT_vals[max_idx]
        max_significance = significance_vals[0][max_idx]
        sig_eff = significance_vals[1][max_idx]/stot
        bkg_eff = significance_vals[2][max_idx]/btot
        text = rf"""$\varepsilon_{{sig}}$ = {round(sig_eff,3)}
$\varepsilon_{{bkg}}$ = {round(bkg_eff,3)}"""
        
        text_id_ar = fr"""$\varepsilon_{{sig}}^{{ar}}$ = {round(sig_eff,3)}
$\varepsilon_{{bkg}}^{{ar}}$ = {round(bkg_eff,3)}"""
        
        if mva_or_ID == 'ID' :
            alt_cut = 0.4
            alt_significance = significance_vals[0][np.argmin(np.abs(BDT_vals-alt_cut))]
            sig_eff_alt = significance_vals[1][np.argmin(np.abs(BDT_vals-alt_cut))]/stot
            bkg_eff_alt = significance_vals[2][np.argmin(np.abs(BDT_vals-alt_cut))]/btot
            # text_tuned = fr"Cut point (tuned) = {round(alt_cut,3)}\n $\varepsilon_{{sig}}^{{t}}$ = {round(sig_eff_alt,3)}\n$\varepsilon_{{bkg}}^{{t}}$ = {round(bkg_eff_alt,3)}"
            text_tuned=fr"""$\varepsilon_{{sig}}^{{*}}$ = {round(sig_eff_alt,3)}
$\varepsilon_{{bkg}}^{{*}}$ = {round(bkg_eff_alt,3)}"""
        hep.style.use("CMS")
        plt.figure(figsize=(10, 8))
        hep.cms.label("Preliminary",data=True,lumi=config["lumi"]["offline"], com=config["com"])
        plt.plot(BDT_vals, significance_vals[0],label="Significance")
        plt.scatter([max_cut],[max_significance],label="Arithmetic w.p.",c='red')
        ax = plt.gca()
        if mva_or_ID == "ID":
            plt.scatter([alt_cut],[alt_significance],label="Used w.p.",c='green')
            # plt.text(0.1,0.45,text_tuned, fontsize=14, bbox=dict(facecolor='white', edgecolor='black'),transform=ax.transAxes) 
            plt.text(0.37,alt_significance*0.97,text_tuned, fontsize=15) 
            plt.text(0.29,alt_significance*0.97,text_id_ar, fontsize=15) 
        else:   
            plt.text(0.77,max_significance*0.9,text, fontsize=15) 
        print("working point : ",  max_cut)
        plt.xlabel(r'Prompt $J/\psi$ BDT cut' if mva_or_ID=="mva" else 'Muon Soft MVA cut')
        plt.ylabel('Significance $s/\sqrt{b}$')
        plt.legend()
        plt.show()