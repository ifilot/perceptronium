# Python preprocessing utilities (`pysrc`)

This folder contains helper scripts for preparing datasets, converting between file formats, and inspecting trajectories used by Perceptronium.

## Files

- `xdatcar2data.py`  
  Converts an `XDATCAR` trajectory plus matching energy list (`XDATEN_*`) into per-frame `.data` files (`struc_00001.data`, ...), including geometry/metadata blocks expected by the descriptor pipeline.

- `xdatcar2pack.py`  
  Converts an `XDATCAR` + energy file pair directly into a compact binary package containing per-frame geometry and metadata.

- `pos2dat.py`  
  Converts a single POSCAR/XDATCAR-like structure file into one `.data` file (useful for one-off structures and debugging).

- `pack.py`  
  Packs existing `.data` files into a binary dataset format and verifies round-trip integrity by re-reading and checking fields.

- `collect.py`  
  Collects symmetry-function data from a folder of `.data` files and stores a compressed `.npz` dataset (energies, coefficients, element IDs) for PyTorch training workflows.

- `vis.py`  
  Quick ASE-based viewer for an input trajectory file to inspect structures interactively.

## Typical workflow

1. Convert trajectory/energies to `.data` files (`xdatcar2data.py`) or directly to binary (`xdatcar2pack.py`).
2. Run descriptor generation to fill the `Symmetry Functions:` blocks in `.data` files.
3. Aggregate generated `.data` results into an `.npz` training dataset with `collect.py`.
4. Optionally compress geometry-only datasets with `pack.py`.
5. Use `vis.py` for visual sanity checks during preparation.

## Notes

- Most scripts expose CLI arguments via `argparse`; run with `-h` for usage details.
- Ensure trajectory frame count and energy count match when using `xdatcar2data.py` or `xdatcar2pack.py`.
