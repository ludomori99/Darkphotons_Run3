import os
from config_loader import load_analysis_config,load_test_config

# Script to write the bash scripts with proper current configuration 

HOME_USER = os.environ["HOMELUDO"]
DP_USER = os.environ["DPLUDO"]
OFFLINE_FOLDER = os.path.join(DP_USER,"pull_data/offline/") 
MCR3_FOLDER = os.path.join(DP_USER,"pull_data/MCRun3/") 

config = load_analysis_config()

#Extract config parameters for submission
offline_grouping_amount_data = config["condor_off"]["grouping_amount_data"]
off_event_fraction = config["condor_off"]["event_fraction"]
off_njobs = config["condor_off"]["njobs"]
off_dump_dir = config["locations"]["offline"]["dump"]
off_logs_dir = config["locations"]["offline"]["logs"]

MCRun3_grouping_amount_data = config["condor_MC"]["grouping_amount_data"]
MCRun3_event_fraction = config["condor_MC"]["event_fraction"]
MCRun3_njobs = config["condor_MC"]["njobs"]
MCRun3_dump_dir = config["locations"]["MCRun3"]["dump"]
MCRun3_logs_dir = config["locations"]["MCRun3"]["logs"]
MCRun3_Jpsi_dir = config["locations"]["MCRun3"]["Jpsi"]
MCRun3_Y_dir = config["locations"]["MCRun3"]["Y"]

MC_grouping_amount_data = config["MC_slurm"]["grouping_amount_data"]
MC_miniaod = config["locations"]["MC"]["MINIAOD"]
MC_Jpsi_dir = config["locations"]["MC"]["Jpsi"]
MC_Y_dir = config["locations"]["MC"]["Y"]
MC_logs_dir = config["locations"]["MC"]["logs"]


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

start=$(( $1*{offline_grouping_amount_data}+1 ))
end=$(( $start+{offline_grouping_amount_data} ))

for (( N=start; N<end; N++ )); do

    file=$(cat ./list.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file input.root
    root -l -b -q generateOffDimuonTree.C\(\"input.root\"\,\"r3tree_$N.root\"\,{off_event_fraction}\)

done
hadd r3tree.root r3tree_*.root
xrdcp -f r3tree.root root://submit50.mit.edu//{off_dump_dir}/DimuonTree$1.root
"""
with open(os.path.join(OFFLINE_FOLDER,"run_merge_off.sh"), "w") as file:
    file.write(template_offline_bash)

### CONDOR SCRIPT for offline 
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
+DESIRED_Sites = "mit_tier3,mit_tier2,T2_AT_Vienna,T2_BE_IIHE,T2_BE_UCL,T2_BR_SPRACE,T2_BR_UERJ,T2_CH_CERN,T2_CH_CERN_AI,T2_CH_CERN_HLT,T2_CH_CERN_Wigner,T2_CH_CSCS,T2_CH_CSCS_HPC,T2_CN_Beijing,T2_DE_DESY,T2_DE_RWTH,T2_EE_Estonia,T2_ES_CIEMAT,T2_ES_IFCA,T2_FI_HIP,T2_FR_CCIN2P3,T2_FR_GRIF_IRFU,T2_FR_GRIF_LLR,T2_FR_IPHC,T2_GR_Ioannina,T2_HU_Budapest,T2_IN_TIFR,T2_IT_Bari,T2_IT_Legnaro,T2_IT_Pisa,T2_IT_Rome,T2_KR_KISTI,T2_MY_SIFIR,T2_MY_UPM_BIRUNI,T2_PK_NCP,T2_PL_Swierk,T2_PL_Warsaw,T2_PT_NCG_Lisbon,T2_RU_IHEP,T2_RU_INR,T2_RU_ITEP,T2_RU_JINR,T2_RU_PNPI,T2_RU_SINP,T2_TH_CUNSTDA,T2_TR_METU,T2_TW_NCHC,T2_UA_KIPT,T2_UK_London_IC,T2_UK_SGrid_Bristol,T2_UK_SGrid_RALPP,T2_US_Caltech,T2_US_Nebraska,T2_US_Purdue,T2_US_UCSD,T3_CH_CERN_CAF,T3_CH_CERN_DOMA,T3_CH_CERN_HelixNebula,T3_CH_CERN_HelixNebula_REHA,T3_CH_CMSAtHome,T3_CH_Volunteer,T3_US_HEPCloud,T3_US_NERSC,T3_US_OSG,T3_US_PSC,T3_US_SDSC,T3_US_MIT"  
use_x509userproxy = True
x509userproxy = {HOME_USER}x509up_u238361
output = {off_logs_dir}job.$(Process).out
error = {off_logs_dir}job.$(Process).err
log = {off_logs_dir}job.$(Process).log
#RequestMemory = 4000
Universe = vanilla
queue {off_njobs}"""
with open(os.path.join(OFFLINE_FOLDER,"condor_off.sub"), "w") as file:
    file.write(template_offline_condor)

### Scripts to import Run3MC from condor
template_MCRun3_Jpsi_bash = rf"""
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

start=$(($1*{MCRun3_grouping_amount_data}+1))
end=$(($start+{MCRun3_grouping_amount_data}))

for (( N=start; N<end; N++ )); do

    file=$(cat ./list.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file input.root
    root -l -b -q generateMCDimuonTree.C\(\"input.root\"\,\"r3tree_$N.root\"\,443\,{MCRun3_event_fraction}\)

done
hadd r3tree.root r3tree_*.root
xrdcp -f r3tree.root root://submit50.mit.edu//{MCRun3_dump_dir}DimuonTreeJpsi$1.root
"""
with open(os.path.join(MCR3_FOLDER,"run_merge_MCRun3_Jpsi.sh"), "w") as file:
    file.write(template_MCRun3_Jpsi_bash)


template_MCRun3_Y_bash = rf"""
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

start=$(($1*{MCRun3_grouping_amount_data}+1))
end=$(($start+{MCRun3_grouping_amount_data}))

for (( N=start; N<end; N++ )); do

    file=$(cat ./list.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file input.root
    root -l -b -q generateMCDimuonTree.C\(\"input.root\"\,\"r3tree_$N.root\"\,553\,{MCRun3_event_fraction}\)

done
hadd r3tree.root r3tree_*.root
xrdcp -f r3tree.root root://submit50.mit.edu//{MCRun3_dump_dir}DimuonTreeY$1.root
"""
with open(os.path.join(MCR3_FOLDER,"run_merge_MCRun3_Y.sh"), "w") as file:
    file.write(template_MCRun3_Y_bash)



template_MCRun3_condor_Jpsi = rf"""executable = {MCR3_FOLDER}run_merge_MCRun3_Jpsi.sh
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = {MCR3_FOLDER}list.txt,{MCR3_FOLDER}generateMCDimuonTree.C
transfer_output_files = ""
MyIndex=$(ProcId)
Extention=$INT(MyIndex,%d)
arguments = "$(Extention)"
Requirements = ( BOSCOCluster =!= "t3serv008.mit.edu" && BOSCOCluster =!= "ce03.cmsaf.mit.edu" && BOSCOCluster =!= "eofe8.mit.edu")
+REQUIRED_OS = "rhel7"
+DESIRED_Sites = "mit_tier3,mit_tier2,T2_AT_Vienna,T2_BE_IIHE,T2_BE_UCL,T2_BR_SPRACE,T2_BR_UERJ,T2_CH_CERN,T2_CH_CERN_AI,T2_CH_CERN_HLT,T2_CH_CERN_Wigner,T2_CH_CSCS,T2_CH_CSCS_HPC,T2_CN_Beijing,T2_DE_DESY,T2_DE_RWTH,T2_EE_Estonia,T2_ES_CIEMAT,T2_ES_IFCA,T2_FI_HIP,T2_FR_CCIN2P3,T2_FR_GRIF_IRFU,T2_FR_GRIF_LLR,T2_FR_IPHC,T2_GR_Ioannina,T2_HU_Budapest,T2_IN_TIFR,T2_IT_Bari,T2_IT_Legnaro,T2_IT_Pisa,T2_IT_Rome,T2_KR_KISTI,T2_MY_SIFIR,T2_MY_UPM_BIRUNI,T2_PK_NCP,T2_PL_Swierk,T2_PL_Warsaw,T2_PT_NCG_Lisbon,T2_RU_IHEP,T2_RU_INR,T2_RU_ITEP,T2_RU_JINR,T2_RU_PNPI,T2_RU_SINP,T2_TH_CUNSTDA,T2_TR_METU,T2_TW_NCHC,T2_UA_KIPT,T2_UK_London_IC,T2_UK_SGrid_Bristol,T2_UK_SGrid_RALPP,T2_US_Caltech,T2_US_Nebraska,T2_US_Purdue,T2_US_UCSD,T3_CH_CERN_CAF,T3_CH_CERN_DOMA,T3_CH_CERN_HelixNebula,T3_CH_CERN_HelixNebula_REHA,T3_CH_CMSAtHome,T3_CH_Volunteer,T3_US_HEPCloud,T3_US_NERSC,T3_US_OSG,T3_US_PSC,T3_US_SDSC,T3_US_MIT"  
use_x509userproxy = True
x509userproxy = {HOME_USER}x509up_u238361
output = {MCRun3_logs_dir}jobJpsi.$(Process).out
error = {MCRun3_logs_dir}jobJpsi.$(Process).err
log = {MCRun3_logs_dir}jobJpsi.$(Process).log
#RequestMemory = 4000
Universe = vanilla
queue {MCRun3_njobs}"""
with open(os.path.join(MCR3_FOLDER,"condor_MCRun3_Jpsi.sub"), "w") as file:
    file.write(template_MCRun3_condor_Jpsi)


template_MCRun3_condor_Y = rf"""executable = {MCR3_FOLDER}run_merge_MCRun3_Y.sh
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = {MCR3_FOLDER}list.txt,{MCR3_FOLDER}generateMCDimuonTree.C
transfer_output_files = ""
MyIndex=$(ProcId)
Extention=$INT(MyIndex,%d)
arguments = "$(Extention)"
Requirements = ( BOSCOCluster =!= "t3serv008.mit.edu" && BOSCOCluster =!= "ce03.cmsaf.mit.edu" && BOSCOCluster =!= "eofe8.mit.edu")
+REQUIRED_OS = "rhel7"
+DESIRED_Sites = "mit_tier3,mit_tier2,T2_AT_Vienna,T2_BE_IIHE,T2_BE_UCL,T2_BR_SPRACE,T2_BR_UERJ,T2_CH_CERN,T2_CH_CERN_AI,T2_CH_CERN_HLT,T2_CH_CERN_Wigner,T2_CH_CSCS,T2_CH_CSCS_HPC,T2_CN_Beijing,T2_DE_DESY,T2_DE_RWTH,T2_EE_Estonia,T2_ES_CIEMAT,T2_ES_IFCA,T2_FI_HIP,T2_FR_CCIN2P3,T2_FR_GRIF_IRFU,T2_FR_GRIF_LLR,T2_FR_IPHC,T2_GR_Ioannina,T2_HU_Budapest,T2_IN_TIFR,T2_IT_Bari,T2_IT_Legnaro,T2_IT_Pisa,T2_IT_Rome,T2_KR_KISTI,T2_MY_SIFIR,T2_MY_UPM_BIRUNI,T2_PK_NCP,T2_PL_Swierk,T2_PL_Warsaw,T2_PT_NCG_Lisbon,T2_RU_IHEP,T2_RU_INR,T2_RU_ITEP,T2_RU_JINR,T2_RU_PNPI,T2_RU_SINP,T2_TH_CUNSTDA,T2_TR_METU,T2_TW_NCHC,T2_UA_KIPT,T2_UK_London_IC,T2_UK_SGrid_Bristol,T2_UK_SGrid_RALPP,T2_US_Caltech,T2_US_Nebraska,T2_US_Purdue,T2_US_UCSD,T3_CH_CERN_CAF,T3_CH_CERN_DOMA,T3_CH_CERN_HelixNebula,T3_CH_CERN_HelixNebula_REHA,T3_CH_CMSAtHome,T3_CH_Volunteer,T3_US_HEPCloud,T3_US_NERSC,T3_US_OSG,T3_US_PSC,T3_US_SDSC,T3_US_MIT"  
use_x509userproxy = True
x509userproxy = {HOME_USER}x509up_u238361
output = {MCRun3_logs_dir}jobY.$(Process).out
error = {MCRun3_logs_dir}jobY.$(Process).err
log = {MCRun3_logs_dir}jobY.$(Process).log
#RequestMemory = 4000
Universe = vanilla
queue {MCRun3_njobs}"""
with open(os.path.join(MCR3_FOLDER,"condor_MCRun3_Y.sub"), "w") as file:
    file.write(template_MCRun3_condor_Y)


### BASH SCRIPT FOR FINAL MERGING OF MC AND CLEANUP #####
template_MCRun3_merge_final_bash = rf"""
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd {DP_USER}CMSSW_13_0_6/src
eval `scramv1 runtime -sh`
cd {MCR3_FOLDER}
hadd -f {MCRun3_Jpsi_dir}merged_A.root /data/submit/{MCRun3_dump_dir}DimuonTreeJpsi*.root
hadd -f {MCRun3_Y_dir}merged_A.root /data/submit/{MCRun3_dump_dir}DimuonTreeY*.root

# rm {MC_Y_dir}MC*
# rm {MC_Jpsi_dir}MC*
"""
with open(os.path.join(MCR3_FOLDER,"merge_final.sh"), "w") as file:
    file.write(template_MCRun3_merge_final_bash)





### BASH SCRIPT for Y meson MC data to be submitted in bash (From here on Run2 MC)
template_MC_Y_bash = rf"""
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd {DP_USER}CMSSW_13_0_6/src
eval `scramv1 runtime -sh`
cd {DP_USER}pull_data/MC/

start=$(( $1*{MC_grouping_amount_data}+1 ))
end=$(( $start+{MC_grouping_amount_data} ))

for (( N=start; N<end; N++ )); do
        file=$( cat files.txt | sed -n ''$N'p')
        echo running $file
        infile="{MC_miniaod}$file"
        root -l -b -q generateMCDimuonTree.C\(\"$infile\"\,\"{MC_Y_dir}MCtree_$N.root\"\,553\)
    done
"""#hadd -f {MC_Y_dir}MCDimuonTree_merged_$1.root {MC_Y_dir}MCtree_*.root
with open(os.path.join(DP_USER,"pull_data/MC/run_merge_MC_Y.sh"), "w") as file:
    file.write(template_MC_Y_bash)


### BASH SCRIPT for Jpsi meson MC data to be submitted in bash 
template_MC_Jpsi_bash = rf"""
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd {DP_USER}CMSSW_13_0_6/src
eval `scramv1 runtime -sh`
cd {DP_USER}pull_data/MC/

start=$(( $1*{MC_grouping_amount_data}+1 ))
end=$(( $start+{MC_grouping_amount_data} ))

for (( N=start; N<end; N++ )); do
        file=$( cat files.txt | sed -n ''$N'p')
        echo running $file
        infile="{MC_miniaod}$file"
        root -l -b -q generateMCDimuonTree.C\(\"$infile\"\,\"{MC_Jpsi_dir}MCtree_$N.root\"\,443\)
    done

"""
# hadd -f {MC_Jpsi_dir}MCDimuonTree_merged_$1.root {MC_Jpsi_dir}MCtree_*.root
with open(os.path.join(DP_USER,"pull_data/MC/run_merge_MC_Jpsi.sh"), "w") as file:
    file.write(template_MC_Jpsi_bash)


### BASH SCRIPT FOR FINAL MERGING OF MC AND CLEANUP #####

template_MC_merge_final_bash = rf"""
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd {DP_USER}CMSSW_13_0_6/src
eval `scramv1 runtime -sh`
cd {DP_USER}pull_data/MC/
hadd -f {MC_Y_dir}merged.root {MC_Y_dir}MCtree*.root
hadd -f {MC_Jpsi_dir}merged.root {MC_Jpsi_dir}MCtree*.root

# rm {MC_Y_dir}MC*
# rm {MC_Jpsi_dir}MC*
"""
with open(os.path.join(DP_USER,"pull_data/MC/merge_final.sh"), "w") as file:
    file.write(template_MC_merge_final_bash)


