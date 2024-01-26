#!/bin/bash

# Set the folder to monitor
log_folder="/data/submit/mori25/dark_photons_ludo/DimuonTrees/$1"

# Set the keywords to search for in log files
keywords=("error" "warning" "critical" "not found")

# Function to check if a log file contains any of the specified keywords
check_log_file() {
    file="$1"
    for keyword in "${keywords[@]}"; do
        if grep -q "$keyword" "$file"; then
            echo "Suspect log file: $file (contains '$keyword')"
            break
        fi
    done
}

# Main script
if [ -d "$log_folder" ]; then
    echo "Monitoring logs in $log_folder..."

    # Loop through each log file in the folder
    for log_file in "$log_folder"/*.; do
        if [ -f "$log_file" ]; then
            check_log_file "$log_file"
        fi
    done

    echo "Monitoring complete."
else
    echo "Error: Log folder '$log_folder' not found."
fi
