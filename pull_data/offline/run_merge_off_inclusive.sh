
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /work/submit/mori25/Darkphotons_ludo/CMSSW_13_0_6/src
eval `scramv1 runtime -sh`
cd /work/submit/mori25/Darkphotons_ludo/pull_data/offline/

start=$(( $1*40+1 ))
end=$(( $start+40 ))

for (( N=start; N<end; N++ )); do

    file=$(cat ./list_inclusive.txt | sed -n ''$N'p')
    echo running $file
    root -l -b -q generateOffDimuonTree.C\(\"$file\"\,\"r3tree_$N.root\"\,10\)

done
hadd r3tree.root r3tree_*.root
xrdcp -f r3tree.root root://submit50.mit.edu//mori25/dark_photons_ludo/DimuonTrees/offline_inclusive/dump//DimuonTree$1.root
