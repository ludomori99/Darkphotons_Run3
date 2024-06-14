using namespace std;


double compute_mva_sf(double mass) {

    double SF_Jpsi = 0.932; //data/MC
    double SF_Y = 0.987;

    double mass_Jpsi = 3.0943;

    double mass_Y1 = 9.451;
    double mass_Y2 = 10.015;
    double mass_Y3 = 10.345;

    //From splot fits
    double Y2_frac = 0.2173;
    double Y3_frac = 0.1367;
    double Y1_frac = 1 - Y2_frac - Y3_frac;

    double mass_Y = Y1_frac*mass_Y1 + Y2_frac*mass_Y2 + Y3_frac*mass_Y3;

    double m = (SF_Jpsi - SF_Y) / (mass_Jpsi - mass_Y);
    double q = - m*mass_Jpsi + SF_Jpsi;
    return m*mass + q;
}