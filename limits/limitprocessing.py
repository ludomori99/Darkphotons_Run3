import os
import subprocess

m=1
for d in range(0,470):
	m=m+(m*0.01)

	#if d<90 or d>112:
	#	continue
	
	print('now at mass', m)
	os.system("combine -M AsymptoticLimits /data/submit/mori25/dark_photons_ludo/DimuonTrees/cards/simple_model/dpCard_2018CIterV0_"+str(d)+".txt -m "+str(m)+" -n asympMassIndex_"+str(d)+" --run blind")