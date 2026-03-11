# Data directory

This folder contains example input data and descriptor-request settings used by the Perceptronium pipeline.

## Files

- `XDATCAR_ch4_ht`, `XDATCAR_ch4_lt`  
  VASP-style methane trajectories (high-temperature and low-temperature variants).
- `XDATCAR_ethane_ht`, `XDATCAR_ethane_lt`  
  VASP-style ethane trajectories (high-temperature and low-temperature variants).
- `XDATCAR_ethylene_ht`, `XDATCAR_ethylene_lt`  
  VASP-style ethylene trajectories (high-temperature and low-temperature variants).
- `XDATEN_ch4_ht`, `XDATEN_ch4_lt`  
  Per-frame energies corresponding to the methane trajectories.
- `XDATEN_ethane_ht`, `XDATEN_ethane_lt`  
  Per-frame energies corresponding to the ethane trajectories.
- `XDATEN_ethylene_ht`, `XDATEN_ethylene_lt`  
  Per-frame energies corresponding to the ethylene trajectories.
- `request`  
  Symmetry-function request file used to configure descriptor generation (G0/G1/G2/G3/G3f/G4/G5 sections).
- `.gitignore`  
  Ignore rules for generated/intermediate data products that should not be committed.

## Intended use

Typical flow for this folder:

1. Pick one `XDATCAR_*` trajectory and its matching `XDATEN_*` file.
2. Convert trajectory + energies into per-structure `.data` files (Python utilities in `pysrc/`).
3. Run descriptor generation with `request`.
4. Save generated coefficient datasets (for example `.npz`) back into this folder or another data output folder for training scripts under `torch/`.

Keep trajectory frame counts and energy line counts aligned for each pair.
