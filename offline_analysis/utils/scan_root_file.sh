#!/bin/bash

# Define the directory path and the given date
directory=/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/dump/  # Replace this with your directory path
given_date="2023-01-29"  # Replace this with your given date in YYYY-MM-DD format

# Convert the given date to seconds since the epoch
given_date_seconds=$(date -d "$given_date" +%s)

# Loop through the files in the directory
for file in "$directory"/*; do
    # Check if the item is a file
    if [ -f "$file" ]; then
        # Get the last modification time of the file in seconds since the epoch
        last_modified_seconds=$(stat -c %Y "$file")
        # Compare the last modification time with the given date
        if [ "$given_date_seconds" -gt "$last_modified_seconds" ]; then
            # Print the filename and its last modification time
            echo "$(basename "$file"): Last modified on $(date -d @"$last_modified_seconds" "+%Y-%m-%d %H:%M:%S")"
        fi
    fi
done
