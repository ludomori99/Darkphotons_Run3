import os
import subprocess
from config_loader import load_analysis_config,load_test_config

DP_USER = os.environ["DPUSER"]
config = load_analysis_config()

# SLURM script template
slurm_script_template = """#!/bin/bash
#SBATCH --job-name={njob}
#SBATCH --output={log_dir}{njob}.out
#SBATCH --error={log_dir}{njob}.err
#SBATCH --time=02:00:00
#SBATCH --mem=2GB
#SBATCH --partition=submit

cd {work_dir}
{cmd1}
echo done
"""

njobs=97

for i in range(njobs):
    work_dir = DP_USER
    log_dir = config["locations"]["MC_lmDY"]["logs"]
    cmd1 = rf"bash {DP_USER}pull_data/MC_lmDY/run_merge_MC_lmDY.sh " +str(i)

    slurm_script_content = slurm_script_template.format(
        log_dir=log_dir, work_dir=work_dir, cmd1=cmd1, njob=i
    )

    # Write the SLURM script to a file
    slurm_script_file = f"{log_dir}submit_{i}.sh"
    with open(slurm_script_file, "w") as f:
        f.write(slurm_script_content)

    # Submit the SLURM job
    subprocess.run(["sbatch", slurm_script_file])