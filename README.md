# General information

Framework to perform CMS Dark Photon analysis in Run3.

Pipeline for data extraction, selection, computation of systematics and computation of limits is to be found in `pipelines/extraction_pipeline`. Ideally one can customize the pipeline by choosing a given configuration file. For now it is enough to use one single configuration file; if one wants to be able to use multiple, it is enough to change the path of the yaml file in the `load_analysis_config()` functions at the head of every file to be defined as an environment variable in the pipeline script. Some things are hardcoded (for now) for practicality as long as the framework is under construction.

To run every piece of the analysis it is convenient to simply execute the command `source extraction_pipeline.sh`, after having uncommented the part one is interested in. It is good to do a safety check that the script are doing what is expected by controlling the `main`. 
 
Main steps are:
* extraction of data. Task defined in `pull_data/offline/run_merge_off.sh` and submitted to distributed computing system using Condor with the command `condor_submit $DPLUDO/pull_data/offline/condor_off.sub`. Script templates are constructed based on employed configuration by executing `python3 $DPLUDO/utils/generate_scripts_extraction.py $test_or_depl`. `$test_or_depl` is (for now) the pipeline variable that encodes the choice of configuration. Configurable # of jobs (files) delivers output in `<DATA>/DimuonTrees/offline/dump/`, and is then prepared for training by application of mass cuts using `python3 $DPLUDO/utils/skim_mass.py $test_or_depl`
* extraction of MC, ibid. In this case use Slurm (as long as MC is stored on SubMIT), entirely analog as data. Need a final script bash `$DPLUDO/pull_data/MC/merge_final.sh` to merge the distributed outputs. 
* BDT training to select DY-like dimuon events from known meson resonances. Based off XGBoost, Model name includes method (forest/tree/...) and meson used for training. `BDT/evaluate_BDT.py` will add the MVA score to the ntuple file. Default model is (so far) best model i.e. `forest_standard`. Can be passed in as argument in `main`. 
* Viewing/evaluating the BDT is done using notebooks. Built to be used within or without pipeline, only to produce plots (least possible processing). Evaluation takes place on data and MC to estimate systematic uncertainty.  
Notes on the extraction: condor files and bash scripts are generated in the file utils/generate_scripts_extraction.py. Slurm files (in python) are directly to be edited in the pull_data/... folder, since can integrate with configuration file easily. Only basic assumptions on the structure of the file system are made (e.g. one should not change the name of the directories in the cloned repository.) 

# Training of BDT
This section mainly refers to the script `BDT/training_offline.py`, where the `Trainer` object is defined, as well as the `BDT/view_training.ipynb` notebook, used to visualize the traning results, and the `BDT/evaluate_BDT.py` script, used to extend the ntuples by the BDT score of a given model. The training is performed with the python package XGBoost. Different models are trained, used, and compared, based on different general techniques, hyperparameters, input variables and data used for training. The general syntax for defining a trained network is `<model name>_<particle used for training>`, e.g. `forest_standard_Jpsi` or `forest_ID_Y`. The particle defines the data we are using for training, whereby we take signal to be a mass band close to the meson resonance peak(s), and background as sidebands just outside the resonant region. The bands are defined in the configuration file `config/analysis_config.yml`, as well as each model's (hyper)parameters. The structure of the configuration file should ideally be self-explanatory, or for more information one can look at https://xgboost.readthedocs.io/en/stable/.

The most straightforward way to train a model (say `forest_ID`) on data from a particle (say `Y`) is by including the following lines in the `main` of `training_offline.py`:
```
Y_trainer = Trainer("Y", 'forest_ID')
Y_trainer.complete_train()
Y_trainer.plot_model() #if desired, can provide kwarg saveas=config["locations"]["public_html"]+"BDTs/Y_forest_standard.png"
```
For the Jpsi prompt analysis we need to apply reweighing to the Jpsi data. For this purpose we define the `train_prompt_Jpsi()` function separately. We also added some small scripts to add a `prompt_weight` branch to each ntuple we are using, for the case we are not training on that data. This is just to keep the ntuples consistent when we are to work with them to extract the systematics. 


**Hardcoded parts**
Everything else (except for minor and not relevant stuff) should be only modified from the configuration file. 
- Set of variables stored in the ntuples. Can be changed in `pull_data/MCRun3/generateMCDimuonTree.C` and `pull_data/offline/generateOffDimuonTree.C`. They must necessarily be a superset of the variables used for training and selection as defined in the `config`. Current choice from extraction process is hardcoded to result in skimmed merged datasets defined as `merged_A.root:tree`. Will generalize when needed. 
- Paths of files in sPlot.C. For time being need to change manually. In future might integrate the config to be read by the C++ script as well.
- step size (now 10000) of event chunks when evaluating bdt on offline in evaluate_BDT.py. Probably no need to put in config
