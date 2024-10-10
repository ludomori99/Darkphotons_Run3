#!/bin/bash
#SBATCH --job-name=0
#SBATCH --output=/work/submit/mori25/Darkphotons_ludo/offline_analysis/limits0.out
#SBATCH --error=/work/submit/mori25/Darkphotons_ludo/offline_analysis/limits0.err
#SBATCH --time=02:00:00
#SBATCH --mem=2GB
#SBATCH --partition=submit
#SBATCH --exclude=submit[05,06,80,81]

cd /work/submit/mori25/Darkphotons_ludo/offline_analysis/limits
root -l -b -q /work/submit/mori25/Darkphotons_ludo/offline_analysis/limits/make_hist.C
echo done
