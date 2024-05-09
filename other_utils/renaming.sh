#!/bin/bash

# Directory containing the files
dir="/data/submit/mori25/dark_photons_ludo/DimuonTrees/MC_InclusiveMinBias/dump/"

# # Loop over all .root files in the directory
# for file in "$dir"/DimuonTree_*.root; do
#     # Extract the number from the filename
#     number="${file##*Jpsi}"
#     number="${number%%.root}"

#     # Construct the new filename
#     new_file="${file%%Jpsi*}Jpsi_${number}.root"

#     # Rename the file
#     mv "$file" "$new_file"
# done


# Loop over all .root files in the directory
for file in "$dir"/DimuonTree_*.root; do
    # Extract the number from the filename
    number="${file##*Tree_}"
    number="${number%%.root}"

    # Construct the new filename
    new_file="${file%%Tree_*}TreeInclusive_${number}.root"

    # Rename the file
    mv "$file" "$new_file"
done