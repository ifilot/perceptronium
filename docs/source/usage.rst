Usage
=====

This page describes the end-to-end workflow for using Perceptronium on your
own dataset, starting from VASP-style trajectories and energies (for example,
``XDATCAR`` + ``OUTCAR``), then creating a request file, generating
descriptors, and finally fitting a neural network.

Assumptions
-----------

* You already built the C++ executable (``./build/bpsfp``).
* Your Python environment has the dependencies from :doc:`getting_started`.
* You run commands from the repository root.

Step 1: Prepare geometry and energy files
-----------------------------------------

Perceptronium needs:

* a trajectory with geometries (for example ``XDATCAR``), and
* a text file with one energy value per frame.

If your energies are in ``OUTCAR``, extract them first:

.. code:: bash

   grep "y  w" OUTCAR | awk '{print $7}' > XDATEN

Now you should have a matching pair like ``XDATCAR`` and ``XDATEN``.

.. note::

   Ensure the number of energies in ``XDATEN`` equals the number of trajectory
   frames in ``XDATCAR``.

Step 2: Convert to a package file
---------------------------------

Convert your trajectory + energies to a binary ``.pkg`` file:

.. code:: bash

   python3 pysrc/xdatcar2pack.py \
     -x /path/to/XDATCAR \
     -e /path/to/XDATEN \
     -o /path/to/my_system.pkg

You can repeat this for multiple simulations (for example, low/high
temperature runs).

Step 3: Create the request file
-------------------------------

The request file defines which symmetry functions are computed and with which
hyperparameters (``G0``, ``G1``, ``G2``, ``G3``, ``G3f``, ``G4``, ``G5``).

The easiest way to start is to copy the example request file and edit it:

.. code:: bash

   cp data/request data/my_request

Then open ``data/my_request`` and adapt the sections/values to your system.
See :doc:`symmetry_functions` for the functional forms.

Step 4: Generate descriptors
----------------------------

Run the descriptor generator with your package file and request file:

.. code:: bash

   ./build/bpsfp \
     -d /path/to/my_system.pkg \
     -r data/my_request \
     -o /path/to/my_system_coeff.npz

This creates an ``.npz`` file containing the energies and generated
symmetry-function coefficients.

Step 5: Fit a neural network
----------------------------

Use the training utilities in ``torch/`` with:

* one or more descriptor files (``*.npz``), and
* the same request file used during descriptor generation.

Minimal training example:

.. code:: python

   import os
   from perceptronium import MultiMolDataset, MolFitter

   paths = [
       os.path.join('data', 'my_system_coeff.npz'),
   ]

   dataset = MultiMolDataset(paths, os.path.join('data', 'my_request'))

   fitter = MolFitter(num_types=2)  # set this to the number of element types
   fitter.fit(dataset, num_epochs=2000)

A practical reference script is available at ``torch/train_ch4.py``.

Summary workflow
----------------

1. ``OUTCAR`` -> extract energies to ``XDATEN``.
2. ``XDATCAR`` + ``XDATEN`` -> convert to ``.pkg``.
3. Create/edit request file.
4. ``.pkg`` + request -> generate ``.npz`` descriptors.
5. ``.npz`` + request -> fit network in PyTorch.
