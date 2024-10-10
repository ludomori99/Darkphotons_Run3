import os
import subprocess

# SLURM script template
slurm_script_template = """#!/bin/bash
#SBATCH --job-name={njob}
#SBATCH --output={log_dir}{njob}.out
#SBATCH --error={log_dir}{njob}.err
#SBATCH --time=02:00:00
#SBATCH --mem=2GB
#SBATCH --partition=submit
#SBATCH --exclude=submit[05,06,80,81]

cd {log_dir}
{cmd1}
echo done
"""

njobs=1
log_dir="/work/submit/mori25/Darkphotons_ludo/offline_analysis/limits"

for i in range(njobs):
    cmd1 = rf"root -l -b -q /work/submit/mori25/Darkphotons_ludo/offline_analysis/limits/make_hist.C"

    slurm_script_content = slurm_script_template.format(
        log_dir=log_dir, cmd1=cmd1, njob=i
    )

    # Write the SLURM script to a file
    slurm_script_file = f"{log_dir}submit_{i}.sh"
    with open(slurm_script_file, "w") as f:
        f.write(slurm_script_content)

    # Submit the SLURM job
    subprocess.run(["sbatch", slurm_script_file])