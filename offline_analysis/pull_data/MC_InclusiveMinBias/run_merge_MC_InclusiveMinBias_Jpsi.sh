
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

start=$(($1*53+1))
end=$(($start+53))

for (( N=start; N<end; N++ )); do

    file=$(cat ./list_InclMinBias.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file input.root
    root -l -b -q generateMCDimuonTree.C\(\"input.root\"\,\"r3tree_$N.root\"\,443\,1\)

done
hadd r3tree.root r3tree_*.root
xrdcp -f r3tree.root root://submit50.mit.edu//mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/dump/DimuonTreeJpsi$1.root
