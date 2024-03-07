
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

start=$(( $1*66+1 ))
end=$(( $start+66 ))

for (( N=start; N<end; N++ )); do

    file=$(cat ./list_offline.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file input.root
    root -l -b -q generateOffDimuonTree.C\(\"input.root\"\,\"r3tree_$N.root\"\,20\)

done
hadd r3tree.root r3tree_*.root
xrdcp -f r3tree.root root://submit50.mit.edu//mori25/dark_photons_ludo/DimuonTrees/offline/dump//DimuonTree$1.root
