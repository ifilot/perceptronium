# PyTorch utilities

This folder contains model training/optimization scripts and the `perceptronium` Python package used for neural-network potential fitting.

## Top-level scripts

- `train_ch4.py`  
  Trains a methane model from the high-temperature coefficient dataset (`data/ch4_ht_coeff.npz`) and writes plots/model files into `output/`.
- `train_ch4_accurate.py`  
  Trains on both high- and low-temperature methane datasets for a longer run and stores a higher-accuracy model artifact.
- `opt_ch4.py`  
  Uses a trained methane model (`output/ch4_network.pth`) to optimize CH4 geometry (Nelder–Mead), then writes animation and bond-length diagnostics.
- `opt_h2.py`  
  Reuses the trained model and descriptor calculator for a simple H2 optimization demo, saving an optimization GIF.

## `perceptronium/` package

- `__init__.py`  
  Package exports for dataset/network/fitter/symmetry-calculator utilities.
- `dataset.py`  
  Dataset loader and preprocessing helpers for `.npz` coefficient files (including optional normalization/statistics by atom type and request-file-driven feature grouping).
- `network.py`  
  Neural-network model definitions used for atom-wise or molecule-level energy prediction.
- `fitter.py`  
  Training/evaluation loop wrapper (`MolFitter`) with model persistence and plotting helpers.
- `symmetryfunctioncalculator.py`  
  Pure-Python symmetry-function calculator compatible with request-file parameterization, used for inference-time feature generation.

## Expected inputs/outputs

- Inputs typically come from generated descriptor datasets in `data/*.npz` plus the shared request file in `data/request`.
- Outputs are usually written to `output/` (saved `.pth` model weights, parity/loss plots, and optimization GIFs).
