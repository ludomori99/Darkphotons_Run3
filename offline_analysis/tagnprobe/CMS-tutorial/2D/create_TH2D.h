
TH2D* create_TH2D(const char* name, const char* title, string xquantity, string yquantity, int nbinsx, int nbinsy,
	double* xbins, double* ybins)
{
	TH2D* hist2d = new TH2D(name, title, nbinsx, xbins, nbinsy, ybins);

	//Set x axis title for plot
	if      (xquantity == "Probe_pt" ) hist2d->GetXaxis()->SetTitle("p_{T} [GeV/c]");
	else if (xquantity == "Probe_eta") hist2d->GetXaxis()->SetTitle("|#eta|");
	else if (xquantity == "Mm_dR") hist2d->GetXaxis()->SetTitle("#Delta R");

	//Set y axis title for plot
	if      (yquantity == "Probe_pt" ) hist2d->GetYaxis()->SetTitle("p_{T} [GeV/c]");
	else if (yquantity == "Probe_eta") hist2d->GetYaxis()->SetTitle("|#eta|");
	else if (yquantity == "Mm_dR") hist2d->GetYaxis()->SetTitle("#Delta R");
	
	return hist2d;
}