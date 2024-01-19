
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

    file=$(cat ./list.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file input.root
    root -l -b -q generateMCDimuonTree.C\(\"input.root\"\,\"r3treeY1_$N.root\"\,553\,1\)
    root -l -b -q generateMCDimuonTree.C\(\"input.root\"\,\"r3treeY2_$N.root\"\,100553\,1\)
    root -l -b -q generateMCDimuonTree.C\(\"input.root\"\,\"r3treeY3_$N.root\"\,200553\,1\)
done
hadd r3treeY1.root r3treeY1_*.root
hadd r3treeY2.root r3treeY2_*.root
hadd r3treeY3.root r3treeY3_*.root
xrdcp -f r3treeY1.root root://submit50.mit.edu//mori25/dark_photons_ludo/DimuonTrees/MCRun3/dump/DimuonTreeY1_$1.root
xrdcp -f r3treeY2.root root://submit50.mit.edu//mori25/dark_photons_ludo/DimuonTrees/MCRun3/dump/DimuonTreeY2_$1.root
xrdcp -f r3treeY3.root root://submit50.mit.edu//mori25/dark_photons_ludo/DimuonTrees/MCRun3/dump/DimuonTreeY3_$1.root
