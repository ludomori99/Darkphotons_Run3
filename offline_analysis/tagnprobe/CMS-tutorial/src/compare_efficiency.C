#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/efficiency_comparison.C"

//Compare efficiency
void compare_efficiency(string quantity, string MC, string Run3, string MuonId, bool barrelVsEndcap = false)
{
    TFile *file0 = TFile::Open(MC.c_str());
    TFile *file1 = TFile::Open(Run3.c_str());

    if (file0 == NULL || file1 == NULL)
    {
        std::cerr << "ABORTING...\n";
        abort();
    }
    if (barrelVsEndcap)  compare_plot_BarrelVsEndcap(file0, file1, "Efficiency_", quantity, MuonId);
    else compare_plot(file0, file1, "Efficiency", quantity, MuonId);
}


