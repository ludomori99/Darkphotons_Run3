
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /work/submit/mori25/Darkphotons_ludo/CMSSW_13_0_6/src
eval `scramv1 runtime -sh`
cd /work/submit/mori25/Darkphotons_ludo/pull_data/MCRun3/
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/Jpsi/merged_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/dump/DimuonTreeJpsi*.root
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/Y/mergedY1_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/dump/DimuonTreeY1_*.root
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/Y/mergedY2_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/dump/DimuonTreeY2_*.root
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/Y/mergedY3_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/dump/DimuonTreeY3_*.root
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/Y/mergedY_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MCRun3/dump/DimuonTreeY*.root
