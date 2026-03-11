import sys
import numpy
import os
import argparse
import ase.io
from ase.data import atomic_numbers
import numpy as np
import tqdm
import struct

BLUE = "\033[1;34m"
WHITE = "\033[0m"

def main():
    """Grabs command-line arguments and parses these
    """
    parser = argparse.ArgumentParser(description="A script that processes files and folders.")
    parser.add_argument("-x", "--xdatcar", type=validate_file, required=True, 
                        help="Path to a file.")
    parser.add_argument("-e", "--energies", type=validate_file, required=True, 
                        help="Path to a file.")
    parser.add_argument("-o", "--output", type=str, required=True, 
                        help="Path to an output file.")
    
    # parse command-line arguments
    args = parser.parse_args()

    # generate data files
    xdatcar2pack(args.xdatcar, args.energies, args.output)

def xdatcar2pack(xdatcar_path:str,
                 xdaten_path:str,
                 outpath:str) -> None:
    """Converts POSCAR file to a highly compressed pack file

    Args:
        xdatcar_path (str): path to POSCAR file
    
    Function will automatically overwrite any existing file
    """
    
    if not os.path.exists(xdatcar_path):
        print('Could not parse: %s' % xdatcar_path)
        sys.exit()

    if not os.path.exists(xdaten_path):
        print('Could not parse: %s' % xdaten_path)
        sys.exit()

    energies = np.loadtxt(xdaten_path)

    # get the contents of the XDATCAR
    trajectory = ase.io.read(xdatcar_path, index=":")
    all_data = []
    for i,(energy, atoms) in tqdm.tqdm(enumerate(zip(energies,trajectory)), total=len(energies)):
        data = {}

        # Step 1: Get unique element list (in order of first appearance)
        unique_elements = []
        for element in atoms.get_chemical_symbols():
            if element not in unique_elements:
                unique_elements.append(element)
        element_numbers = np.array([atomic_numbers[element] for element in unique_elements], dtype=np.uint8)

        # Step 2: Generate symbol ID list based on unique_elements
        symbol_ids = [unique_elements.index(element) for element in atoms.get_chemical_symbols()]

        # Step 3: Sort atoms based on symbol_id
        sorted_indices = sorted(range(len(atoms)), key=lambda i: symbol_ids[i])

        # Step 4: Reorder atoms
        sorted_atoms = atoms[sorted_indices]

        # Step 5: Generate new sorted symbol ID list
        sorted_symbol_ids = [unique_elements.index(el) for el in sorted_atoms.get_chemical_symbols()]

        # convert coordinates from Direct to Cartesian:
        latticevectors = atoms.cell
        cartesian = atoms.positions

        data['Energy'] = energy
        data['Periodicity'] = 3
        data['Lattice'] = latticevectors
        data['Atomlist'] = symbol_ids
        data['Chemical Symbols'] = element_numbers
        data['Electronic Convergence'] = [0,1]
        data['Ionic Convergence'] = 0
        data['Coordinates'] = cartesian
        data['Number of Bulk Atoms'] = len(sorted_indices)

        all_data.append(data)

    store_multiple_as_binary(all_data, outpath)

def store_multiple_as_binary(data_list, output_file):
    """
    Stores multiple extracted data entries in a raw binary format with clearly defined data types.
    """
    with open(output_file, 'wb') as bin_file:
        # Writing the number of data entries (Little Endian format)
        bin_file.write(struct.pack('<Q', len(data_list)))  # 64-bit unsigned integer for count
        
        for data in data_list:
            # Writing metadata (Little Endian format)
            bin_file.write(struct.pack('<d', data['Energy']))  # 64-bit double for energy
            bin_file.write(struct.pack('<Q', data['Periodicity']))  # 64-bit unsigned integer for periodicity
            bin_file.write(struct.pack('<Q', data['Number of Bulk Atoms']))  # 64-bit unsigned integer for bulk atom count
            
            # Writing Lattice (Little Endian format)
            for row in data['Lattice']:
                bin_file.write(struct.pack('<3f', *row))  # Three 64-bit doubles per row
            
            # Writing Atomlist as 8-bit unsigned integers (Little Endian format)
            bin_file.write(struct.pack('<Q', len(data["Atomlist"])))
            bin_file.write(struct.pack(f'<{len(data["Atomlist"])}B', *data['Atomlist']))
            
            # Writing Chemical Symbols as atomic numbers (8-bit unsigned integers, Little Endian format)
            bin_file.write(struct.pack('<Q', len(data["Chemical Symbols"])))
            bin_file.write(struct.pack(f'<{len(data["Chemical Symbols"])}B', *data['Chemical Symbols']))
            
            # Writing Coordinates (Little Endian format)
            bin_file.write(struct.pack('<Q', len(data['Coordinates'])))
            for coord in data['Coordinates']:
                bin_file.write(struct.pack('<3f', *coord))  # Three 64-bit doubles per coordinate
    
    print(f"Stored {len(data_list)} data files in {output_file}")

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