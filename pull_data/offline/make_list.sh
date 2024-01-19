ls /data/submit/wangzqe/lowDY/1to15/nano/* >& $DPUSER/pull_data/offline/list_inclusive.txt

xrdfs root://eoscms.cern.ch ls /store/group/phys_bphys/bmm/bmm6/NanoAOD/529/ | grep ParkingDoubleMuonLowMass >& $DPUSER/pull_data/offline/dataset.txt
rm $DPUSER/pull_data/offline/list.txt
while IFS= read -r line; do
  xrdfs root://eoscms.cern.ch ls $line >> $DPUSER/pull_data/offline/list.txt
done < "$DPUSER/pull_data/offline/dataset.txt"

