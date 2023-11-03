
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /work/submit/mori25/Darkphotons_ludo/CMSSW_13_0_6/src
eval `scramv1 runtime -sh`
cd /work/submit/mori25/Darkphotons_ludo/pull_data/MC/
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC/Y/merged.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC/Y/MCtree*.root
hadd -f /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC/Jpsi/merged.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC/Jpsi/MCtree*.root

# rm /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC/Y/MC*
# rm /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC/Jpsi/MC*
