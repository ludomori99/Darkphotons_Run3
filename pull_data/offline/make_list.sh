xrdfs root://eoscms.cern.ch ls /store/group/phys_bphys/bmm/bmm6/NanoAOD/523/ | grep ParkingDoubleMuonLowMass >& dataset.txt
rm $DPLUDO/pull_data/offline/list.txt
while IFS= read -r line; do
  xrdfs root://eoscms.cern.ch ls $line >> $DPLUDO/pull_data/offline/list.txt
done < "$DPLUDO/pull_data/offline/dataset.txt"
