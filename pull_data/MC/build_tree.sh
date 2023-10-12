source /cvmfs/cms.cern.ch/cmsset_default.sh

cd /work/submit/mori25/DimuonAnalysis/CMSSW_13_0_6/src
eval `scramv1 runtime -sh`
cd /work/submit/mori25/DimuonAnalysis/macros

index=$(($1))

start=$(( $index*50+1 ))
end=$(( $start+49 ))

for (( N=start; N<=end; N++ )); do
        file=$( cat files.txt | sed -n ''$N'p')
        echo running $file
        infile="/data/submit/wangzqe/InclusiveDileptonMinBias_TuneCP5Plus_13p6TeV_pythia8+Run3Summer22MiniAODv3-Pilot_124X_mcRun3_2022_realistic_v12-v5+MINIAODSIM/$file"
        root -l -b -q generateOffDimuonTree.C\(\"$infile\"\,\"/data/submit/mori25/DPh/MC/Jpsi/r3MCtree_$N.root\"\)
    done

# hadd -f /data/submit/mori25/DPh/MC/Jpsi/r3MCtree_merged_$1.root /data/submit/mori25/DPh/MC/Jpsi/r3MCtree_*.root