import sys
import numpy
import os
import argparse
import ase.io
from ase.data import atomic_numbers
import numpy as np
import tqdm

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
    parser.add_argument("-o", "--output", type=validate_folder, required=True, 
                        help="Path to an output folder (must be a folder).")
    parser.add_argument("-n", "--offset", type=int, required=False, 
                        help="Enumeration offset", default=0)
    parser.add_argument("-f", "--force", action="store_true", required=False, 
                        help="Whether to force overwriting", default=False)
    
    # parse command-line arguments
    args = parser.parse_args()

    # generate data files
    xdatcar2data(args.xdatcar, args.energies, args.output, args.offset, args.force)

def xdatcar2data(xdatcar_path:str,
                 xdaten_path:str,
                 datapath:str,
                 offset:int=0,
                 force:bool=False) -> None:
    """Converts POSCAR file to .data file

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
    for i,(energy, atoms) in tqdm.tqdm(enumerate(zip(energies,trajectory)), total=len(energies)):

        # Step 1: Get unique element list (in order of first appearance)
        unique_elements = []
        for element in atoms.get_chemical_symbols():
            if element not in unique_elements:
                unique_elements.append(element)

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

        # create DATA file
        filename = os.path.join(datapath, 'struc_%05i.data' % (i + offset + 1))

        if not force:
            if os.path.exists(filename):
                raise Exception("Filename %s exists." % filename)

        generate_data_file(filename, energy, [0,1], 'Yes', sorted_symbol_ids, unique_elements, latticevectors, cartesian)

def generate_data_file(filepath:str, 
                       energy:float, 
                       iterations_electronic:list[int], 
                       converged_ionic:str, 
                       atomlist:list[int], 
                       chemicalSymbols:list[str], 
                       latticevectors:numpy.ndarray, 
                       coordinates:numpy.ndarray,
                       periodicity:int=3) :
    """Generate a data file from geometry information

    Args:
        filepath (str): path to write .data file
        energy (float): energy of the system (if known)
        iterations_electronic (list[int]): number of electronic iterations
        converged_ionic (str): whether system is converged
        atomlist (list[int]): list of atomic symbols
        chemicalSymbols (list[str]): atomic symbol dictionary
        latticevectors (numpy.ndarray): unitcell matrix
        coordinates (numpy.ndarray): cartesian coordinates
        periodicity (int, optional): periodicity type. Defaults to 3.
    """
    # catch empty input
    if len(filepath) == 0:
        print('DATA: No destination filename supplied for new DATA file.')
        exit(-1)

    # DATA files must have the .data extension
    if filepath[-5:] != '.data':
        filepath = filepath + '.data'

    # create container
    lines = []

    lines.append('# Data file containing input for a Behler-Parrinello Neural Network\n')
    lines.append('\n')

    lines.append('Energy: eV\n')
    line = '    {0:.9f}\n'
    lines.append(line.format(energy))
    lines.append('\n')

    lines.append('Lattice:\n')
    for vector in range(0,3):
        line = '    {0:15.9f}    {1:15.9f}    {2:15.9f}\n'
        lines.append(line.format(latticevectors[vector][0],latticevectors[vector][1],latticevectors[vector][2]))
    #endfor
    lines.append('\n')

    lines.append('Periodicity:\n')
    line = '    '+str(periodicity) + '\n'
    lines.append(line)
    lines.append('\n')

    lines.append('Atomlist:\n')
    line = '    '
    for atom in range(0,len(atomlist)):
        line = line + str(atomlist[atom]) + ' '
    #endfor
    line = line[:-1] + '\n'
    lines.append(line)
    lines.append('\n')

    lines.append('Chemical Symbols:\n')
    line = '    '
    for symbol in range(0,len(chemicalSymbols)):
        line = line + chemicalSymbols[symbol] + ' '
    #endfor
    line = line[:-1] + '\n'
    lines.append(line)
    lines.append('\n')

    lines.append('Number of Bulk Atoms:\n')
    line = '    ' + str(len(atomlist)) + '\n'
    lines.append(line)
    lines.append('\n')

    lines.append('Electronic Convergence:\n')
    line = '    ' + str(iterations_electronic[0]) + ' ' + str(iterations_electronic[1]) + '\n'
    lines.append(line)
    lines.append('\n')

    lines.append('Ionic Convergence:\n')
    if converged_ionic == 1:
        lines.append('    Yes\n')
    else:
        lines.append('    No\n')
    #endif
    lines.append('\n')

    lines.append('Coordinates: Cartesian\n')
    for atom in range(0,numpy.shape(coordinates)[0]):
        line = '    {0:15.9f}    {1:15.9f}    {2:15.9f}\n'
        lines.append(line.format(coordinates[atom][0],coordinates[atom][1],coordinates[atom][2]))
    #endfor
    lines.append('\n')

    # * The "Symmetry Functions" block will consist of sub-blocks for each
    #   symmetry type. 
    # * The number of G1 functions is equal to the number of lines until we
    #   encounter a \n marking that the next line will be G2. 
    # * We can check if the Symmetry Functions have been calculated by checking
    #   if the G0 tag is present in this block. 
    # * When recalculating these functions, we read the entire file, filter out
    #   the current Symmetry Functions block, then we re-write the symmetry 
    #   block and append EOF marker.
    lines.append('Symmetry Functions:\n')
    lines.append('\n')

    # write above to DATA file
    with open(filepath, 'w') as f:
        f.writelines(lines)

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