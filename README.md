# Perceptronium

![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/ifilot/perceptronium?label=version)
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

1. Convert trajectory + energies into per-frame `.data` files.
2. Build the C++ executable.
3. Run descriptor generation with a request file:

   ```bash
   ./build/perceptronium -r data/request -d your_dataset.pkg -o descriptors.npz
   ```

   (`-d` expects the binary package generated from your structures; see
   `pysrc/xdatcar2pack.py` / `pysrc/pack.py`.)
4. Train/evaluate models from the generated `.npz` (PyTorch utilities are in
   `torch/perceptronium`).

## Compilation and dependencies

Perceptronium is a mixed C++ / Python software suite. To work with Perceptronium,
ensure you have the following packages installed.

```bash
sudo apt install python3 python3-ase python3-torch python3-tqdm build-essential cmake
```

The above command might not work if python3-torch is not available via your
package manager. If so, create a virtual Python environment and install Torch
in there. The commands below assume you have GPU.

```bash
python3 -m venv ~/.venv
source ~/.venv/bin/activate
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu118
pip install pyyaml
```

### Compilation of perceptronium

```bash
mkdir build
cd build
cmake ../src
make -j
```
