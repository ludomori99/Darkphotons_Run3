import ROOT
import os,sys
from ROOT import TGraphAsymmErrors
from ROOT import TGraphErrors
from ROOT import TColor
#from ROOT import TGraph
from array import array
from ROOT import *
from operator import truediv
import random
import math
import glob


limit1=array('d')
limiteps2=array('d')
limit190=array('d')
limiteps290=array('d')
limit195up=array('d')
limit195down=array('d')
limit168up=array('d')
limit168down=array('d')
limitObserved=array('d')
mass=array('d')
masserr=array('d')

xsec=1.#pb


num_mass_regions = 4 # 278
growth_factor = 0.005
m = 2
#some counters
t=0
#make the loop

a=1.
print("hi")
for d in range(0,num_mass_regions):
	
	m=m+(m*growth_factor)
	print (m)
	print (d)
	

	if (d%3!=0): continue

	if(d>51 and d<146):
		continue

	file=glob.glob("/data/submit/mori25/dark_photons_ludo/DimuonTrees/limits/16_5/output_expected/higgsCombineasympMassIndex_"+str(d)+".AsymptoticLimits.mH*.root")

	f=ROOT.TFile.Open(file[0])
	tree=f.Get("limit")
	tree.GetEntry(2)
	limit1.append(tree.limit*a)
	
	
	tree=f.Get("limit")
	tree.GetEntry(0)
	limit195up.append(abs(tree.limit*a-limit1[-1]))
	tree=f.Get("limit")
	tree.GetEntry(4)
	limit195down.append(abs(tree.limit*a-limit1[-1]))
	
	
	tree.GetEntry(1)
	limit168up.append(abs(tree.limit*a-limit1[-1]))
	tree=f.Get("limit")
	tree.GetEntry(3)
	limit168down.append(abs(tree.limit*a-limit1[-1]))
	
	# tree.GetEntry(5)
	# limitObserved.append(tree.limit*a)
		
	mass.append(m)
	masserr.append(0.)
	print (m)
	

c1=ROOT.TCanvas("c1","c1",800,700)
#c1.SetGrid()
c1.SetLogy()
#c1.SetLogx()

mg=ROOT.TMultiGraph()
mgeps=ROOT.TMultiGraph()

graph_limit1=ROOT.TGraph(len(mass),mass,limit1)
graph_limit1.SetTitle("graph_limit1")
graph_limit1.SetMarkerSize(1)
graph_limit1.SetMarkerStyle(20)
graph_limit1.SetMarkerColor(kBlack)
graph_limit1.SetLineWidth(2)
graph_limit1.SetLineStyle(7)
graph_limit1.GetYaxis().SetRangeUser(0,100)
graph_limit1.GetXaxis().SetRangeUser(2,8)
graph_limit1.GetYaxis().SetTitle("#sigma(pp#rightarrow A)#times BR(A#rightarrow #mu#mu)[pb]")
graph_limit1.GetYaxis().SetTitleSize(2)
graph_limit1.GetXaxis().SetTitle("Dark Photon Mass [GeV]")

# graph_limit=ROOT.TGraph(len(mass),mass,limitObserved)
#graph_limit.Draw("same")
graph_limit95up=ROOT.TGraphAsymmErrors(len(mass),mass,limit1,masserr,masserr,limit195up,limit195down)
graph_limit95up.SetTitle("graph_limit95up")

graph_limit95up.SetFillColor(ROOT.TColor.GetColor(252,241,15))

graph_limit95down=ROOT.TGraph(len(mass),mass,limit195down)
graph_limit95down.SetTitle("graph_limit95down")
graph_limit95down.SetMarkerSize(1)
graph_limit95down.SetMarkerStyle(23)
graph_limit95down.SetMarkerColor(kYellow)


graph_limit68up=ROOT.TGraphAsymmErrors(len(mass),mass,limit1,masserr,masserr,limit168up,limit168down)
graph_limit68up.SetTitle("graph_limit68up")
graph_limit68up.SetFillColor(kGreen)
graph_limit68up.SetMarkerColor(kGreen)

graph_limit68down=ROOT.TGraph(len(mass),mass,limit168down)
graph_limit68down.SetTitle("graph_limit68down")
graph_limit68down.SetMarkerSize(1)
graph_limit68down.SetMarkerStyle(22)
graph_limit68down.SetMarkerColor(kGreen)

mg.Add(graph_limit95up,"3")
mg.Add(graph_limit68up,"3")
mg.Add(graph_limit1,"pl")
#mg.Add(graph_limit,"pl")


mg.Draw("APC")

mg.GetXaxis().SetRangeUser(2.,8.)
mg.GetYaxis().SetRangeUser(0.01,10)

mg.GetYaxis().SetTitle("#sigma(pp#rightarrow A)#times BR(A#rightarrow #mu#mu)#times Acc. [pb]")
mg.GetYaxis().SetTitleSize(0.055)
mg.GetYaxis().SetLabelSize(0.05)
mg.GetXaxis().SetTitle("Mediator Mass [GeV]")
mg.GetXaxis().SetLabelSize(0.05)
mg.GetXaxis().SetTitleSize(0.055)
mg.GetYaxis().SetTitleOffset(0.85)
mg.GetXaxis().SetTitleOffset(0.85)

c1.Update()
legend=ROOT.TLegend(0.5,0.6,0.8,0.9)
cmsTag=ROOT.TLatex(0.1,0.917,"#scale[1.1]{CMS}")
cmsTag.SetNDC()
cmsTag.SetTextAlign(11)
cmsTag.Draw()
cmsTag2=ROOT.TLatex(0.195,0.917,"#scale[0.825]{#bf{#it{Preliminary}}}")
cmsTag2.SetNDC()
cmsTag2.SetTextAlign(11)
#cmsTag.SetTextFont(61)
cmsTag2.Draw()
cmsTag3=ROOT.TLatex(0.90,0.917,"#scale[0.9]{#bf{34.3 fb^{-1} (13 TeV)}}")
cmsTag3.SetNDC()
cmsTag3.SetTextAlign(31)
#cmsTag.SetTextFont(61)
cmsTag3.Draw()
leg=ROOT.TLegend(0.65, 0.65,0.88, 0.85)  
leg.SetBorderSize( 0 )
leg.SetFillStyle( 1001 )
# leg.SetFillColor(kWhite) 

leg.AddEntry( graph_limit1 , "Expected",  "LP" )
leg.AddEntry( graph_limit68up, "#pm 1#sigma",  "F" ) 
leg.AddEntry( graph_limit95up, "#pm 2#sigma",  "F" ) 
leg.Draw("same")
# c1.SaveAs("limit2018C_cmsdas.root")
# c1.SaveAs("limit2018C_cmsdas.pdf")
c1.SaveAs("/work/submit/mori25/Darkphotons_ludo/offline_analysis/limits/limit_17_5.png")





