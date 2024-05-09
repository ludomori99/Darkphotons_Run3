#!/bin/bash

function scan_logs() {
    local log_dir="$1"

    # Loop over all .log files in the directory
    for log_file in "$log_dir"/*.log; do
        # Use awk to find lines containing "chisquare=" and check if the number after "chisquare=" is greater than 3
        # If such a line is found, echo the file name
        awk -F'chisquare=' '/chisquare=/{if ($2 > 5) print FILENAME, "chisquare=" $2}' "$log_file"
    done
}

# Call the function with the directory as an argument
scan_logs "/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits/PassingProbeSoftId/Probe_pt/"
# scan_logs "/work/submit/mori25/Darkphotons_ludo/offline_analysis/tagnprobe/CMS-tutorial/Fits/PassingProbeSoftId/Probe_eta/"