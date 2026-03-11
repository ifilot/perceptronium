/*******************************************************************************
 *  
 *  Copyright (C) 2024 Ivo Filot <ivo@ivofilot.nl>
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

#include <iostream>// cout/endl
#include <fstream> // read/write files
#include <sstream> // istringstream/ostringstream
#include <iomanip> // setprecision(), setw()
#include <cmath>
#include <vector>
#include <algorithm> // STL sort (std::sort)
#include <cmath>
#include <memory>
#include <cstdint>

#include "atomlist.h"
#include "floats.h"
#include "requestfile.h"
#include "data_entry.h"

enum class DataBlock {
    ENERGY,
    CHEMICAL_SYMBOLS,
    ATOMLIST,
    LATTICE,
    ELECTRONIC_CONVERGENCE,
    IONIC_CONVERGENCE,
    PERIODICITY,
    NONE
};

/***************************************************
 * @brief Represents a DATA file for use in a Behler-Parrinello Neural Network.
 * 
 * This class manages atomic structures, symmetry functions, lattice information,
 * and associated metadata.
 ***************************************************/
class DataFile {

private:
    // **Atomic and Structural Data**
    int nr_atoms = 0;                                  ///< Number of atoms in the system.
    std::vector<uint8_t> atomlist;                     ///< List of atom indices.
    std::vector<std::string> chemicalSymbols;          ///< List of chemical symbols.
    EigenMatrixX3f coordinates;                        ///< Cartesian coordinates of unit cell atoms (Nx3).
    EigenMatrix3ff lattice = EigenMatrix3ff::Zero(3, 3);   ///< Lattice vectors (3x3).
    EigenMatrixXff symmetry_functions;                 ///< Symmetry function values.

    // **File Storage and Metadata**
    std::string filename;                              ///< Name of the associated data file.
    double energy = 0.0;                               ///< Energy of the system.
    std::string energy_string;                         ///< Energy stored as a string.
    std::string periodicity_string;                    ///< Periodicity stored as a string.
    std::string convergence_electronic;                ///< Electronic convergence data.
    std::string convergence_ionic;                     ///< Ionic convergence data.

    // **Raw Data Storage for File Content**
    std::string atomlist_string;                       ///< Atom list stored as a string.
    std::string chemicalSymbols_string;                ///< Chemical symbols stored as a string.
    std::string lattice_string1, lattice_string2, lattice_string3; ///< Lattice vectors stored as strings.
    std::vector<std::string> coordinate_strings;       ///< Cartesian coordinates stored as strings.

    // **Bulk Structure Data**
    bool bulk_created = false;                         ///< Flag indicating if bulk structure is generated.
    EigenMatrixXff bulkcoordinates;                    ///< Coordinates of generated bulk atoms.
    std::vector<floaty> distance_matrix_lower_triangular; ///< Lower triangular storage of inter-atomic distances.
    int periodicity = 0;                               ///< Periodicity type: 0 = vacuum, 1 = wire, 2 = surface, 3 = bulk/gas phase.
    int nNeighbours[3] = {0, 0, 0};                    ///< Number of neighboring images considered for bulk.

    // **Private Helper Functions**
    void install_nnlist(bool verbose = false);        ///< Generates a distance matrix.
    void create_bulk(floaty max_cutoff, bool verbose = false); ///< Generates a bulk and distance matrix.

    /**
     * @brief Default constructor.
     */
    DataFile() {}

public:

    // remove copy and assignment oiperators
    DataFile(const DataFile&) = delete;
    DataFile& operator=(const DataFile&) = delete;

    /**
     * @brief Creates a DataFile object from a given filename.
     * @param filename Path to the `.data` file.
     * @return DataFile instance populated with file contents.
     */
    static DataFile* from_file(const std::string& filename);

    /**
     * @brief Populates the DataFile object with data from a DataEntry structure.
     * @param data DataEntry structure containing system information.
     */
    static DataFile* from_entry(const struct DataEntry& data);

    /**
     * @brief Reads and populates the DataFile object from a given file.
     * @param data_path Path to the `.data` file.
     */
    void populate_from_file(const std::string& data_path);

    /**
     * @brief Reads and populates the DataFile object from a given data entry.
     * @param data Data Entry
     */
    void populate_from_entry(const struct DataEntry& data);

    /**
     * @brief Computes symmetry functions and writes them to file.
     * @param request RequestFile object containing function parameters.
     * @param verbose Whether to print output during computation.
     */
    void calculate_symmetryfunctions(const RequestFile& request, bool verbose = false);

    /**
     * @brief Writes the DataFile contents, including symmetry functions, to file.
     * @param verbose Whether to print output during writing.
     */
    void write_to_datafile(bool verbose = false) const;

    // **Getter Functions**
    
    /**
     * @return Number of atoms in the system.
     */
    inline int get_nr_atoms() const { return this->nr_atoms; }

    /**
     * @return Number of unique atom types.
     */
    inline unsigned int get_nAtomTypes() const { return chemicalSymbols.size(); }

    /**
     * @return Number of atoms in the generated bulk.
     */
    inline int get_nr_atoms_bulk() const { return bulkcoordinates.rows(); }

    /**
     * @return The lattice matrix.
     */
    inline const EigenMatrix3ff& get_lattice() const { return this->lattice; }

    /**
     * @return Cartesian coordinates of all atoms.
     */
    inline const EigenMatrixX3f& get_allcoordinates() const { return this->coordinates; }

    /**
     * @return Bulk atom coordinates.
     */
    inline const EigenMatrixXff& get_allbulkcoordinates() const { return this->bulkcoordinates; }

    /**
     * @return Filename associated with this DataFile.
     */
    inline std::string get_filename() const { return this->filename; }

    /**
     * @return Energy of the system.
     */
    inline floaty get_energy() const { return this->energy; }

    /**
     * @return Symmetry function values.
     */
    inline const EigenMatrixXff& get_symmetry_functions() const { return this->symmetry_functions; }

    /**
     * @return Return atomlist
     */
    inline const auto& get_atomlist() const { return this->atomlist; }

    // **Retrieval Functions**
    
    /**
     * @brief Retrieves the atomic index at a given position.
     * @param index Atom index.
     * @return The atomic meta-index.
     */
    unsigned int get_atom(unsigned int index) const;

    /**
     * @brief Retrieves the chemical symbol of a given atom type.
     * @param index Index of the atom type.
     * @return The corresponding chemical symbol.
     */
    std::string get_chemicalSymbol(unsigned int index) const;

    /**
     * @brief Retrieves the chemical symbol of an atom in the atomlist.
     * @param index Atom index.
     * @return The chemical symbol of the atom.
     */
    std::string get_element(unsigned int index) const;

    /**
     * @brief Retrieves a specific lattice vector.
     * @param index Index of the lattice vector (0-2).
     * @return Row vector containing the lattice vector.
     */
    EigenRowVector3ff get_latticevector(int index) const;

    /**
     * @brief Retrieves Cartesian coordinates of a given atom.
     * @param index Atom index.
     * @return Row vector containing x, y, z coordinates.
     */
    EigenRowVector3ff get_coordinates(int index) const;

    /**
     * @brief Retrieves Cartesian coordinates of a bulk atom.
     * @param index Bulk atom index.
     * @return Row vector containing x, y, z coordinates.
     */
    EigenRowVector3ff get_bulkcoordinates(int index) const;

    // **Distance Matrix Functions**
    
    /**
     * @brief Retrieves inter-atomic distance between two atoms.
     * @param i Atom index.
     * @param j Atom index.
     * @return The distance between atoms i and j.
     */
    floaty get_distance(int i, int j) const;

    /**
     * @brief Determines the number of periodic copies needed for a given cutoff.
     * @param copies Array to store the number of periodic images in each direction.
     * @param cutoff Distance cutoff.
     */
    void get_copy(int (&copies)[3], floaty cutoff) const;

    /**
     * @brief Computes the smooth radial cut-off function for BPNN.
     *
     * This function calculates the smooth radial cut-off value based on the given
     * distance and cutoff radius. It is used in 2-body and 3-body symmetry 
     * functions such as G1, G2, G3, G3f, G4, and G5.
     *
     * @param distance Distance between two atoms in Angstroms.
     * @param cutoff   Cutoff radius in Angstroms.
     * @return The computed radial cut-off value.
     */
    static double Gij_radial_valid(double distance, double cutoff);

    /**
     * @brief Computes the smooth radial cut-off function with cutoff handling.
     *
     * This function applies a smooth radial cut-off based on the given distance
     * and cutoff radius. If the distance is greater than or equal to the cutoff,
     * it returns 0. Otherwise, it calls `Gij_radial_valid()` to compute the value.
     *
     * @param distance Distance between two atoms in Angstroms.
     * @param cutoff   Cutoff radius in Angstroms.
     * @return The computed radial cut-off value. Returns 0.0 if the distance
     *         is greater than or equal to the cutoff.
     */
    static double Gij_radial(double distance, double cutoff);

};
