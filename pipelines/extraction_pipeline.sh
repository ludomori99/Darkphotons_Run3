# sequence of commands to obtain the data correctly extracted in the desired folder 

#This is the only file with hard coded stuff (besides config)
export HOMELUDO=/home/submit/mori25/
export DPLUDO=/work/submit/mori25/Darkphotons_ludo/
export PYTHONPATH="$DPLUDO/utils/"

test_or_depl='depl'


# The idea is to be able to execute all in a pipeline with a given configuration, to repeat experiments with different configurations. for now choose between test or depl.
# Still need to enforce the pipeline (need a routine that monitors and waits until all offline data are extracted. same for MC )
# Hardcoded: 
#     - name of ntuples (mergedA.root). When generalize might adopt setting A, B etc. 


############### DATA EXTRACTION ############
# ###generate the scripts to extract data and MC 
python3 $DPLUDO/utils/generate_scripts_extraction.py $test_or_depl
echo Generated scripts

###If needed., create a list with the offline files to extract
# bash $DPLUDO/pull_data/offline/make_list.sh
# echo Made list of files to extract 

###submit the condor batch job to extract the offline data
# condor_submit $DPLUDO/pull_data/offline/condor_off.sub

# TODO : implement check that extraction was successful


## Extract MC 
# python3 $DPLUDO/pull_data/MC/slurm_submit.py
# TODO : implement check that extraction was successful
# bash $DPLUDO/pull_data/MC/merge_final.sh


## Extract new MC

# condor_submit $DPLUDO/pull_data/MCRun3/condor_MCRun3_Jpsi.sub
# condor_submit $DPLUDO/pull_data/MCRun3/condor_MCRun3_Y.sub

# bash $DPLUDO/pull_data/MCRun3/merge_final.sh


########### END OF EXTRACTION ##################### 


############# SKIM DATA FOR BDT #######################
# python3 $DPLUDO/utils/skim_mass.py $test_or_depl


############## EXECUTE TRAINING ON SKIMMED DATA #################
# python3 $DPLUDO/BDT/training_offline.py


############# LOOK AT TRAINING RESULTS ###########################
# jupyter nbconvert --to notebook --execute $DPLUDO/BDT/view_training.ipynb --output view_training_compiled.ipynb


############# STORE BDT SCORE IN DATA FILE ########################
# python3 $DPLUDO/BDT/evaluate_BDT.py


############# PERFORM SPLOT ###############

# root -l -b -q $DPLUDO/utils/sPlot.C\(\"Y\"\,1\)
# root -l -b -q $DPLUDO/utils/sPlot.C\(\"Y\"\,0\)
# root -l -b -q $DPLUDO/utils/sPlot.C\(\"Jpsi\"\,1\)
# root -l -b -q $DPLUDO/utils/sPlot.C\(\"Jpsi\"\,0\)