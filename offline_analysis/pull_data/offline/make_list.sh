xrdfs root://eoscms.cern.ch ls /store/group/phys_bphys/bmm/bmm6/NanoAOD/529/ | grep ParkingDoubleMuonLowMass >& $DPUSER/pull_data/offline/dataset.txt
i=0
N=$(wc -l < "$DPUSER/pull_data/offline/dataset.txt")
rm $DPUSER/pull_data/offline/list_offline.txt
while IFS= read -r line; do
  xrdfs root://eoscms.cern.ch ls $line >> $DPUSER/pull_data/offline/list_offline.txt
  echo -n -e "Progress: $i/$N\r"
  ((i++))
done < "$DPUSER/pull_data/offline/dataset.txt"

