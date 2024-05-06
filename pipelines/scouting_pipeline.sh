
#This is the only file with hard coded stuff (besides config)
export HOMEUSER=/home/submit/mori25/
export DPUSER=/work/submit/mori25/Darkphotons_ludo/scouting_analysis/
export PYTHONPATH="$DPUSER/utils/"


# source $DPUSER/pull_data/scouting/make_list.sh

# bash extract data

# python3 train_BDT.py

# python3 compute_efficiency


# create histo...

# ##### create mass histogramm
root -l -b -q ${DPUSER}limits/make_hist.C


# # ##### make cards out of histogram (edit the systematics manually)
# root -l -b -q ${DPUSER}utils/makeCardsAndWS.C