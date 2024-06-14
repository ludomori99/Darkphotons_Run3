# sequence of commands to obtain the data correctly extracted in the desired folder 

#This is the only file with hard coded stuff (besides config)
export HOMEUSER=/home/submit/mori25/
export DPUSER=/work/submit/mori25/Darkphotons_ludo/offline_analysis/
export DPUSERBASE=/work/submit/mori25/Darkphotons_ludo/
export PYTHONPATH="$DPUSER/utils/"

test_or_depl='depl'


# The idea is to be able to execute all in a pipeline with a given configuration, to repeat experiments with different configurations. for now choose between test or depl.
# Still need to enforce the pipeline (need a routine that monitors and waits until all offline data are extracted. same for MC )
# Hardcoded: 
#     - name of ntuples (mergedA.root). When generalize might adopt setting A, B etc. 


############### DATA EXTRACTION ############
# # ###generate the scripts to extract data and MC 
# python3 $DPUSER/utils/generate_scripts_extraction.py

# # ###If needed., create a list with the offline files to extract
# bash $DPUSER/pull_data/offline/make_list.sh
# echo Made list of files to extract \(offline\)

# ## Extract Dilepton Inclusive MC
# bash $DPUSER/pull_data/MC_InclusiveMinBias/make_list.sh
# echo Made list of files to extract \(MC_InclusiveMinBias\)

# ## Extract low mass DY MC
# bash $DPUSER/pull_data/MC_lmDY/make_list.sh
# echo Made list of files to extract \(MC_lmDY\)


### Condor section 
# condor_submit $DPUSER/pull_data/offline/offline.sub
# condor_submit $DPUSER/pull_data/offline/offline_full.sub

# bash $DPUSER/pull_data/offline_normal/run_merge_off_normal.sh

# ## submit the low mass DY MC data
# python3 $DPUSER/pull_data/MC_lmDY/slurm_submit_lmDY.py


### Extract with slurm the Inclusive Min Bias dilepton inclusive 
# bash $DPUSER/pull_data/MC_InclusiveMinBias/run_try.sh # for testing the script

# condor_submit $DPUSER/pull_data/MC_InclusiveMinBias/MC_InclusiveMinBias_Jpsi.sub
# condor_submit $DPUSER/pull_data/MC_InclusiveMinBias/MC_InclusiveMinBias_Y.sub
# condor_submit $DPUSER/pull_data/MC_InclusiveMinBias/MC_InclusiveMinBias.sub 


# TODO : implement check that extraction is complete (probably monitor .out files)

# bash $DPUSER/utils/scan_logs.sh "offline"
# bash $DPUSER/utils/scan_logs.sh "MC_InclusiveMinBias/"
# bash $DPUSER/utils/scan_logs.sh "MC_lmDY/"

# need to wait until extraction is completed
# bash $DPUSER/pull_data/MC_InclusiveMinBias/merge_final.sh

# hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/inclusive/merged_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/dump/*.root

########### END OF EXTRACTION ##################### 


######## SKIM DATA FOR BDT #######################
# python3 $DPUSER/utils/skim_mass.py $test_or_depl


############## EXECUTE TRAINING ON SKIMMED DATA #################
# python3 $DPUSER/BDT/training_offline.py


############# LOOK AT TRAINING RESULTS ###########################
# jupyter nbconvert --to notebook --execute $DPUSER/BDT/view_training.ipynb --output view_training_compiled.ipynb


########### STORE BDT SCORE IN DATA FILE (merged_A scheme) ########################
# python3 $DPUSER/BDT/evaluate_BDT.py


############# PERFORM SPLOT ###############
# splot on MC is not used anymore in practice but we keep the structure. will be changed

# root -l -b -q ${DPUSER}utils/sPlot.C\(\"Y\"\,1\)
# root -l -b -q ${DPUSER}utils/sPlot.C\(\"Jpsi\"\,1\)
# root -l -b -q ${DPUSER}utils/sPlot.C\(\"Jpsi\"\,0\)
# root -l -b -q ${DPUSER}utils/sPlot.C\(\"Y\"\,0\)

######## LOOK AT SIGNAL MODEL #############

# root -l -b -q ${DPUSER}fits/signal_fit.C\(\"Jpsi\"\,0\,500000\)
# root -l -b -q ${DPUSER}fits/plot_fit.C\(\)
# root -l -b -q ${DPUSER}fits/multi_fit.C\(\)


########## Tag and probe #################

# root -l -b -q  ${DPUSER}tagnprobe/CMS-tutorial/Efficiency.C 
# root -l -b -q  ${DPUSER}tagnprobe/CMS-tutorial/Efficiency2D.C
# source ${DPUSER}tagnprobe/CMS-tutorial/src/scan_fits.sh

# root -l -b -q  ${DPUSER}tagnprobe/CMS-tutorial/Compute_Factor.C\(\"Probe_pt\"\)
# root -l -b -q  ${DPUSER}tagnprobe/CMS-tutorial/Compute_Factor.C\(\"Probe_abs_eta\"\)
# root -l -b -q  ${DPUSER}tagnprobe/CMS-tutorial/Compute_Factor.C\(\"Probe_eta\"\)
# root -l -b -q  ${DPUSER}tagnprobe/CMS-tutorial/Compute_Factor.C\(\"Mm_dR\"\)

#####################################
#####  Now look to select the actual data ########
####################################

####Evaluate the BDT on the offline data dump
# python3 $DPUSER/BDT/evaluate_dump/slurm_evaluate_dump.py
# bash $DPUSER/utils/scan_logs.sh "offline/logs/eval_BDT"


# Obsolete
#Compute global efficiencies (for now, could also do in different phase space slices)
# root -l -b -q ${DPUSER}utils/compute_total_efficiencies.C



####### LIMITS ############
# ##### create mass histogramm
# root -l -b -q ${DPUSER}limits/make_hist.C


# next block need to execute within cmssw-el7 
#go into default cmssenv (CMSSW_13); activate the image with cmssw-el7 ; go into CMSSW_12 ;  run makecards, limitprocessing;  source /work/submit/mori25/Darkphotons_ludo/pipelines/lowmass_offline_pipeline.sh
# cd ${DPUSERBASE}CMSSW_12_6_5/src; 
# cmsenv; 
# cd ${DPUSERBASE};
# # ##### make cards out of histogram (edit the systematics manually)
# root -l -b -q ${DPUSER}limits/makeCardsAndWS.C


# # #### run combine (need to install it. see https://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/)
# python3 ${DPUSER}limits/limitprocessing.py
# mv higgsCombinea* /data/submit/mori25/dark_photons_ludo/DimuonTrees/limits/full_no_nuisances/output_expected
# mv roostats* /data/submit/mori25/dark_photons_ludo/DimuonTrees/limits/full_no_nuisances/stats/

# # # ### plot limits
# python3 ${DPUSER}limits/plot_limits.py
python3 ${DPUSER}limits/plot_limit_plt.py
# 