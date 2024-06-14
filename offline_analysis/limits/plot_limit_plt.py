import numpy as np
import matplotlib.pyplot as plt
import mplhep as hep
import uproot as up

# Initialize arrays
limit1 = []
limit195up = []
limit195down = []
limit168up = []
limit168down = []
mass = []
masserr = []

xsec = 1. # pb

num_mass_regions = 139
growth_factor = 0.01
m = 2

# Generate the data
for d in range(num_mass_regions):
    m += m * growth_factor
    if (26 < d < 74):
        continue
    
    f = up.concatenate(f"/data/submit/mori25/dark_photons_ludo/DimuonTrees/limits/full_no_nuisances/output_expected/higgsCombineasympMassIndex_{str(d)}.AsymptoticLimits.mH*.root:limit")
    # for k in f: print(k,f[k])
    exp = f['limit']
    print(exp, f['mh'][0])
    limit1.append(exp[2])
    limit195up.append(exp[0])  # Dummy uncertainties
    limit195down.append(exp[4])
    limit168up.append(exp[1])
    limit168down.append(exp[3])
    
    mass.append(m)
    # masserr.append(0.)

# Convert lists to numpy arrays for plotting
mass = np.array(mass)
limit1 = np.array(limit1)
limit195up = np.array(limit195up)
limit195down = np.array(limit195down)
limit168up = np.array(limit168up)
limit168down = np.array(limit168down)

q = np.argmin(np.abs(mass - 3))

print(mass[q],mass[q+1])

def plot_mod_ind():
    hep.style.use("CMS")
    fig, ax = plt.subplots(figsize=(9,7))
    hep.cms.label("Preliminary",data=True,lumi=62.4,com=13.6)
    ax.set_xscale('log')
    ax.set_yscale('log')

    plt.fill_between(mass[:q], limit195down[:q], limit195up[:q], color='yellow', alpha=0.8, label='±2σ')
    plt.fill_between(mass[:q], limit168down[:q],  limit168up[:q], color='green', alpha=0.8, label='±1σ')
    plt.plot(mass[:q], limit1[:q], color='black', linestyle='dashed', linewidth=2, label='Expected')


    plt.fill_between(mass[q+1:], limit195down[q+1:], limit195up[q+1:], color='yellow', alpha=0.8)
    plt.fill_between(mass[q+1:], limit168down[q+1:],  limit168up[q+1:], color='green', alpha=0.8)
    plt.plot(mass[q+1:], limit1[q+1:], color='black', linestyle='dashed', linewidth=2)

    plt.xlabel("Dimuon Mass [GeV]")
    plt.ylabel(r"$\sigma(pp \rightarrow X) \times BR(X \rightarrow \mu\mu) \times A [pb]$")
    plt.legend()
    # Set axis limits
    plt.xlim(2, 8)
    # ax.ticklabel_format(style='plain')
    import matplotlib.ticker as ticker
    ticks = [2, 3, 4, 5, 6, 7, 8]
    ax.set_xticks(ticks)
    plt.gca().xaxis.set_major_formatter(ticker.ScalarFormatter())
    plt.gca().xaxis.set_minor_formatter(ticker.ScalarFormatter())
    plt.ylim(0.01, 10)

    # Save the plot
    plt.savefig("/work/submit/mori25/Darkphotons_ludo/offline_analysis/limits/limit_full_nonuisance_C.png")

    # Show the plot
    plt.show()


def plot_dp():
    hep.style.use("CMS")
    fig, ax = plt.subplots(figsize=(9,7))
    hep.cms.label("Preliminary",data=True,lumi=62.4,com=13.6)
    ax.set_xscale('log')
    ax.set_yscale('log')

    plt.fill_between(mass, limit195down, limit195up, color='yellow', alpha=0.5, label='±2σ')
    plt.fill_between(mass, limit168down,  limit168up, color='green', alpha=0.5, label='±1σ')

    plt.plot(mass, limit1, color='black', linestyle='dashed', linewidth=2, label='Expected')
    plt.xlabel("$m__{\mu\mu}$ [GeV]")
    plt.ylabel(r"$\sigma(pp \rightarrow X) \times BR(X \rightarrow \mu\mu) \times Acc. \ [pb]$")
    plt.legend()
    # Set axis limits
    plt.xlim(2, 8)
    # ax.ticklabel_format(style='plain')
    import matplotlib.ticker as ticker
    ticks = [2, 3, 4, 5, 6, 7, 8]
    ax.set_xticks(ticks)
    plt.gca().xaxis.set_major_formatter(ticker.ScalarFormatter())
    plt.gca().xaxis.set_minor_formatter(ticker.ScalarFormatter())
    plt.ylim(0.01, 10)

    # Save the plot
    plt.savefig("/work/submit/mori25/Darkphotons_ludo/offline_analysis/limits/limit_full_nonuisance.png")

    # Show the plot
    plt.show()



if __name__ == "__main__":
    plot_mod_ind()