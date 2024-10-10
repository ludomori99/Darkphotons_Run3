
RELEASE=CMSSW_13_0_6
source /cvmfs/cms.cern.ch/cmsset_default.sh
if [ -r $RELEASE/src ] ; then
 echo release CMSSW_13_0_6 already exists
else
scram p CMSSW $RELEASE
fi

cd $RELEASE/src
eval `scramv1 runtime -sh` # cmsenv is an alias not on the workers
cd ../..
xrdcp -f root://eoscms.cern.ch//store/group/phys_bphys/bmm/bmm6/NanoAOD/525/InclusiveDileptonMinBias_TuneCP5Plus_13p6TeV_pythia8+Run3Summer22MiniAODv3-Pilot_124X_mcRun3_2022_realistic_v12-v5+MINIAODSIM/00383024-0ca1-4cd8-ab64-f302452bf137.root r3tree.root
xrdcp -f r3tree.root root://submit50.mit.edu//mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/test/test.root
