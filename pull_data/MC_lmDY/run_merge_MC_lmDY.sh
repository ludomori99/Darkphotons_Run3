
RELEASE=CMSSW_13_0_6
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd $RELEASE/src
eval `scramv1 runtime -sh` # cmsenv is an alias not on the workers
cd /work/submit/mori25/Darkphotons_ludo/pull_data/MC_lmDY/ 

start=$(( $1*40+1 ))
end=$(( $start+40 ))
echo start job $1

for (( N=start; N<end; N++ )); do
    file=$(cat ./list_lmDY.txt | sed -n ''$N'p')
    echo running $file
    root -l -b -q generateOffDimuonTree.C\(\"$file\"\,\"/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/tmp/DimuonTreeTMP$1_$N.root\"\,1\)
    hadd /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/dump/DimuonTree$1.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/tmp/DimuonTreeTMP$1_*.root
    rm /data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_lmDY/tmp/DimuonTreeTMP$1_*.root
done
