"""Symmetry-function feature generation helpers used by perceptronium."""

import numpy as np
from scipy.spatial.distance import pdist, squareform
import re

class SymmetryFunctionCalculator:
    """
    Class to compute Behler-Parrinello Symmetry Functions (BPSFs) for a given atomic structure.
    """

    def __init__(self, requestfile, networkelements):
        """
        Initializes the symmetry function calculator with given parameters.
        """
        self.parse_request_file(requestfile)
        self.networkelements = networkelements

    def calculate(self, atom_positions, atomic_numbers):
        """Compute per-atom descriptors for one structure."""
        self.atom_positions = np.array(atom_positions)
        self.atomic_numbers = atomic_numbers
        self.num_atoms = len(self.atom_positions)
        self.distances = squareform(pdist(self.atom_positions))  # Compute pairwise distance matrix

        symbol_ids = [self.networkelements.index(element) for element in self.atomic_numbers]

        coefficients = self.calculate_symmetry_functions()
        return np.hstack((np.array(symbol_ids).reshape(len(atom_positions),1), coefficients))

    def calculate_symmetry_functions(self):
        """
        Computes the symmetry functions for all atoms in the system, using parameters 
        parsed from the request file.

        :return: (N, num_symmetry_functions) NumPy array containing symmetry function values.
        """
        if self.Rc_value is None:
            raise ValueError("No Rc value has been set. Ensure that parse_request_file() was called first.")

        symmetry_functions = []

        for i in range(self.num_atoms):
            sf_vector = []

            # One-body symmetry function (G0)
            for beta in self.G0_values:
                sf_vector.append(beta * self.atomic_numbers[i] / 118.0)  # Simply store beta values for each atom

            # Compute two-body interactions (G1, G2, G3, G3f)
            Rc = self.Rc_value  # Use the single cutoff radius

            # G1 - Radial cutoff symmetry function
            mask = np.arange(self.num_atoms) != i  # Mask to exclude self (i != j)
            sf_vector.append(np.sum(self.compute_g1(self.distances[i][mask], Rc)))

            # G2 - Gaussian radial symmetry function
            for eta, Rs in self.G2_params:
                sf_vector.append(np.sum(self.compute_g2(self.distances[i][mask], Rc, eta, Rs)))

            # Compute three-body interactions (G4, G5) per parameter set
            for eta, zeta, lambda_ in self.G5_params:
                G5_value = 0.0

                for j in range(self.num_atoms):
                    if i == j:
                        continue
                    for k in range(self.num_atoms):
                        if i == k or j == k:
                            continue

                        G5_value += self.compute_g5(i, j, k, Rc, eta, zeta, lambda_)

                sf_vector.append(G5_value)

            symmetry_functions.append(sf_vector)

        return np.array(symmetry_functions)

    def cutoff_function(self, distance, Rc):
        """ Behler-Parrinello cutoff function: smoothly decays to zero at Rc. """
        if distance >= Rc:
            return 0.0
        return 0.5 * (np.cos(np.pi * distance / Rc) + 1)

    def compute_g1(self, distances, Rc):
        """ G1 - Radial cutoff symmetry function. """
        return np.array([self.cutoff_function(d, Rc) for d in distances])

    def compute_g2(self, distances, Rc, eta, Rs):
        """ G2 - Gaussian radial symmetry function. """
        return np.exp(-eta * (distances - Rs) ** 2) * np.array([self.cutoff_function(d, Rc) for d in distances])

    def compute_g3(self, distances, Rc, kappa):
        """ G3 - Fourier-based radial symmetry function. """
        return np.cos(kappa * distances) * np.array([self.cutoff_function(d, Rc) for d in distances])

    def compute_g3f(self, distances, Rc, kappa):
        """ G3f - Absolute value of Fourier function (modified G3). """
        return np.abs(np.cos(kappa * distances)) * np.array([self.cutoff_function(d, Rc) for d in distances])

    def compute_g4(self, i, j, k, Rc, eta, zeta, lambda_):
        """ G4 - Angular symmetry function involving three-body interactions. """
        rij = np.linalg.norm(self.atom_positions[j] - self.atom_positions[i])
        rik = np.linalg.norm(self.atom_positions[k] - self.atom_positions[i])
        rjk = np.linalg.norm(self.atom_positions[k] - self.atom_positions[j])

        if rij >= Rc or rik >= Rc or rjk >= Rc:
            return 0.0  # Outside cutoff range

        cos_theta = np.dot(self.atom_positions[j] - self.atom_positions[i], self.atom_positions[k] - self.atom_positions[i]) / (rij * rik)
        cutoff = self.cutoff_function(rij, Rc) * self.cutoff_function(rik, Rc) * self.cutoff_function(rjk, Rc)

        return (2 ** (-zeta)) * ((1 + lambda_ * cos_theta) ** zeta) * np.exp(-eta * (rij**2 + rik**2 + rjk**2)) * cutoff

    def compute_g5(self, i, j, k, Rc, eta, zeta, lambda_):
        """ G5 - Broad angular symmetry function. """
        rij = np.linalg.norm(self.atom_positions[j] - self.atom_positions[i])
        rik = np.linalg.norm(self.atom_positions[k] - self.atom_positions[i])

        if rij >= Rc or rik >= Rc:
            return 0.0  # Outside cutoff range

        cos_theta = np.dot(self.atom_positions[j] - self.atom_positions[i], self.atom_positions[k] - self.atom_positions[i]) / (rij * rik)
        cutoff = self.cutoff_function(rij, Rc) * self.cutoff_function(rik, Rc)

        return (2 ** (-zeta)) * ((1 + lambda_ * cos_theta) ** zeta) * np.exp(-eta * (rij**2 + rik**2)) * cutoff
    
    def parse_request_file(self, filename):
        """
        Parses a request file containing symmetry function parameters and stores them in class attributes.

        :param filename: Path to the request file.
        """
        # Initialize storage variables
        self.G0_values = []
        self.Rc_value = None  # Only one cutoff value allowed
        self.G2_params = []   # List of (eta, Rs) pairs
        self.G5_params = []   # List of (eta, zeta, lambda) triples

        with open(filename, 'r') as file:
            lines = file.readlines()

        section = None  # Track which symmetry function section we're in
        for line in lines:
            line = line.strip()

            # Skip empty lines and comments
            if not line or line.startswith("#"):
                continue

            # Detect new section (e.g., "G0:", "G1:", "G2:", "G5:")
            if re.match(r"^G[0-9]:", line):
                section = line[:-1]  # Store section name without colon
                continue

            # Parse values based on the section
            values = list(map(float, line.split()))
            
            if section == "G0":
                self.G0_values.append(values[0])  # Single float per line
            
            elif section == "G1":
                if self.Rc_value is None:
                    self.Rc_value = values[0]  # Store the single cutoff value
                else:
                    print(f"Warning: Multiple Rc values found. Using the first value: {self.Rc_value}")

            elif section == "G2":
                if len(values) == 2:
                    self.G2_params.append((values[0], values[1]))  # (eta, Rs)
                else:
                    print(f"Warning: Invalid G2 line '{line}', expected format: eta Rs")

            elif section == "G5":
                if len(values) == 3:
                    self.G5_params.append((values[0], values[1], values[2]))  # (eta, zeta, lambda)
                else:
                    print(f"Warning: Invalid G5 line '{line}', expected format: eta zeta lambda")

        # Ensure Rc is set
        if self.Rc_value is None:
            raise ValueError("No Rc value found in the request file (G1 section missing).")

    def print_parameters(self):
        """ Prints the parsed symmetry function parameters. """
        print("Number of parameters", len(self.G0_values) + 1 + len(self.G2_params) + len(self.G5_params))
        print("G0 values:", self.G0_values)
        print("G1 values:", self.Rc_value)
        print("G2 values (eta, Rs):", self.G2_params)
        print("G5 values (eta, zeta, lambda):", self.G5_params)