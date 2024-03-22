string* get_conditions(int bin_n, vector<double> bins, string quantity)
{
    string* conditions = new string[bin_n];
    for (int i = 0; i < bin_n; i++)
    {
        conditions[i] = quantity + ">" + to_string(bins[i]).substr(0,4) + "&&" + quantity + "<" + to_string(bins[i+1]).substr(0,4);
    }
    return conditions;
}
