import matplotlib.pyplot as plt
import networkx as nx

def draw_neural_network(layers):
    G = nx.DiGraph()
    pos = {}
    y_offset = 0.5
    
    # Assign positions for each layer, centering vertically
    x_pos = 0
    for i, num_nodes in enumerate(layers):
        y_start = -((num_nodes - 1) * y_offset) / 2  # Centering vertically
        for j in range(num_nodes):
            pos[(x_pos, j)] = (x_pos, y_start + j * y_offset)
        x_pos += 1
    
    # Connect layers
    for i in range(len(layers) - 1):
        for j in range(layers[i]):
            for k in range(layers[i + 1]):
                G.add_edge((i, j), (i + 1, k))
    
    # Draw the network
    plt.figure(figsize=(24, 16), dpi=300)
    nx.draw(G, pos, with_labels=False, node_size=15, edge_color='grey', alpha=0.5, width=0.5)
    
    plt.title("Neural Network Visualization")
    plt.savefig('nn.jpg')

# Define the network structure
layers = [124, 64, 16, 1]
draw_neural_network(layers)