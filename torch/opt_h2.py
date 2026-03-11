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

# Define the root directory
ROOT = os.path.dirname(__file__)

def main():
    """
    Main function to optimize h2 molecule geometry using a trained neural network.
    """
    # Define atomic symbols and initial positions for H2
    symbols = ['H', 'H']
    positions = np.array([
        [-1.0, 0.0, 0.0],
        [ 1.0, 0.0, 0.0],
    ])

    # Create an ASE Atoms object (no periodic boundary conditions)
    h2 = Atoms(symbols=symbols, positions=positions)
    h2.center(vacuum = 0.0)

    # Initialize Symmetry Function Calculator
    sfc = SymmetryFunctionCalculator(os.path.join(ROOT, '..', 'data', 'request'),
                                    [atomic_numbers[element] for element in ['C', 'H']])
    
    # Load trained neural network model
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    torch.set_float32_matmul_precision('high')
    mask = torch.tensor(np.ones(len(h2))).to(device)
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
        coeff = sfc.calculate(pos.reshape((len(h2), -1)), h2.get_atomic_numbers())
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
            position_data.append(current_positions.reshape((len(h2), -1)))
            
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

    # store as animated gif
    ase.io.write(os.path.join(ROOT, '..', 'output', 'h2opt.gif'), atoms_list, format='gif', interval=100)

if __name__ == '__main__':
    main()