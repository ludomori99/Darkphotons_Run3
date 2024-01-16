# sequence of commands to obtain the data correctly extracted in the desired folder 

#This is the only file with hard coded stuff (besides config)
export HOMEUSER=/home/submit/mori25/
export DPUSER=/work/submit/mori25/Darkphotons_ludo/
export PYTHONPATH="$DPUSER/utils/"

test_or_depl='depl'


# The idea is to be able to execute all in a pipeline with a given configuration, to repeat experiments with different configurations. for now choose between test or depl.
# Still need to enforce the pipeline (need a routine that monitors and waits until all offline data are extracted. same for MC )
# Hardcoded: 
#     - name of ntuples (mergedA.root). When generalize might adopt setting A, B etc. 


############### DATA EXTRACTION ############
# ###generate the scripts to extract data and MC 
# python3 $DPUSER/utils/generate_scripts_extraction.py $test_or_depl
# echo Generated scripts

###If needed., create a list with the offline files to extract
# bash $DPUSER/pull_data/offline/make_list.sh
# echo Made list of files to extract 

###submit the condor batch job to extract the offline data
# condor_submit $DPUSER/pull_data/offline/condor_off.sub

# TODO : implement check that extraction was successful


## Extract MC 
# python3 $DPUSER/pull_data/MC/slurm_submit.py
# TODO : implement check that extraction was successful
# bash $DPUSER/pull_data/MC/merge_final.sh


## Extract new MC

# condor_submit $DPUSER/pull_data/MCRun3/condor_MCRun3_Jpsi.sub
# condor_submit $DPUSER/pull_data/MCRun3/condor_MCRun3_Y.sub

# condor_submit $DPUSER/pull_data/MCRun3/condor_MCRun3.sub

# bash $DPUSER/pull_data/MCRun3/merge_final.sh


########### END OF EXTRACTION ##################### 


############# SKIM DATA FOR BDT #######################
# python3 $DPUSER/utils/skim_mass.py $test_or_depl


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
root -l -b -q ${DPUSER}utils/compute_total_efficiencies.C

# ##### create mass histogramm
# root -l -b -q ${DPUSER}utils/make_hist.C


# # ##### make cards out of histogram (edit the systematics manually)
# root -l -b -q ${DPUSER}utils/makeCardsAndWS.C


# #### run combine (need to install it. see https://cms-analysis.github.io/HiggsAnalysis-CombinedLimit/)
# python3 ${DPUSER}utils/limitprocessing.py
# mv higgsCombinea* /data/submit/mori25/dark_photons_ludo/DimuonTrees/limits/30_11/

# ### plot limits
# python3 ${DPUSER}utils/plot_limits.py
