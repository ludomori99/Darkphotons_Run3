#include "/work/submit/mori25/Darkphotons_ludo/offline_analysis/utils/plot_macros/efficiency_comparison.C"

//Compare efficiency
void compare_efficiency(string quantity, const char* MC, const char* Run3, string MuonId, bool barrelVsEndcap = false)
{
    cout<<MC;
    TFile *file0 = TFile::Open(MC);
    cout<<Run3;
    TFile *file1 = TFile::Open(Run3);

    if (file0 == NULL || file1 == NULL)
    {
        std::cerr << "ABORTING...\n";
        abort();
    }
    if (barrelVsEndcap)  compare_plot_BarrelVsEndcap(file0, file1, "Efficiency_", quantity, MuonId);
    else compare_plot(file0, file1, "Efficiency", quantity, MuonId);
}


