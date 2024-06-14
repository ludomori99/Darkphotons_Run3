#!/bin/bash

# Set the folder to monitor
log_folder="/data/submit/mori25/dark_photons_ludo/DimuonTrees/$1/logs"
dest_folder="/data/submit/mori25/dark_photons_ludo/DimuonTrees/$1/dump_full"
error_indices_file="/data/submit/mori25/dark_photons_ludo/DimuonTrees/$1/logs/error_indices.txt"
# Set the keywords to search for in log files
keywords=("error" "warning" "critical" "not found")

# Function to check if a log file contains any of the specified keywords
check_log_file() {
    file="$1"
    for keyword in "${keywords[@]}"; do
        if grep -q "$keyword" "$file"; then
            echo "Suspect log file: $file (contains '$keyword')"
            echo $(basename "$file" | cut -d'.' -f2) >> "$error_indices_file"
            break
        fi
    done
}

check_missing_files() {
    for ((N=0; N<1442; N++)); do
        if [ ! -f "$dest_folder/DimuonTree$N.root" ]; then
            echo "Missing file: $N"
            echo $N >> "$error_indices_file"
        fi
    done
}


# Main script
if [ -d "$log_folder" ]; then
    echo "Monitoring logs in $log_folder..."

    # Loop through each log file in the folder
    for log_file in $(find "$log_folder" -type f ! -name "*.sh"); do
        if [ -f "$log_file" ]; then
            check_log_file "$log_file"
        fi
    done

    check_missing_files

    echo "Monitoring complete."
else
    echo "Error: Log folder '$log_folder' not found."
fi
