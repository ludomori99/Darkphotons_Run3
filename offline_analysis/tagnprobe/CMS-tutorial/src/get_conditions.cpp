string* get_conditions(int bin_n, vector<double> bins, string quantity,bool isBarrel, bool isEndcap)
{
    string barrelOrEndcap = "";
    if (isBarrel) barrelOrEndcap = "isBarrelMuon==1&&";
    else if (isEndcap) barrelOrEndcap = "isBarrelMuon==0&&";
    string* conditions = new string[bin_n];
    for (int i = 0; i < bin_n; i++)
    {
        conditions[i] = barrelOrEndcap + quantity + ">" + to_string(bins[i]).substr(0,4) + "&&" + quantity + "<" + to_string(bins[i+1]).substr(0,4);
    }
    cout<<"\nconditions successful\n";
    return conditions;
}
