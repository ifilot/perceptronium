import os
import argparse
import numpy as np
import matplotlib.pyplot as plt
import ase.io
from ase.visualize.plot import plot_atoms
from multiprocessing import Pool, cpu_count, Manager, Value, Lock
from tqdm import tqdm  # Import tqdm for the progress bar

def main():
    parser = argparse.ArgumentParser(description="A script that processes XDATCAR and generates PNG images.")
    parser.add_argument("-x", "--xdatcar", type=validate_file, required=True, 
                        help="Path to the XDATCAR file.")
    parser.add_argument("-o", "--output", type=str, required=True, 
                        help="Path to an output folder (must be a folder).")
    
    # Parse command-line arguments
    args = parser.parse_args()

    # Create output folder if it doesn't exist
    os.makedirs(args.output, exist_ok=True)

    # Generate image frames
    create_frames(args.xdatcar, args.output)

def create_frames(xdatcarpath: str, outpath: str, dpi: int = 200):
    """Reads XDATCAR and generates frames in parallel with a proper progress bar."""
    images = ase.io.read(xdatcarpath, index=":")  # Load all frames
    num_workers = min(cpu_count(), len(images))  # Use available CPU cores but not exceed number of frames

    with Manager() as manager:
        progress = manager.list([0])  # Shared variable to track progress

        # Start the progress bar process
        with tqdm(total=len(images), desc="Generating Images", unit="frame") as pbar:
            with Pool(num_workers) as pool:
                results = [pool.apply_async(save_frame, args=(i, images[i], outpath, dpi, progress)) for i in range(len(images))]

                # Monitor progress
                while sum(progress) < len(images):
                    pbar.n = sum(progress)
                    pbar.refresh()

                # Ensure all processes complete
                [r.get() for r in results]

def save_frame(i, atoms, output_dir, dpi, progress):
    """Generates and saves a single frame."""

    fig, ax = plt.subplots(figsize=(6, 6))
    plot_atoms(atoms, ax=ax, radii=0.5)  # Adjust radii for better visibility

    frame_name = f"frame_{i+1:05d}.png"
    output_path = os.path.join(output_dir, frame_name)

    plt.axis("off")  # Hide axes for cleaner images
    plt.savefig(output_path, dpi=dpi, bbox_inches="tight")
    plt.close(fig)

    # Update shared progress list safely
    progress[0] += 1

def validate_file_or_folder(path):
    if not os.path.exists(path):
        raise argparse.ArgumentTypeError(f"The path '{path}' does not exist.")
    return path

def validate_file(path):
    if not os.path.isfile(path):
        raise argparse.ArgumentTypeError(f"The path '{path}' is not a valid file.")
    return path

def validate_folder(path):
    if not os.path.isdir(path):
        raise argparse.ArgumentTypeError(f"The path '{path}' is not a valid folder.")
    return path

if __name__ == '__main__':
    main()