"""Neural-network building blocks used by the perceptronium fitting workflow."""

import torch
import torch.nn as nn


class AtomNet(nn.Module):
    """Per-atom feed-forward network used to predict atomic embedding energy."""

    def __init__(self, netconfig):
        """Create a dense MLP following the layer sizes in ``netconfig``."""
        super(AtomNet, self).__init__()
        self.layers = nn.ModuleList()
        for i in range(len(netconfig) - 1):
            self.layers.append(nn.Linear(netconfig[i], netconfig[i + 1]))

    def forward(self, x):
        """Run inference with ReLU activations on all but the output layer."""
        for i in range(len(self.layers) - 1):
            x = torch.relu(self.layers[i](x))
        return self.layers[-1](x)


class MultiMolNet(nn.Module):
    """Type-resolved molecular network that sums per-atom energy contributions."""

    def __init__(self, num_types, netconfig=[124, 64, 16, 1]):
        """Initialize one ``AtomNet`` per atom type with a shared architecture."""
        super(MultiMolNet, self).__init__()
        self.num_types = num_types
        self.subnetworks = nn.ModuleList([AtomNet(netconfig) for _ in range(num_types)])

    def forward(self, x_padded, mask):
        """Predict total molecular energies from padded atom features and masks."""
        batch_size, max_num_types, feature_size = x_padded.shape

        type_indices = x_padded[:, :, 0].long()
        feature_vectors = x_padded[:, :, 1:]

        outputs = torch.zeros(batch_size, max_num_types, device=x_padded.device)

        for type_idx in range(self.num_types):
            mask_type = (type_indices == type_idx).float().unsqueeze(-1)
            selected_features = feature_vectors * mask_type
            sub_output = self.subnetworks[type_idx](selected_features)
            outputs += sub_output.squeeze(-1) * mask

        return outputs.sum(dim=1).view(batch_size, 1)
