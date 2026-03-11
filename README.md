# Perceptronium

![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/ifilot/perceptronium?label=version)
[![Zenodo DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.18966047.svg)](https://doi.org/10.5281/zenodo.18966047)
[![Unit Tests](https://github.com/ifilot/perceptronium/actions/workflows/unit-tests.yml/badge.svg)](https://github.com/ifilot/perceptronium/actions/workflows/unit-tests.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Minimalistic stand-alone package for generating Behler–Parrinello
symmetry-function descriptors for atomistic machine-learning potentials based on
neural networks.

## File formats

### Data files

A `.data` file represents one structure/frame. It stores geometry, metadata,
and (optionally) the computed symmetry-function matrix.

Data files are created with `pysrc/xdatcar2data.py` from:

- an `XDATCAR` trajectory (`-x/--xdatcar`)
- a plain-text energy file (`-e/--energies`, one value per frame)
- an output folder (`-o/--output`)

Example:

```bash
python3 pysrc/xdatcar2data.py \
  --xdatcar data/XDATCAR_ch4_lt \
  --energies data/XDATEN_ch4_lt \
  --output datafiles
```

By default this writes files named like `struc_00001.data`,
`struc_00002.data`, ... with an initially empty `Symmetry Functions:` block.
The C++ executable later fills that block according to the request file.

The generated `.data` structure contains these sections:

- `Energy: eV` — scalar energy for this frame
- `Lattice:` — 3x3 lattice vectors
- `Periodicity:` — periodicity flag (currently written as `3`)
- `Atomlist:` — integer species indices per atom
- `Chemical Symbols:` — element symbols in index order (e.g. `C H`)
- `Number of Bulk Atoms:` — number of atoms in this frame
- `Electronic Convergence:` / `Ionic Convergence:` — metadata placeholders
- `Coordinates: Cartesian` — Cartesian coordinates for each atom
- `Symmetry Functions:` — populated after descriptor generation

Notes when using your own systems:

- Keep the number of energy entries aligned with the number of trajectory
  frames.
- Keep atom count and ordering consistent across the trajectory.
- Multi-element systems are supported; element IDs are built dynamically from
  symbols found in each frame.

### Request files

The request file specifies which symmetry functions are calculated and with
which hyperparameters. Each block header is followed by one or more parameter
rows:

- `G0:` beta
- `G1:` cut-off radius
- `G2:` `eta Rs`
- `G3:` `kappa`
- `G3f:` `kappa`
- `G4:` `eta zeta lambda`
- `G5:` `eta zeta lambda`

See `data/request` for an example.

## Typical workflow

1. Extract one energy value per frame (e.g. from `OUTCAR`) into `XDATEN`:

   ```bash
   grep "y  w" OUTCAR | awk '{print $7}' > XDATEN
   ```

2. Convert `XDATCAR + XDATEN` into a binary package (`.pkg`):

   ```bash
   python3 pysrc/xdatcar2pack.py \
     -x /path/to/XDATCAR \
     -e /path/to/XDATEN \
     -o /path/to/my_system.pkg
   ```

3. Copy and edit a request file:

   ```bash
   cp data/request data/my_request
   ```

4. Generate descriptors with the C++ executable:

   ```bash
   ./build/bpsfp \
     -d /path/to/my_system.pkg \
     -r data/my_request \
     -o /path/to/my_system_coeff.npz
   ```

5. Train/evaluate models from the generated `.npz` (PyTorch utilities are in
   `torch/perceptronium`).

## Compilation and dependencies

Perceptronium is a mixed C++ / Python software suite. The documented setup is
to install a minimal system toolchain and use a dedicated Python virtual
environment.

```bash
sudo apt install python3 python3-dev python3-venv build-essential cmake
```

Then create and activate an environment:

```bash
python3 -m venv ~/.venv-perceptronium
source ~/.venv-perceptronium/bin/activate
```

Install Python dependencies in that environment:

```bash
pip install torch torchvision torchaudio pyyaml matplotlib ase tqdm
```

### Compilation of perceptronium

```bash
cmake -S src -B build
cmake --build build -j
```

This produces the descriptor executable at `./build/bpsfp`.
