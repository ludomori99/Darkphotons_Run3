for (( N=1; N<148; N++ )); do
    file=$(cat ./pull_data/offline_normal/list_offline_normal.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file /data/submit/mori25/dark_photons_ludo/DimuonTrees/offline_normal/tmp/input.root
    root -l -b -q ./pull_data/offline_normal/generateOffDimuonTree.C\(\"/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline_normal/tmp/input.root\"\,\"/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline_normal/tmp/r3tree_$N.root\"\,100\)

done
hadd /data/submit/mori25/dark_photons_ludo/DimuonTrees/offline_normal/DimuonTree0.root /data/submit/mori25/dark_photons_ludo/DimuonTrees/offline_normal/tmp/r3tree_*.root
