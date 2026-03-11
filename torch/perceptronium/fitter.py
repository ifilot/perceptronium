"""Model fitting utilities for perceptronium neural-network potentials."""

import os
import time

import matplotlib.pyplot as plt
import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from torch.nn.utils.rnn import pad_sequence
from torch.utils.data import DataLoader, random_split

from .network import MultiMolNet


class MolFitter:
    """High-level trainer for ``MultiMolNet`` models."""

    def __init__(self, num_types, netconfig=[124, 64, 16, 1]):
        """Initialize the fitting device and construct the neural network."""
        print("CUDA Available:", torch.cuda.is_available())
        print("Number of GPUs:", torch.cuda.device_count())
        print("GPU Name:", torch.cuda.get_device_name(0) if torch.cuda.is_available() else "No GPU found")

        self.device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
        print(f"Using device: {self.device}")

        self.model = MultiMolNet(num_types, netconfig).to(self.device)

    def fit(self, dataset, num_epochs: int = 100, batch_size: int = 256, track_evolution=None):
        """Train the model and store training/validation loss history."""
        criterion = nn.MSELoss()
        optimizer = optim.Adam(self.model.parameters(), lr=0.001)

        train_size = int(0.8 * len(dataset))
        val_size = len(dataset) - train_size
        train_set, val_set = random_split(dataset, [train_size, val_size])

        batch_size = 256
        self.train_loader = DataLoader(train_set, batch_size=batch_size, shuffle=True, collate_fn=self.custom_collate)
        self.val_loader = DataLoader(val_set, batch_size=batch_size, shuffle=False, collate_fn=self.custom_collate)

        self.train_loss_arr = []
        self.val_loss_arr = []

        print(f"{'Epoch':<10}{'Train Loss':<15}{'Val Loss':<15}{'Time (sec)':<10}")
        print("=" * 50)

        for epoch in range(num_epochs):
            start_time = time.time()

            self.model.train()
            train_loss = 0

            for batch_x, batch_y, mask in self.train_loader:
                batch_x, batch_y, mask = batch_x.to(self.device), batch_y.to(self.device), mask.to(self.device)
                optimizer.zero_grad()
                output = self.model(batch_x, mask)
                loss = criterion(output, batch_y)
                loss.backward()
                optimizer.step()
                train_loss += loss.item()

            train_loss /= len(self.train_loader)

            self.model.eval()
            val_loss = 0

            with torch.no_grad():
                for batch_x, batch_y, mask in self.val_loader:
                    batch_x, batch_y, mask = batch_x.to(self.device), batch_y.to(self.device), mask.to(self.device)
                    output = self.model(batch_x, mask)
                    loss = criterion(output, batch_y)
                    val_loss += loss.item()

            val_loss /= len(self.val_loader)

            self.train_loss_arr.append(train_loss)
            self.val_loss_arr.append(val_loss)

            epoch_time = time.time() - start_time
            print(f"{epoch+1:>6}{train_loss:>15.6f}{val_loss:>15.6f}{epoch_time:>10.2f}")

            if track_evolution is not None:
                self.create_parity_plot(os.path.join(track_evolution, "%05i.png" % (epoch + 1)))

    def custom_collate(self, batch):
        """Pad variable-size atom feature tensors and return a validity mask."""
        batch_X, batch_y = zip(*batch)

        batch_X_padded = pad_sequence(batch_X, batch_first=True, padding_value=0)
        batch_y = torch.tensor(batch_y, dtype=torch.float32).view(-1, 1)
        mask = (batch_X_padded[:, :, 0] != 0).float()

        return batch_X_padded, batch_y, mask

    def create_parity_plot(self, graphfile: str | None = None):
        """Generate a parity plot for validation predictions."""
        self.model.eval()

        y_true_list = []
        y_pred_list = []

        with torch.no_grad():
            for batch_x, batch_y, mask in self.val_loader:
                batch_x, batch_y, mask = batch_x.to(self.device), batch_y.to(self.device), mask.to(self.device)
                y_pred = self.model(batch_x, mask)
                y_true_list.append(batch_y.cpu().numpy())
                y_pred_list.append(y_pred.cpu().numpy())

        y_true = np.concatenate(y_true_list).flatten()
        y_pred = np.concatenate(y_pred_list).flatten()

        plt.figure(figsize=(6, 6))
        plt.scatter(y_true, y_pred, alpha=0.5, label="Predictions")
        plt.plot([y_true.min(), y_true.max()], [y_true.min(), y_true.max()], "r--", label="Perfect Fit")
        plt.xlabel("True Values (y_true)")
        plt.ylabel("Predicted Values (y_pred)")
        plt.title("Parity Plot")
        plt.legend()
        plt.grid()

        if graphfile is not None:
            plt.savefig(graphfile)
        else:
            plt.show()

    def create_loss_history_plot(self, graphfile: str | None = None):
        """Generate a log-scale plot of training and validation loss curves."""
        epochs = list(range(1, len(self.train_loss_arr) + 1))

        plt.figure(figsize=(8, 5))
        plt.semilogy(epochs, self.train_loss_arr, label="Training Loss", marker="o", linestyle="-", alpha=0.5)
        plt.semilogy(epochs, self.val_loss_arr, label="Validation Loss", marker="s", linestyle="--", alpha=0.5)

        plt.xlabel("Epoch")
        plt.ylabel("Loss")
        plt.title("Training vs Validation Loss")
        plt.legend()
        plt.grid(True)

        if graphfile is not None:
            plt.savefig(graphfile)
        else:
            plt.show()

    def save_model(self, path):
        """Serialize the trained model to ``path``."""
        torch.save(self.model, path)
