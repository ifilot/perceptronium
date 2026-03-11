import sys
import numpy
import os
import argparse
import numpy as np
import tqdm
from mendeleev import element
import struct

def main():
    """
    Gathers geometric information from .data files and stores this in a binary
    format; significantly reducing the size of the original data set.
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
    data = package(args.path)
    store_multiple_as_binary(data, args.output)
    data_stored = read_multiple_from_binary(args.output)

    # perform verification
    for struc1,struc2 in zip(data, data_stored):
        np.testing.assert_almost_equal(struc1['Coordinates'], struc2['Coordinates'], decimal=6)
        np.testing.assert_equal(struc1['Atomlist'], struc2['Atomlist'])
        np.testing.assert_equal(struc1['Number of Bulk Atoms'], struc2['Number of Bulk Atoms'])
        np.testing.assert_equal(struc1['Periodicity'], struc2['Periodicity'])
        np.testing.assert_almost_equal(struc1['Lattice'], struc2['Lattice'], decimal=6)
        np.testing.assert_almost_equal(struc1['Energy'], struc2['Energy'], decimal=6)
        np.testing.assert_almost_equal(struc1['Chemical Symbols'], struc2['Chemical Symbols'])
        np.testing.assert_almost_equal(struc1['Number of Bulk Atoms'], struc2['Number of Bulk Atoms'])

    print("All done!")

def atomic_number(symbol):
    """Returns atomic number for a given chemical symbol using mendeleev package."""
    try:
        return element(symbol).atomic_number
    except:
        return 0  # Default to 0 if symbol not found

def parse_data_file(file_path):
    """
    Parses a .data file and extracts all information except the symmetry functions.
    """
    data = {}
    reading_coordinates = False
    coordinates = []
    
    with open(file_path, 'r') as file:
        lines = file.readlines()
        
        for i, line in enumerate(lines):
            line = line.strip()
            
            if line.startswith("Energy:"):
                data['Energy'] = float(lines[i+1].strip())
            elif line.startswith("Periodicity:"):
                data['Periodicity'] = int(lines[i+1].strip())
            elif line.startswith("Lattice:"):
                data['Lattice'] = [list(map(float, lines[i+j+1].split())) for j in range(3)]
            elif line.startswith("Atomlist:"):
                data['Atomlist'] = list(map(int, lines[i+1].split()))
            elif line.startswith("Chemical Symbols:"):
                data['Chemical Symbols'] = [atomic_number(sym) for sym in lines[i+1].split()]
            elif line.startswith("Number of Bulk Atoms:"):
                data['Number of Bulk Atoms'] = int(lines[i+1].strip())
            elif line.startswith("Electronic Convergence:"):
                data['Electronic Convergence'] = list(map(int, lines[i+1].split()))
            elif line.startswith("Ionic Convergence:"):
                data['Ionic Convergence'] = lines[i+1].strip()
            elif line.startswith("Coordinates:"):
                reading_coordinates = True
                continue
            elif line.startswith("Symmetry Functions:"):
                break  # Stop reading before symmetry functions
            
            if reading_coordinates and not line.startswith("Coordinates:") and line:
                coordinates.append(list(map(float, line.split())))
    
    data['Coordinates'] = coordinates
    return data

def package(datapath:str) -> None:
    """
    Package all .data files in a folder and combines them into a single file

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

    # open the output file
    all_data = []
    for i,data_file in enumerate(tqdm.tqdm(data_files)):
        file_path = os.path.join(datapath, data_file)
        data = parse_data_file(file_path)
        all_data.append(data)

    return all_data

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

def read_multiple_from_binary(input_file):
    """
    Reads multiple extracted data entries from a binary file and reconstructs them.
    """
    data_list = []
    with open(input_file, 'rb') as bin_file:
        num_entries = struct.unpack('<Q', bin_file.read(8))[0]  # Read number of data entries
        
        for _ in range(num_entries):
            data = {}
            data['Energy'] = struct.unpack('<d', bin_file.read(8))[0]  # Read 64-bit double for energy
            data['Periodicity'] = struct.unpack('<Q', bin_file.read(8))[0]  # Read 64-bit unsigned int for periodicity
            data['Number of Bulk Atoms'] = struct.unpack('<Q', bin_file.read(8))[0]  # Read 64-bit unsigned int for bulk atom count
            
            # Read Lattice as 32-bit floats
            data['Lattice'] = [list(struct.unpack('<3f', bin_file.read(12))) for _ in range(3)]
            
            # Read Atomlist length and Atomlist as 8-bit unsigned integers
            atomlist_length = struct.unpack('<Q', bin_file.read(8))[0]  
            data['Atomlist'] = list(struct.unpack(f'<{atomlist_length}B', bin_file.read(atomlist_length)))
            
            # Read Chemical Symbols as atomic numbers (8-bit unsigned integers)
            chem_symbols_length = struct.unpack('<Q', bin_file.read(8))[0]
            data['Chemical Symbols'] = list(struct.unpack(f'<{chem_symbols_length}B', bin_file.read(chem_symbols_length)))
            
            # Read Coordinates as 32-bit floats
            num_coordinates = struct.unpack('<Q', bin_file.read(8))[0]
            data['Coordinates'] = [list(struct.unpack('<3f', bin_file.read(12))) for _ in range(num_coordinates)]
            
            data_list.append(data)
    
    print(f"Loaded {len(data_list)} data files from {input_file}")
    return data_list

def validate_folder(path):
    if not os.path.isdir(path):
        raise argparse.ArgumentTypeError(f"The path '{path}' is not a valid folder.")
    return path

if __name__ == '__main__':
    main()