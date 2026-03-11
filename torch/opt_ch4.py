import os
import time
import numpy as np
import ase.io
from ase import Atoms
from ase.visualize import view
from ase.data import atomic_numbers
import torch
from scipy.optimize import minimize
from perceptronium import SymmetryFunctionCalculator
import matplotlib.pyplot as plt

# Define the root directory
ROOT = os.path.dirname(__file__)

def main():
    """
    Main function to optimize methane molecule geometry using a trained neural network.
    """
    # Define atomic symbols and initial positions for methane (CH4)
    symbols = ['C', 'H', 'H', 'H', 'H']
    positions = np.array([
        [5.0, 5.0, 5.0],
        [6.0, 5.0, 5.0],
        [4.0, 5.0, 5.0],
        [5.0, 6.0, 5.0],
        [5.0, 4.0, 5.0]
    ])

    # Create an ASE Atoms object (no periodic boundary conditions)
    methane = Atoms(symbols=symbols, positions=positions)
    methane.center(vacuum = 0.0)

    # Initialize Symmetry Function Calculator
    sfc = SymmetryFunctionCalculator(os.path.join(ROOT, '..', 'data', 'request'),
                                    [atomic_numbers[element] for element in ['C', 'H']])
    
    # Load trained neural network model
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    torch.set_float32_matmul_precision('high')
    mask = torch.tensor(np.ones(len(methane))).to(device)
    model_path = os.path.join(ROOT, '..', 'output', 'ch4_network.pth')
    model = torch.load(model_path, weights_only=False)
    model.to(device).eval()
    model = torch.compile(model)

    # Track iteration history
    iteration_data = []  # Store energy and time per iteration
    position_data = []    # Store molecular positions per iteration
    start_time = time.time()  # Start timer
    bestenergy = float('inf')  # Initialize best energy as a large value
    energy = 0  # Initialize energy value

    def calculate_energy(pos):
        """
        Compute the energy of the system given atomic positions.
        """
        nonlocal energy
        
        # Compute symmetry function coefficients
        coeff = sfc.calculate(pos.reshape((len(methane), -1)), methane.get_atomic_numbers())
        input_tensor = torch.tensor(coeff, dtype=torch.float32).unsqueeze(0).to(device)
        
        # Perform inference with the trained model
        with torch.no_grad():
            energy = model(input_tensor, mask=mask).cpu().detach().numpy()[0, 0]
        return energy

    def callback(current_positions):
        """
        Callback function to track optimization progress.
        """
        nonlocal start_time, bestenergy, energy
        
        # Update best energy if improved
        if energy < bestenergy:
            bestenergy = energy
            position_data.append(current_positions.reshape((len(methane), -1)))
            
            # Compute elapsed time
            elapsed_time = time.time() - start_time
            start_time = time.time()  # Reset timer for next iteration
            
            # Store iteration data
            iteration_data.append((len(iteration_data) + 1, energy, elapsed_time))
            
            # Print iteration progress
            print(f"Iteration {len(iteration_data)}: Energy = {energy:.6f}, Time = {elapsed_time:.4f} sec")

    # Optimize molecular geometry using Nelder-Mead (Simplex Search)
    result = minimize(
        fun=calculate_energy,                       # Energy function
        x0=positions.flatten(),                     # Flattened initial positions
        method='Nelder-Mead',                       # Gradient-free optimization
        callback=callback,                          # Track iterations
        options={'maxiter': 5000, 'fatol': 1e-4}    # Stopping criteria
    )

    # Store optimized atomic configurations, add a unitcell mainly for
    # visualization purposes
    atoms_list = [Atoms(symbols=symbols, positions=pos) for pos in position_data]
    for atoms in atoms_list:
        atoms.set_cell([5.0, 5.0, 5.0])
        atoms.center()

    # show bond lengths
    visualize_bond_lengths(atoms_list)

    # store as animated gif
    ase.io.write(os.path.join(ROOT, '..', 'output', 'ch4opt.gif'), atoms_list, format='gif', interval=100, rotation='-90x')

def visualize_bond_lengths(atoms_list):
    """
    Visualizes C-H bond lengths for each Atoms object in the trajectory.
    Returns a dictionary with lists of bond lengths over time.
    """
    bond_data = {f"H{i+1}": [] for i in range(4)}  # Store bond lengths for each H

    for atoms in atoms_list:
        c_index = 0  # Assuming the first atom (index 0) is Carbon
        h_indices = [1, 2, 3, 4]  # Hydrogen indices

        # Compute bond lengths for this step
        bond_lengths = [atoms.get_distance(c_index, h) for h in h_indices]

        # Store in respective lists
        for i, length in enumerate(bond_lengths):
            bond_data[f"H{i+1}"].append(length)

    # Generate time steps (assuming frames are indexed sequentially)
    time_steps = list(range(len(atoms_list)))

    # Plot the evolution of each C-H bond length over time
    plt.figure(figsize=(8, 5))
    for h_label, bond_lengths in bond_data.items():
        plt.plot(time_steps, bond_lengths, label=h_label, marker='o', alpha=0.5)

    plt.xlabel("Time Step")
    plt.ylabel("C-H Bond Length (Å)")
    plt.title("Evolution of C-H Bond Lengths in Methane")
    plt.legend()
    plt.grid(True)
    plt.savefig('ch4bonds.png')

if __name__ == '__main__':
    main()