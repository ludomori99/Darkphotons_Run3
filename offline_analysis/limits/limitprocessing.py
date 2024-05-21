import os
import subprocess
path = "/data/submit/mori25/dark_photons_ludo/DimuonTrees/cards/dCB_V_multibkg/dpCard_2022-2023_"
m=2.
factor = 0.005
N = 4 # 278
for d in range(0,N):
	m+=(m*factor)

	if (d%3!=0): continue

	if (d>191 and d<286) :
		continue
	
	print('now at mass', m)
	# os.system(f"combine -M AsymptoticLimits {path+str(d)}.txt -m {str(m)} -n asympMassIndex_{str(d)} --run blind")
	# os.system(f"combine -M AsymptoticLimits {path+str(d)}.txt -m {str(m)} -n asympMassIndex_{str(d)} --run expected")
	os.system(f"combine -M AsymptoticLimits {path+str(d)}.txt -m {str(m)} -n asympMassIndex_{str(d)} --cminDefaultMinimizerStrategy 0 --rAbsAcc=0.0001 --rRelAcc=0.0001 --setParameters pdf_index_2022=0  --rMin -1 --rMax 1 --run blind")