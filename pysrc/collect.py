import sys
import numpy
import os
import argparse
import numpy as np
import tqdm

def main():
    """
    Collects all .data files in a folder and stores them in a compressed
    .npz file for usage in NNFIT routines
    """
    parser = argparse.ArgumentParser(description="A script that processes files and folders.")
    parser.add_argument("-p", "--path", type=validate_folder, required=True, 
                        help="Path to an output folder (must be a folder).")
    parser.add_argument("-o", "--output", required=True, 
                        help="Path to an output file.")
    
    # parse command-line arguments
    args = parser.parse_args()

    # generate data files
    print('Reading folder: %s (this can take a while if there are many files in the folder...)' % args.path)
    res = collect(args.path)
    print('Writing to file: %s' % args.output)
    np.savez_compressed(args.output, energies=res[0], coefficients=res[1], elements=res[2])

def collect(datapath:str) -> None:
    """
    Collects all .data files in a folder and combines them into a single file

    Args:
        datapath (str): path to a folder containing .data files

    Function will automatically overwrite any existing file
    """
    
    if not os.path.exists(datapath):
        print('Could not parse: %s' % datapath)
        sys.exit()

    # get all files in the folder
    files = [f for f in os.listdir(datapath) if os.path.isfile(os.path.join(datapath, f))]

    # filter out .data files
    data_files = [f for f in files if f.endswith('.data')]

    coefficients = []
    energies = []

    # open the output file
    elements = None
    for i,data_file in enumerate(tqdm.tqdm(data_files)):
        energy, coeff = parse_file(os.path.join(datapath, data_file))
        if i == 0:
            elements = grab_elements(os.path.join(datapath, data_file))
            print(elements)
        energies.append(energy)
        coefficients.append(coeff)

    return np.array(energies, dtype=np.float32), \
           np.array(coefficients, dtype=np.float32), \
           elements

def parse_file(data_file:str):
    # Read file and extract data
    with open(data_file, "r") as file:
        lines = file.readlines()

    # set containers
    energy = None
    symmetry_functions = []

    # Extract energy value
    for i, line in enumerate(lines):
        if line.strip().startswith("Energy:"):
            energy = float(lines[i + 1].strip())  # The next line contains the energy value
            break

    # Extract symmetry function values
    symmetry_start = None
    for i, line in enumerate(lines):
        if line.strip().startswith("Symmetry Functions:"):
            symmetry_start = i + 1
            break

    if symmetry_start is not None:
        for line in lines[symmetry_start:]:
            if line.strip() == "":
                break
            # Convert each line into a list of floats
            row = list(map(float, line.strip().split()))
            symmetry_functions.append(row)

    # Convert symmetry function values to a NumPy array
    symmetry_array = np.array(symmetry_functions)

    return energy, symmetry_array

def grab_elements(path:str):
    # Read file and extract data
    with open(path, "r") as file:
        lines = file.readlines()

    # set containers
    energy = None
    symmetry_functions = []

    # Extract element indices
    for i, line in enumerate(lines):
        if line.strip().startswith("Atomlist:"):
            elements = [int(e) for e in lines[i+1].strip().split()]
            break

    return elements

def validate_folder(path):
    if not os.path.isdir(path):
        raise argparse.ArgumentTypeError(f"The path '{path}' is not a valid folder.")
    return path

if __name__ == '__main__':
    main()