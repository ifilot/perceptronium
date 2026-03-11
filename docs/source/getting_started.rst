Getting started
===============

The tutorials in this repository work best when executed from a Linux
environment. If you are running Windows, you can make use of Windows Subsystems
for Linux (WSL). The tutorials have been tested on Ubuntu 22.04.



First, install the following packages

.. code:: bash
    
    sudo apt install python3 python3-dev python3-venv build-essential cmake


Next, create a virtual Python environment and activate it. 

.. note::

    Using :code:`python-venv` provides a lightweight and efficient way to create
    isolated environments, ensuring that dependencies for different projects
    remain separate from system-wide packages. This prevents version conflicts,
    as each virtual environment maintains its own set of installed libraries
    without affecting or being affected by global Python packages. By using
    :code:`python-venv`, developers can work on multiple projects requiring
    different package versions without interference, reducing the risk of
    breaking dependencies across projects. Additionally, it enhances
    reproducibility, as the environment can be easily recreated, making it
    easier to share projects and deploy them consistently across different
    systems.

.. code:: bash

    python3 -m venv ~/.venv-perceptronium
    source ~/.venv-perceptronium/bin/activate

Finally, install the following Python packages in the environment we just
created.

.. code:: bash

    pip install torch torchvision torchaudio pyyaml matplotlib ase tqdm

.. important::

    When using :code:`python-venv`, the user must ensure the environment is
    activated before running any Python scripts that depend on it. Activation is
    done using :code:`source ~/venv-perceptronium/bin/activate`. However, this
    activation is temporary: when the user logs out or closes the terminal, the
    environment is automatically deactivated. To continue using it in a new
    session, the user must reactivate the environment before running any
    commands.