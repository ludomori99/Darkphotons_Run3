import os
import subprocess
path = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/cards/dCB_V_multibkg/dpCard_2022-2023full"
m=2.
factor = 0.01
N = 139
for d in range(0,N):
	m+=(m*factor)

	if (d>26 and d<74) :
		continue
	
	print('now at mass', m)
	# os.system(f"combine -M AsymptoticLimits {path+str(d)}.txt -m {str(m)} -n asympMassIndex_{str(d)} --run blind")
	# os.system(f"combine -M AsymptoticLimits {path+str(d)}.txt -m {str(m)} -n asympMassIndex_{str(d)} --run expected")
	# os.system(f"combine -M AsymptoticLimits {path}_{str(d)}.txt -m {str(m)} -n asympMassIndex_{str(d)} --cminDefaultMinimizerStrategy 0 --rAbsAcc=0.0001 --rRelAcc=0.0001 --setParameters pdf_index=0 --run blind") # --rMin -1 --rMax 1 +
	os.system(f"combine -M AsymptoticLimits {path}_{str(d)}.txt -m {str(m)} -n asympMassIndex_CLs90_{str(d)} --cl 0.9 --cminDefaultMinimizerStrategy 0 --rAbsAcc=0.0001 --rRelAcc=0.0001 --setParameters pdf_index=0 --run blind") # --rMin -1 --rMax 1 +
	