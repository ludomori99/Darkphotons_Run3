import os, sys
from config_loader import load_analysis_config,load_test_config

# Script to write the bash scripts with proper current configuration 

HOME_USER = os.environ["HOMELUDO"]
DP_USER = os.environ["DPLUDO"]
OFFLINE_FOLDER = DP_USER+"/pull_data/offline/" 

config = load_analysis_config()

#Extract config parameters for submission
grouping_amount_data = config["condor"]["grouping_amount_data"]
off_njobs = config["condor"]["njobs"]
off_dump_dir = config["locations"]["offline"]["dump"]
off_logs_dir = config["locations"]["offline"]["logs"]


### BASH SCRIPT for offline data to be submitted in condor 
template_offline_bash = rf"""
RELEASE=CMSSW_13_0_6

source /cvmfs/cms.cern.ch/cmsset_default.sh

if [ -r $RELEASE/src ] ; then
 echo release CMSSW_13_0_6 already exists
else
scram p CMSSW $RELEASE
fi

cd $RELEASE/src
eval `scramv1 runtime -sh` # cmsenv is an alias not on the workers
cd ../..

start=$(( $1*{grouping_amount_data}+1 ))
end=$(( $start+{grouping_amount_data} ))

for (( N=start; N<=end; N++ )); do

    file=$(cat ./list.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file input.root
    root -l -b -q generateOffDimuonTree.C\(\"input.root\"\,\"r3tree_$N.root\"\)

done
hadd r3tree.root r3tree_*.root
xrdcp -f r3tree.root root://submit50.mit.edu//{off_dump_dir}/DimuonTree$1.root
"""
with open(DP_USER+"/pull_data/offline/run_merge_off.sh", "w") as file:
    file.write(template_offline_bash)

### CONDOR SCRIPT
template_offline_condor = rf"""executable = {OFFLINE_FOLDER}run_merge_off.sh
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = {OFFLINE_FOLDER}list.txt,{OFFLINE_FOLDER}generateOffDimuonTree.C
transfer_output_files = ""
MyIndex=$(ProcId)
Extention=$INT(MyIndex,%d)
arguments = "$(Extention)"
Requirements = ( BOSCOCluster =!= "t3serv008.mit.edu" && BOSCOCluster =!= "ce03.cmsaf.mit.edu" && BOSCOCluster =!= "eofe8.mit.edu")
+REQUIRED_OS = "rhel7"
+DESIRED_Sites = "mit_tier3,mit_tier2,T2_AT_Vienna,T2_BE_IIHE,T2_BE_UCL,T2_BR_SPRACE,T2_BR_UERJ,T2_CH_CERN,T2_CH_CERN_AI,T2_CH_CERN_HLT,T2_CH_CERN_Wigner,T2_CH_CSCS,T2_CH_CSCS_HPC,T2_CN_Beijing,T2_DE_DESY,T2_DE_RWTH,T2_EE_Estonia,T2_ES_CIEMAT,T2_ES_IFCA,T2_FI_HIP,T2_FR_CCIN2P3,T2_FR_GRIF_IRFU,T2_FR_GRIF_LLR,T2_FR_IPHC,T2_GR_Ioannina,T2_HU_Budapest,T2_IN_TIFR,T2_IT_Bari,T2_IT_Legnaro,T2_IT_Pisa,T2_IT_Rome,T2_KR_KISTI,T2_MY_SIFIR,T2_MY_UPM_BIRUNI,T2_PK_NCP,T2_PL_Swierk,T2_PL_Warsaw,T2_PT_NCG_Lisbon,T2_RU_IHEP,T2_RU_INR,T2_RU_ITEP,T2_RU_JINR,T2_RU_PNPI,T2_RU_SINP,T2_TH_CUNSTDA,T2_TR_METU,T2_TW_NCHC,T2_UA_KIPT,T2_UK_London_IC,T2_UK_SGrid_Bristol,T2_UK_SGrid_RALPP,T2_US_Caltech,T2_US_Florida,T2_US_Nebraska,T2_US_Purdue,T2_US_UCSD,T2_US_Vanderbilt,T2_US_Wisconsin,T3_CH_CERN_CAF,T3_CH_CERN_DOMA,T3_CH_CERN_HelixNebula,T3_CH_CERN_HelixNebula_REHA,T3_CH_CMSAtHome,T3_CH_Volunteer,T3_US_HEPCloud,T3_US_NERSC,T3_US_OSG,T3_US_PSC,T3_US_SDSC,T3_US_MIT"  
use_x509userproxy = True
x509userproxy = {HOME_USER}x509up_u238361
output = {off_logs_dir}job.$(Process).out
error = {off_logs_dir}job.$(Process).err
log = {off_logs_dir}job.$(Process).log
#RequestMemory = 4000
Universe = vanilla
queue {off_njobs}"""

with open(DP_USER+"pull_data/offline/condor_off.sub", "w") as file:
    file.write(template_offline_condor)


