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

# bash $DPUSER/pull_data/offline_normal/run_merge_off_normal.sh

# ## submit the low mass DY MC data
# python3 $DPUSER/pull_data/MC_lmDY/slurm_submit_lmDY.py



### Extract with slurm the Inclusive Min Bias dilepton inclusive 
# bash $DPUSER/pull_data/MC_InclusiveMinBias/run_try.sh # for testing the script

# condor_submit $DPUSER/pull_data/MC_InclusiveMinBias/MC_InclusiveMinBias_Jpsi.sub
# condor_submit $DPUSER/pull_data/MC_InclusiveMinBias/MC_InclusiveMinBias_Y.sub
# condor_submit $DPUSER/pull_data/MC_InclusiveMinBias/MC_InclusiveMinBias.sub 


# TODO : implement check that extraction is complete (probably monitor .out files)

# bash $DPUSER/utils/scan_logs.sh "offline/logs"
# bash $DPUSER/utils/scan_logs.sh "MC_InclusiveMinBias/logs"
# bash $DPUSER/utils/scan_logs.sh "MC_lmDY/logs"


# need to wait until extraction is completed
# bash $DPUSER/pull_data/MC_InclusiveMinBias/merge_final.sh

# hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/inclusive/merged_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/dump/*.root

########### END OF EXTRACTION ##################### 


######### SKIM DATA FOR BDT #######################
python3 $DPUSER/utils/skim_mass.py $test_or_depl


############## EXECUTE TRAINING ON SKIMMED DATA #################
# python3 $DPUSER/BDT/training_offline.py


############# LOOK AT TRAINING RESULTS ###########################
# jupyter nbconvert --to notebook --execute $DPUSER/BDT/view_training.ipynb --output view_training_compiled.ipynb


############# STORE BDT SCORE IN DATA FILE ########################
# python3 $DPUSER/BDT/evaluate_BDT.py


############# PERFORM SPLOT ###############

# root -l -b -q ${DPUSER}utils/sPlot.C\(\"Y\"\,1\)
# root -l -b -q ${DPUSER}utils/sPlot.C\(\"Y\"\,0\)
# root -l -b -q ${DPUSER}utils/sPlot.C\(\"Jpsi\"\,1\)
# root -l -b -q ${DPUSER}utils/sPlot.C\(\"Jpsi\"\,0\)

######## LOOK AT SIGNAL MODEL #############


# root -l -b -q ${DPUSER}fits/signal_fit.C\(\"Jpsi\"\,0\)
# root -l -b -q ${DPUSER}fits/plot_fit.C\(\)
# root -l -b -q ${DPUSER}fits/sandbox.C\(\)


########## Tag and probe #################

# root -l -b -q  ${DPUSER}tagnprobe/CMS-tutorial/Efficiency.C


#####################################
#####  Now look to select the actual data ########
####################################

####Evaluate the BDT on the offline data dump
# python3 $DPUSER/BDT/evaluate_dump/slurm_evaluate_dump.py


####Evaluate the BDT on the MC data dump
# python3 $DPUSER/BDT/evaluate_dump/slurm_evaluate_dump_MC.py


# cd ${DPUSER}CMSSW_12_6_5/src
# cmsenv
# cd ${DPUSER}

#Compute global efficiencies (for now, could also do in different phase space slices)
# root -l -b -q ${DPUSER}utils/compute_total_efficiencies.C

# ##### create mass histogramm
# root -l -b -q ${DPUSER}utils/make_hist.C


# # ##### make cards out of histogram (edit the systematics manually)
# root -l -b -q ${DPUSER}utils/makeCardsAndWS.C


# #### run combine (need to install it. see https://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/)
# python3 ${DPUSER}utils/limitprocessing.py
# mv higgsCombinea* /data/submit/mori25/dark_photons_ludo/DimuonTrees/limits/30_11/

# ### plot limits
# python3 ${DPUSER}utils/plot_limits.py
