import os
import subprocess
import yaml

HOME_USER = os.environ["HOMEUSER"]
DP_USER = os.environ["DPUSER"]

def load_analysis_config():
    try:
        with open(os.path.join(os.environ["DPUSER"],"config/analysis_config.yml"), "r") as f:
            config = yaml.load(f, Loader=yaml.FullLoader)
        return config

    except Exception as e: 
        raise e
    
config = load_analysis_config()


# SLURM script template
slurm_script_template = """#!/bin/bash
#SBATCH --job-name={njob}
#SBATCH --output={log_dir}{njob}.out
#SBATCH --error={log_dir}{njob}.err
#SBATCH --time=02:00:00
#SBATCH --mem=2GB
#SBATCH --partition=submit
#SBATCH --exclude=submit[05,06,80,81]

{cmd1}
echo done
"""

dump_dir = os.path.join("/data/submit",config["locations"]["offline"]["dump"])
dump_files = os.listdir(dump_dir)
log_dir = os.path.join(config["locations"]["offline"]["logs"],"eval_BDT/")

for i,file in enumerate(dump_files):
    cmd1 = rf"python3 {DP_USER}BDT/evaluate_dump/evaluate_dump_BDT.py {i}"

    slurm_script_content = slurm_script_template.format(
        log_dir=log_dir, cmd1=cmd1, njob=i
    )

    # Write the SLURM script to a file
    slurm_script_file = f"{log_dir}/submit_{i}.sh"
    with open(slurm_script_file, "w") as f:
        f.write(slurm_script_content)

    # Submit the SLURM job
    subprocess.run(["sbatch", slurm_script_file])