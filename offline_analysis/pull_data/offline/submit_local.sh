#!/bin/bash

# Set the executable and input files
executable="bash /work/submit/mori25/Darkphotons_ludo/offline_analysis/pull_data/offline/run_merge_off_full.sh"

# Set the indices
indices=(1137 707)
# (259 1404 260 261 865 349 963 1406 697 241 621 1079 2 364 620 126 1137 1323 707 665 611 616 327 1324 955 702 358 969 129 130 165 178 212 215 218)

for index in "${indices[@]}"; do
    $executable $index
done