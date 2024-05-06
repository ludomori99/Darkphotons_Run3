
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /work/submit/mori25/Darkphotons_ludo/CMSSW_13_0_6/src
eval `scramv1 runtime -sh`
cd /work/submit/mori25/Darkphotons_ludo/offline_analysis/pull_data/MC_InclusiveMinBias/
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Jpsi/merged_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/dump/DimuonTreeJpsi*.root
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY1_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/dump/DimuonTreeY1_*.root
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY2_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/dump/DimuonTreeY2_*.root
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY3_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/dump/DimuonTreeY3_*.root
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY12_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY1_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY2_A.root
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY13_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY1_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY3_A.root
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY123_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY1_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY2_A.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/Y/mergedY3_A.root
