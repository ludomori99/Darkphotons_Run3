#!/bin/bash

log_dir="/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/logs"

# Set the indices of the log files to remove
indices=(259 1404 260 261 865 349 963 1406 697 241 621 1079 2 364 620 126 1137 1323 707 665 611 616 327 1324 955 702 358 969 129 130 165 178 212 215 218)

# Loop through each index
for index in "${indices[@]}"; do
    # Construct the log file name
    log_files="$log_dir/job.$index.*"
    for file in $log_files; do
        # Check if the file exists
        if [ -f "$file" ]; then
            # Remove the file
            rm "$file"
            echo "Removed $file"
        else
            echo "File $file does not exist"
        fi
    done
done