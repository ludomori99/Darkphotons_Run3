import uproot
import pandas as pd

# Open the ROOT file and get the tree
file = uproot.open('/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Psi2/merged_A.root')
tree = file['tree']

# Convert the tree to a pandas DataFrame
df = tree.arrays(library='pd')

# Rename the branch in the DataFrame
df.rename(columns={'Mm_mass': 'Mm_mass_Psi2'}, inplace=True)

# Write the DataFrame back to a new ROOT file
with uproot.recreate('/data/submit/mori25/dark_photons_ludo/DimuonTrees/offline/Psi2/merged_A_var.root') as f:
    f.mktree("tree",{name: df[name].dtype for name in df.columns})
    f['tree'].extend(df.to_dict('list'))