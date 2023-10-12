"""
A submitter for processing many samples parallely, using either Slurm or multithread.
Pass to this script the same options you would pass make_plots.py or merge_plots.py,
specify whether you want to plot or merge (--code),
and specify if you want multithread or Slurm (--method).

e.g.
python submit.py -i ../filelist/list.txt --isMC 1 --era 2016 -t July2023_2016 -o July2023_2016 --doABCD 1 --code plot --method multithread

If you're not running on submit, you might need to modify the slurm script template
and some of the work and log paths.

Authors: Luca Lavezzo and Chad Freer.
Date: July 2023
"""
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

cd {work_dir}
{cmd}
echo done
"""

njobs=60

for i in range(njobs):
    work_dir = os.getcwd()
    log_dir = "/data/submit/mori25/DPh/logs/tuning/"
    cmd = "bash /work/submit/mori25/DimuonAnalysis/macros/build_tree.sh " +str(i)

    slurm_script_content = slurm_script_template.format(
        log_dir=log_dir, work_dir=work_dir, cmd=cmd, njob=i
    )

    # Write the SLURM script to a file
    slurm_script_file = f"{log_dir}submit_{i}.sh"
    with open(slurm_script_file, "w") as f:
        f.write(slurm_script_content)

    # Submit the SLURM job
    subprocess.run(["sbatch", slurm_script_file])