import os
from config_loader import load_analysis_config,load_test_config

# Script to write the bash scripts with proper current configuration 

HOME_USER = os.environ["HOMEUSER"]
DP_USER = os.environ["DPUSER"]
OFFLINE_FOLDER = os.path.join(DP_USER,"pull_data/offline/")
MCMINBIAS_FOLDER = os.path.join(DP_USER,"pull_data/MC_InclusiveMinBias/") 
MCLMDY_FOLDER = os.path.join(DP_USER,"pull_data/MC_lmDY/") 

config = load_analysis_config()

#Extract config parameters for submission
off_grouping_amount_data = config["extraction"]["offline"]["grouping_amount_data"]
off_event_fraction = config["extraction"]["offline"]["event_fraction"]
off_njobs = config["extraction"]["offline"]["njobs"]
off_dump_short_dir = config["locations"]["offline"]["dump_short"]
off_logs_dir = config["locations"]["offline"]["logs"]

MC_lmDY_grouping_amount_data = config["extraction"]["MC_lmDY"]["grouping_amount_data"]
MC_lmDY_event_fraction = config["extraction"]["MC_lmDY"]["event_fraction"]
MC_lmDY_dump_dir = config["locations"]["MC_lmDY"]["dump"]
MC_lmDY_tmp_dir = config["locations"]["MC_lmDY"]["tmp"]

MC_InclusiveMinBias_grouping_amount_data = config["extraction"]["MC_InclusiveMinBias"]["grouping_amount_data"]
MC_InclusiveMinBias_event_fraction = config["extraction"]["MC_InclusiveMinBias"]["event_fraction"]
MC_InclusiveMinBias_njobs = config["extraction"]["MC_InclusiveMinBias"]["njobs"]
MC_InclusiveMinBias_dump_short_dir = config["locations"]["MC_InclusiveMinBias"]["dump_short"]
MC_InclusiveMinBias_logs_dir = config["locations"]["MC_InclusiveMinBias"]["logs"]
MC_InclusiveMinBias_Jpsi_dir = config["locations"]["MC_InclusiveMinBias"]["Jpsi"]
MC_InclusiveMinBias_Y_dir = config["locations"]["MC_InclusiveMinBias"]["Y"]


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

start=$(( $1*{off_grouping_amount_data}+1 ))
end=$(( $start+{off_grouping_amount_data} ))

for (( N=start; N<end; N++ )); do

    file=$(cat ./list_offline.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file input.root
    root -l -b -q generateOffDimuonTree.C\(\"input.root\"\,\"r3tree_$N.root\"\,{off_event_fraction}\)

done
hadd r3tree.root r3tree_*.root
xrdcp -f r3tree.root root://submit50.mit.edu//{off_dump_short_dir}/DimuonTree$1.root
"""
with open(os.path.join(OFFLINE_FOLDER,"run_merge_off.sh"), "w") as file:
    file.write(template_offline_bash)


#### ,T2_AT_Vienna,T2_BE_IIHE,T2_BE_UCL,T2_BR_SPRACE,T2_BR_UERJ,T2_CH_CERN,T2_CH_CERN_AI,T2_CH_CERN_HLT,T2_CH_CERN_Wigner,T2_CH_CSCS,T2_CH_CSCS_HPC,T2_CN_Beijing,T2_DE_DESY,T2_DE_RWTH,T2_EE_Estonia,T2_ES_CIEMAT,T2_ES_IFCA,T2_FI_HIP,T2_FR_CCIN2P3,T2_FR_GRIF_IRFU,T2_FR_GRIF_LLR,T2_FR_IPHC,T2_GR_Ioannina,T2_HU_Budapest,T2_IN_TIFR,T2_IT_Bari,T2_IT_Legnaro,T2_IT_Pisa,T2_IT_Rome,T2_KR_KISTI,T2_MY_SIFIR,T2_MY_UPM_BIRUNI,T2_PK_NCP,T2_PL_Swierk,T2_PL_Warsaw,T2_PT_NCG_Lisbon,T2_RU_IHEP,T2_RU_INR,T2_RU_ITEP,T2_RU_JINR,T2_RU_PNPI,T2_RU_SINP,T2_TH_CUNSTDA,T2_TR_METU,T2_TW_NCHC,T2_UA_KIPT,T2_UK_London_IC,T2_UK_SGrid_Bristol,T2_UK_SGrid_RALPP,T2_US_Caltech,T2_US_Nebraska,T2_US_Purdue,T2_US_UCSD,T3_CH_CERN_CAF,T3_CH_CERN_DOMA,T3_CH_CERN_HelixNebula,T3_CH_CERN_HelixNebula_REHA,T3_CH_CMSAtHome,T3_CH_Volunteer,T3_US_HEPCloud,T3_US_NERSC,T3_US_OSG,T3_US_PSC,T3_US_SDSC,T3_US_MIT
### CONDOR SCRIPT for offline 
template_offline_condor = rf"""executable = {OFFLINE_FOLDER}run_merge_off.sh
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = {OFFLINE_FOLDER}list_offline.txt,{OFFLINE_FOLDER}generateOffDimuonTree.C
transfer_output_files = ""
MyIndex=$(ProcId)
Extention=$INT(MyIndex,%d)
arguments = "$(Extention)"
Requirements = ( BOSCOCluster =!= "t3serv008.mit.edu" && BOSCOCluster =!= "ce03.cmsaf.mit.edu" && BOSCOCluster =!= "eofe8.mit.edu")
+REQUIRED_OS = "rhel7"
+DESIRED_Sites = "mit_tier3,mit_tier2"  
use_x509userproxy = True
x509userproxy = {HOME_USER}x509up_u238361
output = {off_logs_dir}job.$(Process).out
error = {off_logs_dir}job.$(Process).err
log = {off_logs_dir}job.$(Process).log
#RequestMemory = 4000
Universe = vanilla
queue {off_njobs}"""
with open(os.path.join(OFFLINE_FOLDER,"offline.sub"), "w") as file:
    file.write(template_offline_condor)


### MC lmDY
template_MC_lmDY_bash = rf"""
RELEASE=CMSSW_13_0_6
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd $RELEASE/src
eval `scramv1 runtime -sh` # cmsenv is an alias not on the workers
cd {MCLMDY_FOLDER} 

start=$(( $1*{MC_lmDY_grouping_amount_data}+1 ))
end=$(( $start+{MC_lmDY_grouping_amount_data} ))
echo start job $1

for (( N=start; N<end; N++ )); do
    file=$(cat ./list_lmDY.txt | sed -n ''$N'p')
    echo running $file
    root -l -b -q generateMCDimuonTree.C\(\"$file\"\,\"{MC_lmDY_tmp_dir}DimuonTreeTMP$1_$N.root\"\,1\,{MC_lmDY_event_fraction}\,false\)
done
hadd -f {MC_lmDY_dump_dir}DimuonTree$1.root {MC_lmDY_tmp_dir}DimuonTreeTMP$1_*.root    
rm {MC_lmDY_tmp_dir}DimuonTreeTMP$1_*.root

"""
with open(os.path.join(MCLMDY_FOLDER,"run_merge_MC_lmDY.sh"), "w") as file:
    file.write(template_MC_lmDY_bash)

### Scripts to import MC_InclusiveMinBias from condor
template_MC_InclusiveMinBias_Jpsi_bash = rf"""
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

start=$(($1*{MC_InclusiveMinBias_grouping_amount_data}+1))
end=$(($start+{MC_InclusiveMinBias_grouping_amount_data}))

for (( N=start; N<end; N++ )); do

    file=$(cat ./list_InclMinBias.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file input.root
    root -l -b -q generateMCDimuonTree.C\(\"input.root\"\,\"r3tree_$N.root\"\,443\,{MC_InclusiveMinBias_event_fraction}\)

done
hadd r3tree.root r3tree_*.root
xrdcp -f r3tree.root root://submit50.mit.edu//{MC_InclusiveMinBias_dump_short_dir}DimuonTreeJpsi$1.root
"""
with open(os.path.join(MCMINBIAS_FOLDER,"run_merge_MC_InclusiveMinBias_Jpsi.sh"), "w") as file:
    file.write(template_MC_InclusiveMinBias_Jpsi_bash)


template_MC_InclusiveMinBias_Y_bash = rf"""
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

start=$(($1*{MC_InclusiveMinBias_grouping_amount_data}+1))
end=$(($start+{MC_InclusiveMinBias_grouping_amount_data}))

for (( N=start; N<end; N++ )); do

    file=$(cat ./list_InclMinBias.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file input.root
    root -l -b -q generateMCDimuonTree.C\(\"input.root\"\,\"r3treeY1_$N.root\"\,553\,{MC_InclusiveMinBias_event_fraction}\)
    root -l -b -q generateMCDimuonTree.C\(\"input.root\"\,\"r3treeY2_$N.root\"\,100553\,{MC_InclusiveMinBias_event_fraction}\)
    root -l -b -q generateMCDimuonTree.C\(\"input.root\"\,\"r3treeY3_$N.root\"\,200553\,{MC_InclusiveMinBias_event_fraction}\)
done
hadd r3treeY1.root r3treeY1_*.root
hadd r3treeY2.root r3treeY2_*.root
hadd r3treeY3.root r3treeY3_*.root
xrdcp -f r3treeY1.root root://submit50.mit.edu//{MC_InclusiveMinBias_dump_short_dir}DimuonTreeY1_$1.root
xrdcp -f r3treeY2.root root://submit50.mit.edu//{MC_InclusiveMinBias_dump_short_dir}DimuonTreeY2_$1.root
xrdcp -f r3treeY3.root root://submit50.mit.edu//{MC_InclusiveMinBias_dump_short_dir}DimuonTreeY3_$1.root
"""
with open(os.path.join(MCMINBIAS_FOLDER,"run_merge_MC_InclusiveMinBias_Y.sh"), "w") as file:
    file.write(template_MC_InclusiveMinBias_Y_bash)


template_MC_InclusiveMinBias_bash = rf"""
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

start=$(($1*{MC_InclusiveMinBias_grouping_amount_data}+1))
end=$(($start+{MC_InclusiveMinBias_grouping_amount_data}))

for (( N=start; N<end; N++ )); do

    file=$(cat ./list_InclMinBias.txt | sed -n ''$N'p')
    echo running $file
    xrdcp -f root://eoscms.cern.ch/$file input.root
    root -l -b -q generateMCDimuonTree.C\(\"input.root\"\,\"r3tree_$N.root\"\,0\,{MC_InclusiveMinBias_event_fraction}\,\false\)
done
hadd r3tree.root r3tree_*.root
xrdcp -f r3tree.root root://submit50.mit.edu//{MC_InclusiveMinBias_dump_short_dir}DimuonTree_$1.root
"""
with open(os.path.join(MCMINBIAS_FOLDER,"run_merge_MC_InclusiveMinBias.sh"), "w") as file:
    file.write(template_MC_InclusiveMinBias_bash)

template_MC_InclusiveMinBias_Jpsi_condor = rf"""executable = {MCMINBIAS_FOLDER}run_merge_MC_InclusiveMinBias_Jpsi.sh
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = {MCMINBIAS_FOLDER}list_InclMinBias.txt,{MCMINBIAS_FOLDER}generateMCDimuonTree.C
transfer_output_files = ""
MyIndex=$(ProcId)
Extention=$INT(MyIndex,%d)
arguments = "$(Extention)"
Requirements = ( BOSCOCluster =!= "t3serv008.mit.edu" && BOSCOCluster =!= "ce03.cmsaf.mit.edu" && BOSCOCluster =!= "eofe8.mit.edu")
+REQUIRED_OS = "rhel7"
+DESIRED_Sites = "mit_tier3,mit_tier2,T2_AT_Vienna,T2_BE_IIHE,T2_BE_UCL,T2_BR_SPRACE,T2_BR_UERJ,T2_CH_CERN,T2_CH_CERN_AI,T2_CH_CERN_HLT,T2_CH_CERN_Wigner,T2_CH_CSCS,T2_CH_CSCS_HPC,T2_CN_Beijing,T2_DE_DESY,T2_DE_RWTH,T2_EE_Estonia,T2_ES_CIEMAT,T2_ES_IFCA,T2_FI_HIP,T2_FR_CCIN2P3,T2_FR_GRIF_IRFU,T2_FR_GRIF_LLR,T2_FR_IPHC,T2_GR_Ioannina,T2_HU_Budapest,T2_IN_TIFR,T2_IT_Bari,T2_IT_Legnaro,T2_IT_Pisa,T2_IT_Rome,T2_KR_KISTI,T2_MY_SIFIR,T2_MY_UPM_BIRUNI,T2_PK_NCP,T2_PL_Swierk,T2_PL_Warsaw,T2_PT_NCG_Lisbon,T2_RU_IHEP,T2_RU_INR,T2_RU_ITEP,T2_RU_JINR,T2_RU_PNPI,T2_RU_SINP,T2_TH_CUNSTDA,T2_TR_METU,T2_TW_NCHC,T2_UA_KIPT,T2_UK_London_IC,T2_UK_SGrid_Bristol,T2_UK_SGrid_RALPP,T2_US_Caltech,T2_US_Nebraska,T2_US_Purdue,T2_US_UCSD,T3_CH_CERN_CAF,T3_CH_CERN_DOMA,T3_CH_CERN_HelixNebula,T3_CH_CERN_HelixNebula_REHA,T3_CH_CMSAtHome,T3_CH_Volunteer,T3_US_HEPCloud,T3_US_NERSC,T3_US_OSG,T3_US_PSC,T3_US_SDSC,T3_US_MIT"  
use_x509userproxy = True
x509userproxy = {HOME_USER}x509up_u238361
output = {MC_InclusiveMinBias_logs_dir}jobJpsi.$(Process).out
error = {MC_InclusiveMinBias_logs_dir}jobJpsi.$(Process).err
log = {MC_InclusiveMinBias_logs_dir}jobJpsi.$(Process).log
#RequestMemory = 4000
Universe = vanilla
queue {MC_InclusiveMinBias_njobs}"""
with open(os.path.join(MCMINBIAS_FOLDER,"MC_InclusiveMinBias_Jpsi.sub"), "w") as file:
    file.write(template_MC_InclusiveMinBias_Jpsi_condor)


template_MC_InclusiveMinBias_Y_condor = rf"""executable = {MCMINBIAS_FOLDER}run_merge_MC_InclusiveMinBias_Y.sh
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = {MCMINBIAS_FOLDER}list_InclMinBias.txt,{MCMINBIAS_FOLDER}generateMCDimuonTree.C
transfer_output_files = ""
MyIndex=$(ProcId)
Extention=$INT(MyIndex,%d)
arguments = "$(Extention)"
Requirements = ( BOSCOCluster =!= "t3serv008.mit.edu" && BOSCOCluster =!= "ce03.cmsaf.mit.edu" && BOSCOCluster =!= "eofe8.mit.edu")
+REQUIRED_OS = "rhel7"
+DESIRED_Sites = "mit_tier3,mit_tier2,T2_AT_Vienna,T2_BE_IIHE,T2_BE_UCL,T2_BR_SPRACE,T2_BR_UERJ,T2_CH_CERN,T2_CH_CERN_AI,T2_CH_CERN_HLT,T2_CH_CERN_Wigner,T2_CH_CSCS,T2_CH_CSCS_HPC,T2_CN_Beijing,T2_DE_DESY,T2_DE_RWTH,T2_EE_Estonia,T2_ES_CIEMAT,T2_ES_IFCA,T2_FI_HIP,T2_FR_CCIN2P3,T2_FR_GRIF_IRFU,T2_FR_GRIF_LLR,T2_FR_IPHC,T2_GR_Ioannina,T2_HU_Budapest,T2_IN_TIFR,T2_IT_Bari,T2_IT_Legnaro,T2_IT_Pisa,T2_IT_Rome,T2_KR_KISTI,T2_MY_SIFIR,T2_MY_UPM_BIRUNI,T2_PK_NCP,T2_PL_Swierk,T2_PL_Warsaw,T2_PT_NCG_Lisbon,T2_RU_IHEP,T2_RU_INR,T2_RU_ITEP,T2_RU_JINR,T2_RU_PNPI,T2_RU_SINP,T2_TH_CUNSTDA,T2_TR_METU,T2_TW_NCHC,T2_UA_KIPT,T2_UK_London_IC,T2_UK_SGrid_Bristol,T2_UK_SGrid_RALPP,T2_US_Caltech,T2_US_Nebraska,T2_US_Purdue,T2_US_UCSD,T3_CH_CERN_CAF,T3_CH_CERN_DOMA,T3_CH_CERN_HelixNebula,T3_CH_CERN_HelixNebula_REHA,T3_CH_CMSAtHome,T3_CH_Volunteer,T3_US_HEPCloud,T3_US_NERSC,T3_US_OSG,T3_US_PSC,T3_US_SDSC,T3_US_MIT"  
use_x509userproxy = True
x509userproxy = {HOME_USER}x509up_u238361
output = {MC_InclusiveMinBias_logs_dir}jobY.$(Process).out
error = {MC_InclusiveMinBias_logs_dir}jobY.$(Process).err
log = {MC_InclusiveMinBias_logs_dir}jobY.$(Process).log
#RequestMemory = 4000
Universe = vanilla
queue {MC_InclusiveMinBias_njobs}"""
with open(os.path.join(MCMINBIAS_FOLDER,"MC_InclusiveMinBias_Y.sub"), "w") as file:
    file.write(template_MC_InclusiveMinBias_Y_condor)

template_MC_InclusiveMinBias_condor = rf"""executable = {MCMINBIAS_FOLDER}run_merge_MC_InclusiveMinBias.sh
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = {MCMINBIAS_FOLDER}list_InclMinBias.txt,{MCMINBIAS_FOLDER}generateMCDimuonTree.C
transfer_output_files = ""
MyIndex=$(ProcId)
Extention=$INT(MyIndex,%d)
arguments = "$(Extention)"
Requirements = ( BOSCOCluster =!= "t3serv008.mit.edu" && BOSCOCluster =!= "ce03.cmsaf.mit.edu" && BOSCOCluster =!= "eofe8.mit.edu")
+REQUIRED_OS = "rhel7"
+DESIRED_Sites = "mit_tier3,mit_tier2,T2_AT_Vienna,T2_BE_IIHE,T2_BE_UCL,T2_BR_SPRACE,T2_BR_UERJ,T2_CH_CERN,T2_CH_CERN_AI,T2_CH_CERN_HLT,T2_CH_CERN_Wigner,T2_CH_CSCS,T2_CH_CSCS_HPC,T2_CN_Beijing,T2_DE_DESY,T2_DE_RWTH,T2_EE_Estonia,T2_ES_CIEMAT,T2_ES_IFCA,T2_FI_HIP,T2_FR_CCIN2P3,T2_FR_GRIF_IRFU,T2_FR_GRIF_LLR,T2_FR_IPHC,T2_GR_Ioannina,T2_HU_Budapest,T2_IN_TIFR,T2_IT_Bari,T2_IT_Legnaro,T2_IT_Pisa,T2_IT_Rome,T2_KR_KISTI,T2_MY_SIFIR,T2_MY_UPM_BIRUNI,T2_PK_NCP,T2_PL_Swierk,T2_PL_Warsaw,T2_PT_NCG_Lisbon,T2_RU_IHEP,T2_RU_INR,T2_RU_ITEP,T2_RU_JINR,T2_RU_PNPI,T2_RU_SINP,T2_TH_CUNSTDA,T2_TR_METU,T2_TW_NCHC,T2_UA_KIPT,T2_UK_London_IC,T2_UK_SGrid_Bristol,T2_UK_SGrid_RALPP,T2_US_Caltech,T2_US_Nebraska,T2_US_Purdue,T2_US_UCSD,T3_CH_CERN_CAF,T3_CH_CERN_DOMA,T3_CH_CERN_HelixNebula,T3_CH_CERN_HelixNebula_REHA,T3_CH_CMSAtHome,T3_CH_Volunteer,T3_US_HEPCloud,T3_US_NERSC,T3_US_OSG,T3_US_PSC,T3_US_SDSC,T3_US_MIT"  
use_x509userproxy = True
x509userproxy = {HOME_USER}x509up_u238361
output = {MC_InclusiveMinBias_logs_dir}job.$(Process).out
error = {MC_InclusiveMinBias_logs_dir}job.$(Process).err
log = {MC_InclusiveMinBias_logs_dir}job.$(Process).log
#RequestMemory = 4000
Universe = vanilla
queue {MC_InclusiveMinBias_njobs}"""
with open(os.path.join(MCMINBIAS_FOLDER,"MC_InclusiveMinBias.sub"), "w") as file:
    file.write(template_MC_InclusiveMinBias_condor)


### BASH SCRIPT FOR FINAL MERGING OF MC AND CLEANUP #####
template_MC_InclusiveMinBias_merge_final_bash = rf"""
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd {DP_USER}CMSSW_13_0_6/src
eval `scramv1 runtime -sh`
cd {MCMINBIAS_FOLDER}
hadd -f {MC_InclusiveMinBias_Jpsi_dir}merged_A.root /data/submit/{MC_InclusiveMinBias_dump_short_dir}DimuonTreeJpsi*.root
hadd -f {MC_InclusiveMinBias_Y_dir}mergedY1_A.root /data/submit/{MC_InclusiveMinBias_dump_short_dir}DimuonTreeY1_*.root
hadd -f {MC_InclusiveMinBias_Y_dir}mergedY2_A.root /data/submit/{MC_InclusiveMinBias_dump_short_dir}DimuonTreeY2_*.root
hadd -f {MC_InclusiveMinBias_Y_dir}mergedY3_A.root /data/submit/{MC_InclusiveMinBias_dump_short_dir}DimuonTreeY3_*.root
hadd -f {MC_InclusiveMinBias_Y_dir}mergedY12_A.root {MC_InclusiveMinBias_Y_dir}mergedY1_A.root {MC_InclusiveMinBias_Y_dir}mergedY2_A.root
hadd -f {MC_InclusiveMinBias_Y_dir}mergedY13_A.root {MC_InclusiveMinBias_Y_dir}mergedY1_A.root {MC_InclusiveMinBias_Y_dir}mergedY3_A.root
hadd -f {MC_InclusiveMinBias_Y_dir}mergedY123_A.root {MC_InclusiveMinBias_Y_dir}mergedY1_A.root {MC_InclusiveMinBias_Y_dir}mergedY2_A.root {MC_InclusiveMinBias_Y_dir}mergedY3_A.root
"""
with open(os.path.join(MCMINBIAS_FOLDER,"merge_final.sh"), "w") as file:
    file.write(template_MC_InclusiveMinBias_merge_final_bash)
