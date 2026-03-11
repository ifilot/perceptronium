Symmetry Functions
==================

Below, an overview of the symmetry functions are provided. These symmetry
functions are largerly based on `the work of Behler 
<https://doi.org/10.1063/1.3553717>`_.

One-body functions
------------------

The function :math:`G_{0}` encodes the atomic identity. Canonically, this
function is evaluated 6 times with values for :math:`\beta` corresponding
to  :math:`(0.4, 0.5, 0.6, 0.8, 0.9, 1.0)`.

.. math::

    G^{(0)}_{i} = \frac{\min(|\beta| , 1) \cdot Z_{i}}{118}

:math:`Z` corresponds to the atomic number, i.e. :math:`Z=1` for H and
:math:`Z=6` for carbon.

Two-body functions
------------------

The two-body functions, also called the radial symmetry functions, probe the
radial environment of every atom :math:`i`.

.. math::

    G^{(1)}_{i} = \sum_{j \neq i} f_{c} \left(R_{ij} \right)

where :math:`f_{c}` is a smooth cut-off function given by

.. math::

    f_{c}(R_{ij}) = \begin{cases}
        0.5 \cdot \left[ \cos \left( \frac{\pi R_{ij}}{R_{c}} \right) + 1 \right],\;&\text{for}\; R_{ij} \leq R_{c} \\
        0&\text{otherwise}
    \end{cases}

In the above equation, :math:`R_{c}` acts as the cut-off radius, which is
predefined at the start of the study. A typical value is :math:`R_{c} = 6.5`.

.. math::

    G^{(2)}_{i} = \sum_{j \neq i} \exp \left(- \eta \left(R_{ij} - R_{s} \right)^{2} \right) f_{c} \left(R_{ij} \right)

In the above equation, the atomic environment is sampled using Gaussians. These
Gaussians can be shifted using :math:`R_{s}`. When using atom-centered
Gaussians, the value used corresponds to :math:`R_{s} = 0`. The parameter
:math:`\eta` is used to tune the width of the Gaussians, its values are
typically set to :math:`\eta = (0, 0.01, 0.025, 0.05, 0.1, 0.15, 0.25, 0.5, 1)`.