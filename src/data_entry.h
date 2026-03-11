/*******************************************************************************
 *  
 *  Copyright (C) 2024 Ivo Filot
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 * 
 ******************************************************************************/

#pragma once

#include <array>
#include <cstdint>
#include <vector>

/**
 * @brief Represents a single data entry for atomic simulations.
 *
 * This structure stores information about a material or molecular system,
 * including energy, periodicity, lattice parameters, atomic species, and coordinates.
 */
struct DataEntry {
    double Energy;  ///< Total energy of the system (e.g., in eV).

    uint64_t Periodicity;  ///< Periodicity of the system (e.g., 0 for non-periodic, 1 for periodic in 1D, 2 for 2D, 3 for 3D).

    uint64_t NumBulkAtoms;  ///< Number of atoms considered as part of the bulk structure.

    std::array<std::array<float, 3>, 3> Lattice;  ///< 3x3 matrix representing lattice vectors (for periodic systems).

    std::vector<uint8_t> Atomlist;  ///< List of atomic numbers (Z-values) for each atom in the system.

    std::vector<uint8_t> ChemicalSymbols;  ///< Encoded representation of chemical element symbols (mapped to periodic table indices).

    std::vector<std::array<float, 3>> Coordinates;  ///< List of atomic coordinates in Cartesian space (x, y, z).
};
