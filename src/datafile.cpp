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

#include "datafile.h"

#include <cmath>  // For M_PI

/**
 * @brief Creates a DataFile object from a specified file.
 *
 * This function reads data from the given file path and populates a 
 * DataFile object using the `populate_from_file` method.
 *
 * @param data_path The path to the file containing the data.
 * @return A DataFile object populated with the contents of the specified file.
 */
DataFile* DataFile::from_file(const std::string& data_path) {
    DataFile *datafile = new DataFile();                // Create an empty DataFile object
    datafile->populate_from_file(data_path);            // Populate the object from the file
    return datafile;                                    // Return the populated object
}

/**
 * @brief Populates a DataFile object by reading data from a `.data` file.
 *
 * This function reads a specified `.data` file and extracts relevant information
 * such as energy, chemical symbols, atom list, lattice vectors, convergence data, 
 * periodicity, and atomic coordinates.
 *
 * @param data_path Path to the `.data` file to be read.
 */
void DataFile::populate_from_file(const std::string& data_path) {
    // Attempt to open the file for reading
    std::ifstream data_file(data_path);
    if (!data_file.is_open()) {
        std::cerr << "Error: DATA file [" << data_path << "] does not exist." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Reset parse state in case this object is re-used.
    this->atomlist.clear();
    this->chemicalSymbols.clear();
    this->coordinate_strings.clear();

    // Store the file path for later use
    this->filename = data_path;

    std::string line;
    std::vector<std::string> lines; // Stores the full file content

    // Skip the file header (first line)
    std::getline(data_file, line);

    // Read and store the rest of the file content
    while (std::getline(data_file, line)) {
        lines.push_back(line);
    }

    // Close the file after reading
    data_file.close();

    // Initialize parsing state variables
    bool readflag = false;                 // Whether we're inside a data block
    DataBlock blocktype = DataBlock::NONE; // Tracks the type of data being processed
    std::istringstream iss;                // String stream for parsing

    floaty var_double;    // For reading floating-point values
    unsigned int var_int; // For reading integer values
    std::string var_string; // For reading string values

    int lattice_counter = 0;     // Index tracker for lattice vectors
    int coordinates_counter = 0; // Index tracker for atomic coordinates

    // Iterate through the file content and process data blocks
    for (const auto& line : lines) {
        const auto first_non_whitespace = line.find_first_not_of(" \t\r");
        const bool is_blank_line = (first_non_whitespace == std::string::npos);

        if (!readflag) {
            // Identify the start of a data block
            if (line.find("Energy") != std::string::npos) {
                readflag = true;
                blocktype = DataBlock::ENERGY;
            } else if (line.find("Chemical Symbols") != std::string::npos) {
                readflag = true;
                blocktype = DataBlock::CHEMICAL_SYMBOLS;
            } else if (line.find("Atomlist") != std::string::npos) {
                readflag = true;
                blocktype = DataBlock::ATOMLIST;
            } else if (line.find("Lattice") != std::string::npos) {
                readflag = true;
                blocktype = DataBlock::LATTICE;
            } else if (line.find("Electronic Convergence") != std::string::npos) {
                readflag = true;
                blocktype = DataBlock::ELECTRONIC_CONVERGENCE;
            } else if (line.find("Ionic Convergence") != std::string::npos) {
                readflag = true;
                blocktype = DataBlock::IONIC_CONVERGENCE;
            } else if (line.find("Periodicity") != std::string::npos) {
                readflag = true;
                blocktype = DataBlock::PERIODICITY;
            }
        } 
        else { // Reading data from an active block
            if (is_blank_line) {
                readflag = false; // End of current block
            } 
            else if (blocktype == DataBlock::ENERGY) {
                this->energy_string = line;
                this->energy = std::stod(line);
            } 
            else if (blocktype == DataBlock::CHEMICAL_SYMBOLS) {
                this->chemicalSymbols_string = line;
                iss.clear();
                iss.str(line);
                while (iss >> var_string) {
                    this->chemicalSymbols.push_back(var_string);
                }
            } 
            else if (blocktype == DataBlock::ATOMLIST) {
                this->atomlist_string = line;
                iss.clear();
                iss.str(line);
                while (iss >> var_int) {
                    this->atomlist.push_back(var_int);
                }
                this->nr_atoms = this->atomlist.size();
            } 
            else if (blocktype == DataBlock::LATTICE) {
                if (lattice_counter >= 3) {
                    std::cerr << "Error: Improper DATA file formatting for [" << data_path << "]." << std::endl;
                    std::cerr << "       Too many lattice vectors or multiple Lattice blocks detected." << std::endl;
                    exit(EXIT_FAILURE);
                }
                if (lattice_counter == 0) {
                    lattice_string1 = line;
                } else if (lattice_counter == 1) {
                    lattice_string2 = line;
                } else if (lattice_counter == 2) {
                    lattice_string3 = line;
                }
                iss.clear();
                iss.str(line);
                iss >> this->lattice(lattice_counter, 0);
                iss >> this->lattice(lattice_counter, 1);
                iss >> this->lattice(lattice_counter, 2);
                lattice_counter++;
            } 
            else if (blocktype == DataBlock::ELECTRONIC_CONVERGENCE) {
                this->convergence_electronic = line;
            } 
            else if (blocktype == DataBlock::IONIC_CONVERGENCE) {
                this->convergence_ionic = line;
            } 
            else if (blocktype == DataBlock::PERIODICITY) {
                this->periodicity_string = line;
                iss.clear();
                iss.str(line);
                iss >> this->periodicity;
            }
        }
    }

    if (this->nr_atoms <= 0) {
        std::cerr << "Error: DATA file [" << data_path << "] contains no atoms in Atomlist." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (lattice_counter != 3) {
        std::cerr << "Error: DATA file [" << data_path << "] does not contain exactly 3 lattice vectors." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (this->periodicity < 0 || this->periodicity > 3) {
        std::cerr << "Error: DATA file [" << data_path << "] contains invalid periodicity " << this->periodicity
                  << " (allowed: 0..3)." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Initialize the coordinates matrix
    this->coordinates = EigenMatrixX3f::Zero(this->nr_atoms, 3);

    // Second pass: extract atomic coordinates
    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i].find("Coordinates") != std::string::npos) {
            for (size_t j = i + 1; j < i + 1 + this->nr_atoms && j < lines.size(); j++) {
                if (coordinates_counter >= this->nr_atoms) {
                    break;
                }
                coordinate_strings.push_back(lines[j]);
                iss.clear();
                iss.str(lines[j]);
                iss >> this->coordinates(coordinates_counter, 0);
                iss >> this->coordinates(coordinates_counter, 1);
                iss >> this->coordinates(coordinates_counter, 2);
                coordinates_counter++;
            }
            break; // Exit loop after processing coordinates
        }
    }

    if (coordinates_counter != this->nr_atoms) {
        std::cerr << "Error: DATA file [" << data_path << "] contains " << coordinates_counter
                  << " coordinates, expected " << this->nr_atoms << "." << std::endl;
        exit(EXIT_FAILURE);
    }
}

DataFile* DataFile::from_entry(const struct DataEntry& data) {
    DataFile* datafile = new DataFile();                // Create an empty DataFile object
    datafile->populate_from_entry(data);                // Populate the object from the file
    return datafile;                                    // Return the populated object
}

void DataFile::populate_from_entry(const struct DataEntry& data) {
    this->nr_atoms = data.Atomlist.size();
    this->atomlist = data.Atomlist;

    // parse elements
    this->chemicalSymbols.clear();
    for(auto atidx : data.ChemicalSymbols) {
        this->chemicalSymbols.push_back(periodicTableVector[atidx]);
    }
    
    this->coordinates = EigenMatrixX3f::Zero(this->nr_atoms, 3);  // Correct function is Zero(), not Zeros()
    for (unsigned int i = 0; i < this->nr_atoms; i++) {
        this->coordinates.set_row3(i, {data.Coordinates[i][0], data.Coordinates[i][1], data.Coordinates[i][2]});
    }

    for (unsigned int i = 0; i < 3; i++) {
        this->lattice.set_row3(i, {data.Lattice[i][0], data.Lattice[i][1], data.Lattice[i][2]});
    }

    this->energy_string = std::to_string(data.Energy);
    this->energy = data.Energy;
    // std::string periodicity_string;
    // std::string convergence_electronic;
    // std::string convergence_ionic;

    // std::string atomlist_string;
    // std::string chemicalSymbols_string;
    // std::string lattice_string1;
    // std::string lattice_string2;
    // std::string lattice_string3;
    // std::vector<std::string> coordinate_strings;

    // std::string filename; // the file to which we will write this DATA file

    periodicity = data.Periodicity;
}

/**
 * Write out the data file, including symmetry functions
 */
void DataFile::write_to_datafile(bool verbose) const{

    // attempt to open the file
    std::ofstream data_file;
    data_file.open(this->filename);

    if(verbose) {
        std::cout << "Writing symmetry character for: " << this->get_filename() << "." << std::endl;
    }

    data_file << "# Data file containing input for a Behler-Parrinello Neural Network" << std::endl;
    data_file << std::endl;

    data_file << "Energy: eV" << std::endl;
    data_file << this->energy_string << std::endl;
    data_file << std::endl;

    data_file << "Periodicity:" << std::endl;
    data_file << this->periodicity << std::endl;
    data_file << std::endl;

    data_file << "Lattice:" << std::endl;
    data_file << this->lattice_string1 << std::endl;
    data_file << this->lattice_string2 << std::endl;
    data_file << this->lattice_string3 << std::endl;
    data_file << std::endl;

    data_file << "Atomlist:" << std::endl;
    data_file << this->atomlist_string << std::endl;
    data_file << std::endl;

    data_file << "Chemical Symbols:" << std::endl;
    data_file << this->chemicalSymbols_string << std::endl;
    data_file << std::endl;

    data_file << "Number of Bulk Atoms:" << std::endl; // we store here also the number of bulk atoms so we can normalise the symmetry functions later
    data_file << "    " << this->get_nr_atoms_bulk() << std::endl;
    data_file << std::endl;

    data_file << "Electronic Convergence:" << std::endl;
    data_file << this->convergence_electronic << std::endl;
    data_file << std::endl;

    data_file << "Ionic Convergence:" << std::endl;
    data_file << this->convergence_ionic << std::endl;
    data_file << std::endl;

    data_file << "Coordinates: Cartesian" << std::endl;
    for(unsigned int atom=0; atom<this->get_nr_atoms(); atom++) {
        data_file << this->coordinate_strings[atom] << std::endl;
    }
    data_file << std::endl;

    data_file << "Symmetry Functions:" << std::endl;

    std::ostringstream strs;
    strs << std::fixed << std::setprecision(9); // use 9 digits after the . and set field width to 15 = 9 digits + . + 4 digits + minus sign
    std::string line;
    for(int atom=0; atom<this->get_nr_atoms(); atom++) {
        strs.str(""); // clear contents of ostringstream, do not reset flags to maintain precision and field width formatting
        for(int function=0; function<this->symmetry_functions.cols(); function++) {
            strs << "    " << std::setw(15) << this->symmetry_functions(atom,function); // setw needs to be in front of the double _every_ time, or it does not work
        }
        line.clear();      // empty string
        line = strs.str(); // convert ostringstream to string
        data_file << line << std::endl;
    }
    data_file << std::endl;

    data_file.close(); // close stream
}

/**
 * @brief Computes and writes symmetry functions.
 *
 * This function calculates the symmetry functions specified in the REQUEST file 
 * and writes the computed values to the DATA file.
 *
 * @param request The RequestFile object containing parameters and symmetry functions to compute.
 * @param verbose If true, outputs progress messages.
 */
void DataFile::calculate_symmetryfunctions(const RequestFile& request, bool verbose) {

    // Compute all 3-atom interactions:
    // - For each cutoff, dynamically adjust bulk indexing to exclude out-of-range atoms.
    // - Use the distance matrix to find 2-atom interactions.
    // - For each neighbor, identify valid diads and compute 3-body interactions.

    /**
     * Structure of the SymmetryFunctions matrix.
     *
     * Each row corresponds to an atom in the unit cell and contains all symmetry function values from the REQUEST file.
     * The order within each row:
     * 1. G0 values
     * 2. G1 values
     * 3. Rc blocks (sorted from smallest to largest cutoff), each containing:
     *    - G2 values
     *    - G3 values
     *    - G3f values
     *    - G4 values
     *    - G5 values
     */
    EigenMatrixXff SymmetryFunctions = EigenMatrixXff::Zero(this->get_nr_atoms(), request.get_nr_symmetry_functions()); // will be N x S

    // Symmetry function parameters used in BPNN descriptors; values are obtained from Request file
    floaty beta;   // Scaling factor for G0 symmetry function
    floaty Rc;     // Cutoff radius, controls range of atomic interactions
    floaty eta;    // Width parameter for Gaussian symmetry functions (G2, G4, G5)
    floaty Rs;     // Radial shift for Gaussian functions (G2)
    floaty kappa;  // Frequency parameter for cosine-based functions (G3, G3f)
    floaty zeta;   // Angular resolution parameter for three-body terms (G4, G5)
    int lambda;    // Angular inversion factor (+1 or -1) for G4 and G5

    // Interatomic distances used for neighbor identification and symmetry function calculations
    floaty distance_ij; // Distance between central atom (i) and neighbor (j)
    floaty distance_ik; // Distance between central atom (i) and second neighbor (k)
    floaty distance_kj; // Distance between first neighbor (j) and second neighbor (k)
    floaty cos_theta;   // Cosine of the bond angle (ijk) in three-body interactions
    floaty suffix;      // Cached radial cutoff product to optimize G4 and G5 calculations

    // Function indexing and atom mapping
    int function_counter; // Tracks position in the symmetry function matrix
    int bulkatom;         // Converts unit cell atomic index to bulk index for periodic systems

    // Precompute symmetry function subblock sizes to avoid redundant calculations
    int twobodyblock   = request.count_G2() + request.count_G3() + request.count_G3f();     // Number of two-body terms {G2, G3, G3f}
    int threebodyblock = request.count_G4() + request.count_G5();                           // Number of three-body terms {G4, G5}
    int nSyms_per_G1   = twobodyblock + threebodyblock;                                     // Total symmetry functions per cutoff Rc


    if(verbose) {
        std::cout << "Calculating symmetry character for: " << this->get_filename() << "." << std::endl;
    }

    // Process 1-body interactions (G0), which must be handled outside the cutoff cycle
    for (unsigned int atom = 0; atom < this->get_nr_atoms(); ++atom) {
        
        unsigned int function_counter = 0; // G0 values are stored first in each row

        // Compute the identity fingerprint (G0) to ensure feature scaling and normalization
        for (unsigned int G0 = 0; G0 < request.count_G0(); ++G0) {
            
            floaty beta = request.get_G0(G0); // Retrieve the beta parameter

            // Use system-independent indexing to ensure transferability across networks
            // Guarantees a nonzero baseline contribution, even for isolated atoms
            SymmetryFunctions(atom, function_counter) = CFLOATY_min(std::abs(beta)) * 
                                                        get_atomnumber(this->get_element(atom)) / 
                                                        get_element_normalisation();
            ++function_counter;
        }
    }

    // Expand the bulk structure to the maximum cutoff range  
    // This determines the maximum number of atoms to consider  
    // Dynamic indexing will be applied later to eliminate unnecessary atoms  
    this->create_bulk(request.get_max_cutoff());

    // Compute the number of image cells generated in each dimension  
    int xc = 2 * this->nNeighbours[0] + 1; // Total x-blocks  
    int yc = 2 * this->nNeighbours[1] + 1; // Total y-blocks per x-block  
    int zc = 2 * this->nNeighbours[2] + 1; // Total z-blocks per y-block  

    int copy[3] = {1, 1, 1}; // Initial number of images within the cutoff range  

    // Offset for unit cell atoms in bulk indexing  
    int bulkatomoff = this->nNeighbours[0] * yc * zc * this->get_nr_atoms() +
                      this->nNeighbours[1] * zc * this->get_nr_atoms() +
                      this->nNeighbours[2] * this->get_nr_atoms();

    // Offsets for dynamic bulk atom indexing (used for neighbor searches)  
    int neighbourxoff, neighbouryoff, neighbourzoff;
    int otherneighbourxoff, otherneighbouryoff, otherneighbourzoff;

    // Radial cutoff function values for two-body interactions  
    floaty fcp; // Evaluated for the first neighbor  
    floaty fmq; // Evaluated for the next neighbor  

    if (verbose) {
        std::cout << "Commencing Symmetry Calculations" << std::endl;
    }

    // Cutoff Cycle:  
    // Computes symmetry function values for each cutoff radius (Rc), minimizing  
    // bulk reorganization. G1 functions dictate the cycle as each Rc has one G1.  
    //  
    // Process:  
    // 1. Apply dynamic cutoff to define the bulk region for Rc.  
    // 2. Identify neighbors and compute 2-body interactions (G1, G2, G3, G3f).  
    // 3. Locate additional neighbors to form triads.  
    // 4. Compute 3-body interactions (G4, G5) for valid triads.  
    //  
    // Note: G4 is skipped if the triad does not contain three diads.  
    for(int G1=request.count_G1()-1; G1>-1; G1--) { // we perform a reverse-trace since the Rc values are ordered with lowest first

        Rc = request.get_G1(G1); // the cutoff values Rc are just the parameter values used for G1

        if(verbose) {
            std::cout << "Symmetry Block for Rc[" << G1 << "] = "<< Rc << " Angstrom" << std::endl;
        }

        this->get_copy(copy, Rc); // apply variational cutoff by choosing a dynamic index

        for(int atom=0; atom<this->get_nr_atoms(); atom++) { // atom i - we only need the symmetry functions for the atoms in the original unit cell

            bulkatom = bulkatomoff + atom; // offset unit cell index to the new bulk index

            // neighbour j -  compare against all atoms in the bulk
            // only include relevant x-blocks:
            for(int neighbourx=-copy[0]; neighbourx<copy[0]+1; neighbourx++) {

                neighbourxoff = (neighbourx + this->nNeighbours[0])*yc*zc*this->get_nr_atoms();

                 // only include relevant y-blocks:
                for(int neighboury=-copy[1]; neighboury<copy[1]+1; neighboury++) {

                    neighbouryoff = neighbourxoff + (neighboury + this->nNeighbours[1])*zc*this->get_nr_atoms();

                    // only include relevant z-blocks:
                    for(int neighbourz=-copy[2]; neighbourz<copy[2]+1; neighbourz++) {

                        // offset the coordinates of neighbour to those in the list of bulk coordinates
                        neighbourzoff = neighbouryoff + (neighbourz + this->nNeighbours[2])*this->get_nr_atoms();

                        // go over all nAtoms in this {x,y,z}-block
                        for(int neighbour=neighbourzoff; neighbour<neighbourzoff+this->get_nr_atoms(); neighbour++) {

                            // we will calculate the 2-body interactions first:
                            distance_ij = this->get_distance(bulkatom, neighbour);

                            if(distance_ij < Rc && bulkatom!=neighbour) { // skip non-neighbours, exclude i=j

                                /***************************************************

                                 The BPNN radial symmetry function

                                 G1 -- {Rc}

                                ***************************************************/
                                function_counter = request.count_G0() + G1;

                                fcp = Gij_radial_valid(distance_ij, Rc);
                                SymmetryFunctions(atom, function_counter) += fcp;

                                function_counter = request.count_G0() + request.count_G1() + G1 * nSyms_per_G1; // make sure nSyms is per Rc value


                                /***************************************************

                                 The BPNN Gaussian radial symmetry functions

                                 G2 --- {Rc,eta,Rs}

                                ***************************************************/
                                for(unsigned int G2=0; G2<request.count_G2(); G2++) {
                                    auto G2param = request.get_G2(G2);
                                    eta = G2param.eta;
                                    Rs = G2param.Rs;
                                    
                                    SymmetryFunctions(atom,function_counter) += std::exp(-eta*(distance_ij-Rs)*(distance_ij-Rs)) * fcp;
                                    function_counter++;
                                }

                                // BPNN Rectified Fourier Radial Symmetry Function (G3f)  
                                // Parameters: {Rc, kappa}  
                                // Computes a modified Fourier-based radial symmetry function  
                                // using absolute values to ensure non-negative outputs.  
                                for(unsigned int G3=0; G3<request.count_G3(); G3++) {
                                    kappa = request.get_G3(G3);
                                    SymmetryFunctions(atom,function_counter) += std::cos(kappa*distance_ij) * fcp;
                                    function_counter++;
                                }

                                // BPNN Fourier Radial Symmetry Function (G3)  
                                // Parameters: {Rc, kappa}  
                                // Computes a Fourier-based radial symmetry function.  
                                // G3 and G3f are kept separate to allow independent usage.
                                for(unsigned int G3f=0; G3f<request.count_G3f(); G3f++) {
                                    kappa = request.get_G3f(G3f);
                                    SymmetryFunctions(atom,function_counter) += std::abs(std::cos(kappa*distance_ij)) * fcp;
                                    function_counter++;
                                }

                                // now do the 2-body interactions:
                                // loop over all other diads in order to find the triads:
                                // find other neighbour k - again consider all bulk atoms as potential neighbours:
                                // only include relevant x-blocks:
                                for(int otherneighbourx=-copy[0]; otherneighbourx<copy[0]+1; otherneighbourx++) {

                                    otherneighbourxoff = (otherneighbourx + this->nNeighbours[0])*yc*zc*this->get_nr_atoms();

                                    // only include relevant y-blocks:
                                    for(int otherneighboury=-copy[1]; otherneighboury<copy[1]+1; otherneighboury++) {

                                        otherneighbouryoff = otherneighbourxoff + (otherneighboury + this->nNeighbours[1])*zc*this->get_nr_atoms();

                                        // only include relevant z-blocks:
                                        for(int otherneighbourz=-copy[2]; otherneighbourz<copy[2]+1; otherneighbourz++) {

                                            // offset the coordinates of otherneighbour to those in the list of bulk coordinates
                                            otherneighbourzoff = otherneighbouryoff + (otherneighbourz + this->nNeighbours[2])*this->get_nr_atoms();

                                            // go over all nAtoms in this block
                                            for(int otherneighbour=otherneighbourzoff; otherneighbour<otherneighbourzoff+this->get_nr_atoms(); otherneighbour++) {

                                                distance_ik = this->get_distance(bulkatom, otherneighbour);

                                                if(distance_ik < Rc && bulkatom!=otherneighbour && neighbour!=otherneighbour) { // skip if not a neighbour, or not a triad

                                                    // reset index for every "other neighbour" we find
                                                    function_counter = request.count_G0() + request.count_G1() + G1 * nSyms_per_G1 + twobodyblock;

                                                    distance_kj = this->get_distance(neighbour, otherneighbour); // only needed for G4, but this getter is cheaper than to write an exception
                                                    cos_theta = (this->get_bulkcoordinates(bulkatom) - this->get_bulkcoordinates(neighbour)).dot(this->get_bulkcoordinates(bulkatom) - this->get_bulkcoordinates(otherneighbour)) / (distance_ij*distance_ik);

                                                    if(distance_kj < Rc) { // G4 are only non-zero if the triad ijk is composed of 3 diads ij, ik _and_ kj, so skip all of them is this is not the case

                                                        // BPNN Angular Symmetry Function (G4)  
                                                        // Parameters: {Rc, eta, zeta}  
                                                        // Captures three-body interactions using bond angles and distances.  
                                                        // G4 is kept separate from G5 to allow independent parameter tuning.
                                                        fmq = Gij_radial_valid(distance_ik,Rc);
                                                        suffix = fcp * fmq * Gij_radial_valid(distance_kj,Rc); // avoid re-calculating this radial part for every angular component
                                                        for(unsigned int G4=0; G4<request.count_G4(); G4++) {

                                                            auto G4Param = request.get_G4(G4);
                                                            eta = G4Param.eta;
                                                            zeta = G4Param.zeta;
                                                            lambda = G4Param.lambda;

                                                            SymmetryFunctions(atom,function_counter) += powglob(std::abs(1.0+lambda*cos_theta), zeta) * 
                                                                                                        powglob(2.0, (-zeta) ) * 
                                                                                                        std::exp(-eta*(distance_ij*distance_ij + distance_ik*distance_ik + distance_kj*distance_kj)) * suffix;
                                                            function_counter++;
                                                        }
                                                        suffix = fcp * fmq; // suffix for G5
                                                    }
                                                    else{
                                                        function_counter += request.count_G4(); // correct function index if we skip G4
                                                        suffix = fcp * Gij_radial_valid(distance_ik,Rc); // suffix for G5
                                                    }

                                                    // BPNN Broad Angular Symmetry Function (G5)  
                                                    // Parameters: {Rc, eta, zeta}  
                                                    // Captures three-body interactions based on bond angles and distances.  
                                                    for(unsigned int G5=0; G5<request.count_G5(); G5++) {

                                                        auto G5param = request.get_G5(G5);
                                                        eta = G5param.eta;
                                                        zeta = G5param.zeta;
                                                        lambda = G5param.lambda;
                                                        SymmetryFunctions(atom,function_counter) += powglob(std::abs(1.0+lambda*cos_theta), zeta) * powglob(2.0, (-zeta) ) * std::exp(-eta*(distance_ij*distance_ij + distance_ik*distance_ik)) * suffix;
                                                        function_counter++;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Store in internal structure.
    // Use swap to avoid an element-wise Eigen assignment path that can trigger
    // aligned AVX packet loads on some toolchain/runtime combinations.
    this->symmetry_functions.swap(SymmetryFunctions);
}

/*************************************************************************************

 Setup the distance matrix

 We choose our indexing so that always i >= j
 This allows us to only save a lower triangular matrix, saving ca. 50% storage
 Which becomes quite significant as the number of bulk atoms can grow large quickly

*************************************************************************************/
void DataFile::install_nnlist(bool verbose) {

    if(verbose) {
        std::cout << "Generating Distance Matrix" << std::endl;
    }

    if(!this->bulk_created) {
        std::cerr << "First create bulk expansion before setting up distance matrix." << std::endl;
        exit(EXIT_FAILURE);
    }

    this->distance_matrix_lower_triangular.clear();
    this->distance_matrix_lower_triangular.reserve((this->get_nr_atoms_bulk()*(this->get_nr_atoms_bulk()+1))/2); // this will be the max size of the lower triangular matrix

    floaty distance_ij;

    //here we calculate all of the inter-atomic distances -> automatically sorted in the 1-D mapping
    for(int jj=0; jj<this->get_nr_atoms_bulk(); jj++) {
        for(int ii=jj; ii<this->get_nr_atoms_bulk(); ii++) {
            // fill a ii x jj lower triangular matrix given i<j?
            distance_ij = (this->get_bulkcoordinates(ii) - this->get_bulkcoordinates(jj)).norm(); // we need norm for our fingerprints... so cannot use norm2 for distances
            this->distance_matrix_lower_triangular.push_back(distance_ij);
        }
    }

}

/*************************************************************************************

 Generate the bulk for generating the symmetry characters

 The distance matrix is also calculated here. This makes it so that we only need to
 calculate the distances once (one of the most expensive parts of the algorithm).
 Otherwise, these would need to be re-calculated for every atom in the unit cell.

 Input:
        double max_cutoff - the maximum cutoff radius in Angstrom
                            -> this allows us to generate a list of all the atoms that we might need (this list automatically encompasses the lists for smaller cutoffs)

*************************************************************************************/
void DataFile::create_bulk(floaty max_cutoff, bool verbose) {

    if(verbose) {
        std::cout << "Generating Bulk Structure for " << this->get_nr_atoms() << " atoms" << std::endl;
    }

    this->get_copy(this->nNeighbours, max_cutoff); // set the current limits of the bulk expansion for the current cutoff

    int xc = 2*this->nNeighbours[0] + 1; // the total number of x-blocks
    int yc = 2*this->nNeighbours[1] + 1; // the total number of y-blocks per x-block
    int zc = 2*this->nNeighbours[2] + 1; // the total number of z-blocks per y-block

    // offsets for indexing the different {x,y,z}-blocks
    int xoff;
    int yoff;
    int zoff;

    this->bulkcoordinates = EigenMatrixXff::Zero(xc*yc*zc*this->get_nr_atoms(),3); // allocate the size of the matrix storing the bulk coordinates

    // add the coordinates of all the neighbouring unit cells that can lie within the cutoff
    for(int xl=-this->nNeighbours[0]; xl<this->nNeighbours[0]+1; xl++) {           // generate all x-blocks {...,-2,-1,0,+1,+2,...}

        xoff = (xl + this->nNeighbours[0]) * yc * zc * this->get_nr_atoms();        // start of the current x-block

        for(int yl=-this->nNeighbours[1]; yl<this->nNeighbours[1]+1; yl++) {       // generate all y-blocks in this x-block

            yoff = xoff + (yl + this->nNeighbours[1]) * zc * this->get_nr_atoms();  // start of the current y-block

            for(int zl=-this->nNeighbours[2]; zl<this->nNeighbours[2]+1; zl++) {   // generate all z-blocks in this y-block

                zoff = yoff + (zl + this->nNeighbours[2]) * this->get_nr_atoms();   // start of the current z-block

                for(int atom=0; atom<this->get_nr_atoms(); atom++) {                 // modify each of the atomic coordinates by applying the image offset

                    // perform the required translations along the lattice vectors to get the atomic coordinates in the image specified by {x,y,z}
                    this->bulkcoordinates.set_row3(
                        zoff+atom,
                        this->get_coordinates(atom) +
                        xl * this->get_latticevector(0) +
                        yl * this->get_latticevector(1) +
                        zl * this->get_latticevector(2)
                    );
                }
            }
        }
    }

    this->bulk_created = true; // mark the bulk as created
    this->install_nnlist();    // generate the distance matrix
}

/**
 * @brief Retrieves the atomic index in the atom list.
 *
 * @param index The position in the atom list.
 * @return The atomic meta-index corresponding to the given index.
 * @throws std::out_of_range if the index is invalid.
 */
unsigned int DataFile::get_atom(unsigned int index) const {
    if (index >= this->get_nr_atoms()) {
        std::cerr << "Error: Index out of bounds - requested atom #" << index 
                  << " out of " << this->get_nr_atoms() << "." << std::endl;
        exit(EXIT_FAILURE);
    }
    return this->atomlist[index];
}

/**
 * @brief Retrieves the chemical symbol of an atom type.
 *
 * @param index The index of the atom type.
 * @return The chemical symbol as a string.
 * @throws std::out_of_range if the index is invalid.
 */
std::string DataFile::get_chemicalSymbol(unsigned int index) const {
    if (index >= this->get_nAtomTypes()) {
        std::cerr << "Error: Index out of bounds - requested atom type #" << index 
                  << " out of " << this->get_nAtomTypes() << "." << std::endl;
        exit(EXIT_FAILURE);
    }
    return this->chemicalSymbols[index];
}

/**
 * @brief Retrieves the chemical symbol of an atom based on its position in the atom list.
 *
 * @param index The atom index.
 * @return The corresponding chemical symbol.
 */
std::string DataFile::get_element(unsigned int index) const {
    return this->get_chemicalSymbol(this->get_atom(index));
}

/**
 * @brief Retrieves a specific lattice vector.
 *
 * @param index The lattice vector index (0, 1, or 2).
 * @return The lattice vector as a row vector.
 * @throws std::out_of_range if the index is invalid.
 */
EigenRowVector3ff DataFile::get_latticevector(int index) const {
    if (index >= 3) {
        std::cerr << "Error: Index out of bounds - requested lattice vector #" << index 
                  << " out of 3." << std::endl;
        exit(EXIT_FAILURE);
    }
    return this->lattice.get_row3(index);
}

/**
 * @brief Retrieves the Cartesian coordinates of a specific atom.
 *
 * @param index The atom index.
 * @return The (x, y, z) coordinates as a row vector.
 * @throws std::out_of_range if the index is invalid.
 */
EigenRowVector3ff DataFile::get_coordinates(int index) const {
    if (index >= this->get_nr_atoms()) {
        std::cerr << "Error: Index out of bounds - requested coordinates for atom #" << index 
                  << " out of " << this->get_nr_atoms() << "." << std::endl;
        exit(EXIT_FAILURE);
    }
    return this->coordinates.get_row3(index);
}

/**
 * @brief Retrieves the Cartesian coordinates of a bulk atom.
 *
 * @param index The bulk atom index.
 * @return The (x, y, z) coordinates as a row vector.
 * @throws std::out_of_range if the index is invalid.
 */
EigenRowVector3ff DataFile::get_bulkcoordinates(int index) const {
    if (index >= this->get_nr_atoms_bulk()) {
        std::cerr << "Error: Index out of bounds - requested coordinates for bulk-atom #" << index 
                  << " out of " << this->get_nr_atoms_bulk() << "." << std::endl;
        exit(EXIT_FAILURE);
    }
    return this->bulkcoordinates.get_row3(index);
}

/**
 * @brief Retrieves the distance between two atoms using a lower triangular distance matrix.
 *
 * @param i The first atom index.
 * @param j The second atom index.
 * @return The distance between atoms i and j.
 * @throws std::out_of_range if indices are out of bounds.
 */
floaty DataFile::get_distance(int i, int j) const {
    if (i >= this->get_nr_atoms_bulk() || j >= this->get_nr_atoms_bulk()) {
        std::cerr << "Error: Distance Matrix - Index out of bounds. Requested elements [" 
                  << i << "] and [" << j << "] with max bound [" 
                  << this->get_nr_atoms_bulk() << "]." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Lower triangular matrix storage: i > j ensures minimal memory usage
    if (i > j) {
        return this->distance_matrix_lower_triangular[i + (this->get_nr_atoms_bulk() - 1) * j - (j * (j - 1)) / 2];
    } else {
        return this->distance_matrix_lower_triangular[j + (this->get_nr_atoms_bulk() - 1) * i - (i * (i - 1)) / 2];
    }
}

/**
 * @brief Determines the number of periodic images required for a given cutoff distance.
 *
 * @param copies Reference to an array of size 3 to store the number of periodic images in each direction.
 * @param cutoff The cutoff distance for determining periodicity.
 */
void DataFile::get_copy(int (&copies)[3], floaty cutoff) const {
    if (this->periodicity < 0 || this->periodicity > 3) {
        std::cerr << "Error: Invalid periodicity value " << this->periodicity << " (allowed: 0..3)." << std::endl;
        exit(EXIT_FAILURE);
    }

    floaty lattice_length;

    // Calculate periodic copies for each lattice direction
    for (unsigned int vector = 0; vector < this->periodicity; vector++) {
        lattice_length = this->get_latticevector(vector).norm();
        copies[vector] = static_cast<int>(CFLOATY_max(std::ceil(cutoff / lattice_length)));
    }

    // Set remaining directions to non-periodic (0)
    for (int vector = this->periodicity; vector < 3; vector++) {
        copies[vector] = 0;
    }
}

/**
 * @brief Computes the BPNN smooth radial cut-off function.
 *
 * This function calculates the smooth radial cut-off value based on the
 * given distance and cutoff radius. It is used in 2-body and 3-body
 * descriptors such as G1, G2, G3, G3f, G4, and G5.
 *
 * @param distance Distance between a diad in Angstroms.
 * @param cutoff   Specified cutoff radius in Angstroms.
 * @return The computed radial cut-off value.
 */
double DataFile::Gij_radial_valid(double distance, double cutoff) {
    return 0.5 * (std::cos(M_PI * distance / cutoff) + 1.0);
}

/**
 * @brief Computes the BPNN smooth radial cut-off function.
 *
 * This function applies a smooth radial cut-off based on the given distance
 * and cutoff radius. It is used in the 2-body and 3-body descriptors such
 * as G1, G2, G3, G3f, G4, and G5.
 *
 * @param distance Distance between a diad in Angstroms.
 * @param cutoff   Specified cutoff radius in Angstroms.
 * @return The computed radial cut-off value. Returns 0.0 if the distance
 *         is greater than or equal to the cutoff.
 */
double DataFile::Gij_radial(double distance, double cutoff) {
    return (distance < cutoff) ? Gij_radial_valid(distance, cutoff) : 0.0;
}
