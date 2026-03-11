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

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include <ranges>

#include "atomlist.h"
#include "floats.h"

// Structs for returning multiple parameters in a single call
struct G2Params {
    floaty eta;
    floaty Rs;
};

struct G4Params {
    floaty eta;
    floaty zeta;
    double lambda;
};

struct G5Params {
    floaty eta;
    floaty zeta;
    double lambda;
};

/**
 * @brief Enum class representing symmetry function block types.
 */
enum class BlockType {
    G0, G1, G2, G3f, G3, G4, G5, Unknown
};

/**
 * File containing symmetry function directives
 */
class RequestFile {

private:
    // Symmetry function parameters for Behler-Parrinello Neural Networks (BPNN)

    // One-body symmetry function (Identity fingerprint)
    std::vector<floaty> G0;    // Scaling factor beta

    // Two-body symmetry function (Radial cutoff)
    std::vector<floaty> G1;    // Cutoff radius Rc

    // Two-body Gaussian symmetry function (G2)
    std::vector<floaty> G2_eta; // Width parameter (controls Gaussian spread)
    std::vector<floaty> G2_Rs;  // Radial shift (controls peak position)

    // Two-body Fourier symmetry functions
    std::vector<floaty> G3_kappa;  // Frequency parameter for cosine-based symmetry function (G3)
    std::vector<floaty> G3f_kappa; // Frequency parameter for rectified cosine symmetry function (G3f)

    // Three-body angular symmetry function (G4)
    std::vector<floaty> G4_eta;    // Gaussian width parameter
    std::vector<floaty> G4_zeta;   // Angular resolution (higher zeta sharpens angular sensitivity)
    std::vector<floaty> G4_lambda; // Inversion factor (+1 or -1)

    // Three-body broad angular symmetry function (G5)
    std::vector<floaty> G5_eta;    // Gaussian width parameter
    std::vector<floaty> G5_zeta;   // Angular resolution
    std::vector<floaty> G5_lambda; // Inversion factor (+1 or -1)

    // Total number of symmetry functions used in feature representation
    int nr_symmetry_functions;

public:

    // Reads and parses the REQUEST file during object construction
    RequestFile(const std::string& request_path);

    /**
     * @brief Returns the number of G0 symmetry functions.
     * @return Number of G0 symmetry functions.
     */
    inline int count_G0() const {
        return this->G0.size();
    }

    /**
     * @brief Returns the number of G1 symmetry functions (cutoff radii).
     * @return Number of G1 symmetry functions.
     */
    inline int count_G1() const {
        return this->G1.size();
    }

    /**
     * @brief Returns the number of G2 symmetry functions.
     * @return Number of G2 symmetry functions.
     */
    inline int count_G2() const {
        return this->G2_eta.size();
    }

    /**
     * @brief Returns the number of G3 symmetry functions.
     * @return Number of G3 symmetry functions.
     */
    inline int count_G3() const {
        return this->G3_kappa.size();
    }

    /**
     * @brief Returns the number of G3f symmetry functions.
     * @return Number of G3f symmetry functions.
     */
    inline int count_G3f() const {
        return this->G3f_kappa.size();
    }

    /**
     * @brief Returns the number of G4 symmetry functions.
     * @return Number of G4 symmetry functions.
     */
    inline int count_G4() const {
        return this->G4_eta.size();
    }

    /**
     * @brief Returns the number of G5 symmetry functions.
     * @return Number of G5 symmetry functions.
     */
    inline int count_G5() const {
        return this->G5_eta.size();
    }

    /**
     * @brief Returns the total number of symmetry functions across all types.
     * @return Total number of symmetry functions.
     */
    inline int get_nr_symmetry_functions() const {
        return this->nr_symmetry_functions;
    }

    /**
     * @brief Returns the maximum cutoff radius used in G1 symmetry functions.
     * @note Assumes G1 values are sorted in ascending order.
     * @return Maximum cutoff radius.
     */
    inline floaty get_max_cutoff() const {
        return this->G1[this->count_G1() - 1]; // Since G1 is sorted, last element is the largest
    }

    /**
     * @brief Retrieves the beta parameter for the G0 symmetry function.
     * @param index Index of the requested G0 function.
     * @return The beta value.
     * @throws std::out_of_range if index is invalid.
     */
    floaty get_G0(unsigned int index) const;

    /**
     * @brief Retrieves the cutoff radius (Rc) for the G1 symmetry function.
     * @param index Index of the requested G1 function.
     * @return The Rc value.
     * @throws std::out_of_range if index is invalid.
     */
    floaty get_G1(unsigned int index) const;

    /**
     * @brief Retrieves the frequency parameter (kappa) for the G3 symmetry function.
     * @param index Index of the requested G3 function.
     * @return The kappa value.
     * @throws std::out_of_range if index is invalid.
     */
    floaty get_G3(unsigned int index) const;

    /**
     * @brief Retrieves the frequency parameter (kappa) for the G3f symmetry function.
     * @param index Index of the requested G3f function.
     * @return The kappa value.
     * @throws std::out_of_range if index is invalid.
     */
    floaty get_G3f(unsigned int index) const;

    /**
     * @brief Retrieves the parameters (eta, Rs) for the G2 symmetry function.
     * @param index Index of the requested G2 function.
     * @return G2Params struct containing eta and Rs.
     * @throws std::out_of_range if index is invalid.
     */
    G2Params get_G2(unsigned int index) const;

    /**
     * @brief Retrieves the parameters (eta, zeta, lambda) for the G4 symmetry function.
     * @param index Index of the requested G4 function.
     * @return G4Params struct containing eta, zeta, and lambda.
     * @throws std::out_of_range if index is invalid.
     */
    G4Params get_G4(unsigned int index) const;

    /**
     * @brief Retrieves the parameters (eta, zeta, lambda) for the G5 symmetry function.
     * @param index Index of the requested G5 function.
     * @return G5Params struct containing eta, zeta, and lambda.
     * @throws std::out_of_range if index is invalid.
     */
    G5Params get_G5(unsigned int index) const;

private:

    /**
     * @brief Rectified half-maximum Heaviside step function H(|x|, -1).
     *
     * Used for preprocessing the REQUEST file input.
     *
     * @param x Input value.
     * @return -1 if x < 0, otherwise 1.
     */
    floaty halfMaxAbsHeavyside(floaty x);

    /**
     * @brief Overloaded Heaviside step function for integer input.
     *
     * Ensures lambda values are constrained to +1 or -1.
     *
     * @param x Input integer.
     * @return -1 if x < 0, otherwise 1.
     */
    int halfMaxAbsHeavyside(int x);

    /**
     * @brief Splits a string into a vector of tokens based on whitespace.
     *
     * @param line The input string.
     * @return A vector of tokens.
     */
    std::vector<std::string_view> split_line(std::string_view line);
};
