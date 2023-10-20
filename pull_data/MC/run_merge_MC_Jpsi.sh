
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /work/submit/mori25/Darkphotons_ludo/CMSSW_13_0_6/src
eval `scramv1 runtime -sh`
cd /work/submit/mori25/Darkphotons_ludo/pull_data/MC/

start=$(( $1*50+1 ))
end=$(( $start+50 ))

for (( N=start; N<end; N++ )); do
        file=$( cat files.txt | sed -n ''$N'p')
        echo running $file
        infile="/data/submit/wangzqe/InclusiveDileptonMinBias_TuneCP5Plus_13p6TeV_pythia8+Run3Summer22MiniAODv3-Pilot_124X_mcRun3_2022_realistic_v12-v5+MINIAODSIM/$file"
        root -l -b -q generateMCDimuonTree.C\(\"$infile\"\,\"/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC/Jpsi/MCtree_$N.root\"\,443\)
    done

