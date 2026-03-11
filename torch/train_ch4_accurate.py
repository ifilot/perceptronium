import os
from perceptronium import MultiMolDataset, MultiMolNet, MolFitter

ROOT = os.path.dirname(__file__)

def main():
    paths = [
        os.path.join(ROOT, '..', 'data', 'ch4_ht_coeff.npz'),
        os.path.join(ROOT, '..', 'data', 'ch4_lt_coeff.npz'),
    ]
    dataset = MultiMolDataset(paths, os.path.join(ROOT, '..', 'data', 'request'))

    outpath = os.path.join(ROOT, '..', 'output')
    os.makedirs(outpath, exist_ok=True) # ensure folder is created if it does not yet exists

    # set-up fitter class and start training the network
    fitter = MolFitter(num_types=2)
    fitter.fit(dataset, num_epochs=5000)

    # produce fitting plots
    fitter.create_parity_plot(graphfile=os.path.join(outpath, 'ch4_parity.png'))
    fitter.create_loss_history_plot(graphfile=os.path.join(outpath, 'ch4_loss_history.png'))

    # store network
    fitter.save_model(os.path.join(outpath, 'ch4_network_accurate.pth'))

if __name__ == '__main__':
    main()