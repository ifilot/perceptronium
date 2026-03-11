"""Dataset helpers for loading and preprocessing perceptronium training data."""

import torch
import numpy as np
from torch.utils.data import DataLoader, Dataset, TensorDataset, ConcatDataset, random_split
from collections import defaultdict
import matplotlib.pyplot as plt

class MultiMolDataset(torch.utils.data.Dataset):
    """Torch dataset for molecular symmetry-function features and target energies."""

    def __init__(self, dataset_paths:list[str], requestfile_path:str, normalize=False):
        """Load one or more packaged datasets and optionally normalize features."""
        
        # count number of entries per symmetry functions
        self.entry_counts = self.count_lines_per_entry(requestfile_path)

        datasets = []
        for f in dataset_paths:
            data = np.load(f)
            self.energies = data['energies']
            coefficients = data['netcoefficients']
            self.elset = data['elements']

            # incorporate network indicator into dataset alongside feature vector
            network_indicator = np.tile(np.array(self.elset).reshape(len(self.elset),1), (len(coefficients), 1, 1))
            coefficients = np.concatenate((network_indicator, coefficients), axis=2)

            # create input
            x = torch.tensor(coefficients, dtype=torch.float32)
            y = torch.tensor(self.energies.flatten(), dtype=torch.float32).unsqueeze(1)
            datasets.append(TensorDataset(x, y))
        
        self.raw_data = ConcatDataset(datasets)
    
        if normalize:
            print("Normalizing data...")
            self.grouped_indices = self.group_indices(self.elset)
            symmetry_stats = self.compute_symmetry_statistics(self.raw_data)
            self.data = self.normalize_dataset_per_atom_type(self.raw_data, symmetry_stats)
        else:
            self.data = self.raw_data

    def save_normalized(self, filename):
        """Persist the current (possibly normalized) coefficients to disk."""
        coefficients = np.vstack([data[0].numpy() for data in self.data]).reshape((-1, len(self.elset), self.data[0][0].shape[-1]))
        np.savez_compressed(filename, energies=self.energies, coefficients=coefficients, elements=self.elset)

        print("Storing coefficients: %s" % str(coefficients.shape))

    def count_lines_per_entry(self, file_path:str):
        """Count request-file entries per symmetry-function section."""
        entry_counts = {}
        current_entry = None
        
        with open(file_path, 'r') as f:
            for line in f:
                line = line.strip()
                
                # Skip empty lines and comments
                if not line or line.startswith("#"):
                    continue
                
                # Check if the line starts with an entry name (e.g., G0:)
                if line.endswith(":"):
                    current_entry = line[:-1]  # Remove the colon
                    entry_counts[current_entry] = 0  # Initialize count
                
                # Count lines under the current entry
                elif current_entry is not None:
                    entry_counts[current_entry] += 1
        
        return entry_counts

    def compute_symmetry_statistics(self, dataset):
        """
        Computes statistics (mean, std, min, max) per symmetry function and per
        atom type from a ConcatDataset.
        
        Args:
            dataset (ConcatDataset): The dataset containing entries for multiple
            symmetry functions. entry_counts (dict): Dictionary containing the
            number of rows per symmetry function.

        Returns:
            dict: A dictionary with computed statistics for each symmetry
            function and per atom type.
        """
        statistics = []
        
        for i,group in enumerate(self.grouped_indices):
            index = 1  # Track index for slicing
            stats = {}
            for sym_func, num_rows in self.entry_counts.items():
                if num_rows == 0:
                    continue  # Skip if no entries exist

                # Extract relevant rows from the dataset
                entries = [dataset[i][0][list(group),index:(index+num_rows)] for i in range(len(dataset))]
                index += num_rows  # Move index forward

                # Convert to tensor for numerical calculations
                data_tensor = torch.stack(entries)  # Shape: (num_rows, N+1)

                # Compute statistics
                stats[sym_func] = {
                    'mean': torch.mean(data_tensor).item(),
                    'std': torch.std(data_tensor).item(),
                    'min': torch.min(data_tensor).item(),
                    'max': torch.max(data_tensor).item(),
                }

            statistics.append(stats)

        return statistics

    def normalize_dataset_per_atom_type(self, dataset, statistics):
        """
        Normalizes a ConcatDataset using precomputed mean and std per symmetry function and per atom type.
        
        The first column is copied without normalization.

        Args:
            dataset (ConcatDataset): The dataset to normalize.
            entry_counts (dict): Dictionary mapping symmetry function names to their entry count.
            grouped_indices (list of sets): A list where each set contains indices corresponding to an atom type.
            statistics (list of dicts): A list of dictionaries containing mean and std per symmetry function per atom type.

        Returns:
            list: A list of normalized tensors for each entry.
        """
        normalized_data = []

        for sample_idx in range(len(dataset)):  # Iterate over dataset entries
            normalized_sample = []
            first_column = dataset[sample_idx][0][:, 0].unsqueeze(-1)  # Copy first column without normalization
            
            for atom_type_idx, group in enumerate(self.grouped_indices):  # Iterate over atom types
                index = 1  # Start from second column
                normalized_atom_type = []

                for sym_func, num_rows in self.entry_counts.items():  # Iterate over symmetry functions
                    if num_rows == 0:
                        continue  # Skip if no entries exist
                    
                    # Extract relevant columns for this symmetry function
                    raw_values = dataset[sample_idx][0][list(group), index:(index + num_rows)]
                    
                    # Retrieve precomputed mean and std
                    min = statistics[atom_type_idx][sym_func]['min']
                    max = statistics[atom_type_idx][sym_func]['max']
                    mean = statistics[atom_type_idx][sym_func]['mean']
                    std = statistics[atom_type_idx][sym_func]['std']
                    
                    if sym_func != 'G0':
                        # Apply Z-score normalization
                        #normalized_values = (raw_values - mean) / (std + 1e-8)  # Avoid division by zero
                        normalized_values = (raw_values - min) / (max - min)  # Avoid division by zero
                    else:
                        normalized_values = raw_values
                    
                    normalized_atom_type.append(normalized_values)
                    index += num_rows  # Move to next symmetry function

                normalized_sample.append(torch.cat(normalized_atom_type, dim=-1))  # Concatenate for atom type
            
            # Stack atom types together and keep labels if dataset contains them
            normalized_data.append((torch.cat([first_column, torch.cat(normalized_sample, dim=0)], dim=-1), dataset[sample_idx][1]))

        return normalized_data

    def group_indices(self, indices):
        """
        Groups indices into sets where each set contains indices with the same value.
        
        Args:
            indices (list): A list of index values.

        Returns:
            list of sets: A list where each set contains indices with the same value.
        """
        grouped = defaultdict(set)

        for i, value in enumerate(indices):
            grouped[value].add(i)  # Store index positions in sets

        return list(grouped.values())

    def compute_and_plot_histograms(self, graphfile=None, raw=False):
        """
        Computes and plots histograms of symmetry function values per atom type and per dataset entry.

        Args:
            entry_counts (dict): Dictionary containing the number of rows per symmetry function.
            grouped_indices (list of sets): A list where each set contains indices corresponding to an atom type.

        Returns:
            dict: A dictionary containing computed statistics for each symmetry function and per atom type.
        """       

        # select whether to use raw data or normalized data
        if raw:
            dataset = self.raw_data
        else:
            dataset = self.data

        fig, ax = plt.subplots(len(self.grouped_indices), len(self.entry_counts), 
                               figsize=(len(self.entry_counts)*4,len(self.grouped_indices)*4))

        for i,group in enumerate(self.grouped_indices):
            index = 1  # Track index for slicing

            for j,(sym_func, num_rows) in enumerate(self.entry_counts.items()):
                if num_rows == 0:
                    continue  # Skip if no entries exist

                # Extract relevant rows from the dataset
                entries = [dataset[i][0][list(group),index:(index+num_rows)] for i in range(len(dataset))]
                index += num_rows  # Move index forward

                # Convert to tensor for numerical calculations
                data_tensor = torch.stack(entries)  # Shape: (num_rows, N+1)

                hist, bin_edges = np.histogram(data_tensor.flatten(), bins=50)
                bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2

                # Create bar plot
                ax[i,j].bar(bin_centers, hist, width=bin_edges[1]-bin_edges[0], edgecolor='black', alpha=0.7)

                # Labels and title
                ax[i,j].set_xlabel('Value')
                ax[i,j].set_ylabel('Frequency')
                ax[i,j].set_title(sym_func)

        plt.tight_layout()
        # plot or show
        if graphfile is not None:
            plt.savefig(graphfile)
        else:
            plt.show()

    def __len__(self):
        """Return the number of available samples."""
        return len(self.data)

    def __getitem__(self, idx):
        """Return one sample tuple ``(x, y)`` by index."""
        return self.data[idx]
