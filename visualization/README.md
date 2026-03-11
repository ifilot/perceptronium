# Visualization tools

This folder contains scripts and assets for turning descriptors/trajectories into figures and videos.

## Files

- `trajectory.py`  
  CLI tool that reads an `XDATCAR` trajectory and renders per-frame structure images (`frame_00001.png`, ...), in parallel.
- `coeff.py`  
  Renders per-frame heatmap-like coefficient panels from a coefficient dataset (`.npz`) into image frames (for example under `movie/coefficients/`).
- `combine.py`  
  Composites coefficient frames and structure frames onto a single 1920x1080 canvas for final movie assembly.
- `movie.sh`  
  `ffmpeg` command wrapper that converts combined frame images into an MP4 video.
- `networkpicture.py`  
  Generates a static diagram of a neural-network architecture using `networkx` and `matplotlib`.
- `tahoma.ttf`  
  Font resource used by `coeff.py` for block labels.

## Typical workflow

1. Generate structural frames with `trajectory.py`.
2. Generate descriptor-panel frames with `coeff.py`.
3. Merge both frame streams using `combine.py`.
4. Encode the final movie with `movie.sh`.

Adjust hard-coded paths/frame counts in these scripts as needed for your dataset size and output layout.
