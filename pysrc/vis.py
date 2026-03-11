import ase.io
from ase.visualize import view
import argparse
import os

def validate_file(path):
    if not os.path.isfile(path):
        raise argparse.ArgumentTypeError(f"The file '{path}' does not exist.")
    return path

def main():
    parser = argparse.ArgumentParser(description="Visualize the contents of an XDATCAR file using ASE.")
    parser.add_argument("-i", "--input", type=validate_file, required=True, 
                        help="Path to the XDATCAR file.")
    args = parser.parse_args()
    
    # Read the XDATCAR file as a trajectory
    trajectory = ase.io.read(args.input, index=':')
    
    if not trajectory:
        print("No structures found in XDATCAR.")
        return
    
    print(f"Loaded {len(trajectory)} structures from {args.input}")
    
    # Visualize the trajectory
    view(trajectory)

if __name__ == "__main__":
    main()