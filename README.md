# Darkphotons_ludo

Framework to perform CMS Dark Photon analysis in Run3.

Pipeline for extraction, selection, computation of systematics and computation of limits is to be found in `pipelines/extraction_pipeline`. Ideally can customize pipeline by choosing a given `config` file. Some things are hardcoded (for now) for practicality as long as the framework is under construction. A list of hardcoded parts are below.

Main steps are:
    - extraction of data. Task defined in `pull_data/offline/run_merge_off.sh` and submitted to distributed computing system using Condor with the command `condor_submit $DPLUDO/pull_data/offline/condor_off.sub`. Script templates are constructed based on employed configuration by executing `python3 $DPLUDO/utils/generate_scripts_extraction.py $test_or_depl`. `$test_or_depl` is (for now) the pipeline variable that encodes the choice of configuration. Configurable # of jobs (files) delivers output in `<DATA>/DimuonTrees/offline/dump/`, and is then prepared for training by application of mass cuts using `python3 $DPLUDO/utils/skim_mass.py $test_or_depl`
    - extraction of MC, ibid. In this case use Slurm (as long as MC is stored on SubMIT), entirely analog as data. Need a final script bash `$DPLUDO/pull_data/MC/merge_final.sh` to merge the distributed outputs. 
    - BDT training to select DY-like dimuon events from known meson resonances. Based off XGBoost, Model name includes method (forest/tree/...) and meson used for training. `BDT/evaluate_BDT.py` will add the MVA score to the ntuple file. Default model is (so far) best model i.e. `forest_standard`. Can be passed in as argument in `main`. 
    - Viewing/evaluating the BDT is done using notebooks. Built to be used within or without pipeline, only to produce plots (least possible processing). Evaluation takes place on data and MC to estimate systematic uncertainty.  


# Hardcoded parts
- Set of variables stored in the ntuples. Can be changed in `pull_data/MC/generateMCDimuonTree.C` and `pull_data/offline/generateOffDimuonTree.C`. They must necessarily be a superset of the variables used for training and selection as defined in the `config`. Current choice from extraction process is hardcoded to result in skimmed merged datasets defined as `merged_A.root:tree`. Will generalize when needed. 
- 