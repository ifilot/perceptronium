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

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <charconv>
#include <stdexcept>

#include "requestfile.h"

/**
 * @brief Constructs a RequestFile object by parsing the given file.
 * @param request_path Path to the REQUEST file.
 * @throws std::runtime_error if the file cannot be opened.
 */
RequestFile::RequestFile(const std::string& request_path) {
    // Open the REQUEST file
    std::ifstream file(request_path);
    if (!file) {
        throw std::runtime_error("Error: REQUEST file [" + request_path + "] does not exist.");
    }

    // Read file contents into memory
    std::vector<std::string> lines;
    std::string line;

    // Skip file header
    std::getline(file, line);

    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close(); // Close file immediately after reading

    // Mapping from block labels to BlockType enum
    const std::unordered_map<std::string_view, BlockType> block_map = {
        {"G0", BlockType::G0}, {"G1", BlockType::G1}, {"G2", BlockType::G2},
        {"G3f", BlockType::G3f}, {"G3", BlockType::G3}, {"G4", BlockType::G4},
        {"G5", BlockType::G5}
    };

    // Symmetry function parsing
    bool readflag = false;
    BlockType blocktype = BlockType::Unknown;

    // Process lines
    for (const auto& line : lines) {
        std::string_view line_view = line;  // Use string_view for efficient parsing

        if (line_view.empty()) {
            readflag = false; // End current block
            continue;
        }

        // Identify symmetry block type using `starts_with()`
        if (!readflag) {
            for (const auto& [key, value] : block_map) {
                if (line_view.starts_with(key)) {
                    blocktype = value;
                    readflag = true;
                    break;
                }
            }
            if (!readflag) continue; // Skip lines without relevant blocks
        } else { // Parse block contents
            auto tokens = this->split_line(line_view);

            try {
                switch (blocktype) {
                    case BlockType::G0: { // G0
                        double value = std::stod(std::string(tokens.at(0)));
                        this->G0.push_back(this->halfMaxAbsHeavyside(value) * CFLOATY_min(std::abs(value)));
                        break;
                    }
                    case BlockType::G1: { // G1
                        this->G1.push_back(std::stod(std::string(tokens.at(0))));
                        break;
                    }
                    case BlockType::G2: { // G2
                        if (tokens.size() < 2) throw std::runtime_error("Invalid G2 format");
                        this->G2_eta.push_back(std::stod(std::string(tokens.at(0))));
                        this->G2_Rs.push_back(std::stod(std::string(tokens.at(1))));
                        break;
                    }
                    case BlockType::G3f: { // G3f
                        this->G3f_kappa.push_back(std::stod(std::string(tokens.at(0))));
                        break;
                    }
                    case BlockType::G3: { // G3
                        this->G3_kappa.push_back(std::stod(std::string(tokens.at(0))));
                        break;
                    }
                    case BlockType::G4: { // G4
                        if (tokens.size() < 3) throw std::runtime_error("Invalid G4 format");
                        this->G4_eta.push_back(std::stod(std::string(tokens.at(0))));
                        this->G4_zeta.push_back(std::stod(std::string(tokens.at(1))));
                        this->G4_lambda.push_back(this->halfMaxAbsHeavyside(std::stoi(std::string(tokens.at(2)))));
                        break;
                    }
                    case BlockType::G5: { // G5
                        if (tokens.size() < 3) throw std::runtime_error("Invalid G5 format");
                        this->G5_eta.push_back(std::stod(std::string(tokens.at(0))));
                        this->G5_zeta.push_back(std::stod(std::string(tokens.at(1))));
                        this->G5_lambda.push_back(this->halfMaxAbsHeavyside(std::stoi(std::string(tokens.at(2)))));
                        break;
                    }
                    default:
                        throw std::runtime_error("Unknown blocktype");
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing line: " << line << " - " << e.what() << std::endl;
            }
        }
    }

    // Compute total number of symmetry functions
    this->nr_symmetry_functions = this->count_G0() +
                                  this->count_G1() * (1 + this->count_G2() + this->count_G3() +
                                                      this->count_G3f() + this->count_G4() + this->count_G5());

    // Sort cutoffs from lowest to highest
    std::sort(this->G1.begin(), this->G1.end());
}


/**
 * @brief Retrieves the beta parameter for the G0 symmetry function.
 * @param index The index of the requested G0 function.
 * @return The beta value.
 * @throws std::out_of_range if index is invalid.
 */
floaty RequestFile::get_G0(unsigned int index) const {
    if (index >= count_G0()) {
        throw std::out_of_range("Index out of bounds: requested G0 function #" + std::to_string(index));
    }
    return G0[index];
}

/**
 * @brief Retrieves the cutoff radius (Rc) for the G1 symmetry function.
 * @param index The index of the requested G1 function.
 * @return The Rc value.
 * @throws std::out_of_range if index is invalid.
 */
floaty RequestFile::get_G1(unsigned int index) const {
    if (index >= count_G1()) {
        throw std::out_of_range("Index out of bounds: requested G1 function #" + std::to_string(index));
    }
    return G1[index];
}

/**
 * @brief Retrieves the parameters (eta, Rs) for the G2 symmetry function.
 * @param index The index of the requested G2 function.
 * @return G2Params struct containing eta and Rs.
 * @throws std::out_of_range if index is invalid.
 */
G2Params RequestFile::get_G2(unsigned int index) const {
    if (index >= count_G2()) {
        throw std::out_of_range("Index out of bounds: requested G2 function #" + std::to_string(index));
    }
    return { G2_eta[index], G2_Rs[index] };
}

/**
 * @brief Retrieves the frequency parameter (kappa) for the G3 symmetry function.
 * @param index The index of the requested G3 function.
 * @return The kappa value.
 * @throws std::out_of_range if index is invalid.
 */
floaty RequestFile::get_G3(unsigned int index) const {
    if (index >= count_G3()) {
        throw std::out_of_range("Index out of bounds: requested G3 function #" + std::to_string(index));
    }
    return G3_kappa[index];
}

/**
 * @brief Retrieves the frequency parameter (kappa) for the G3f symmetry function.
 * @param index The index of the requested G3f function.
 * @return The kappa value.
 * @throws std::out_of_range if index is invalid.
 */
floaty RequestFile::get_G3f(unsigned int index) const {
    if (index >= count_G3f()) {
        throw std::out_of_range("Index out of bounds: requested G3f function #" + std::to_string(index));
    }
    return G3f_kappa[index];
}

/**
 * @brief Retrieves the parameters (eta, zeta, lambda) for the G4 symmetry function.
 * @param index The index of the requested G4 function.
 * @return G4Params struct containing eta, zeta, and lambda.
 * @throws std::out_of_range if index is invalid.
 */
G4Params RequestFile::get_G4(unsigned int index) const {
    if (index >= count_G4()) {
        throw std::out_of_range("Index out of bounds: requested G4 function #" + std::to_string(index));
    }
    return { G4_eta[index], G4_zeta[index], G4_lambda[index] };
}

/**
 * @brief Retrieves the parameters (eta, zeta, lambda) for the G5 symmetry function.
 * @param index The index of the requested G5 function.
 * @return G5Params struct containing eta, zeta, and lambda.
 * @throws std::out_of_range if index is invalid.
 */
G5Params RequestFile::get_G5(unsigned int index) const {
    if (index >= count_G5()) {
        throw std::out_of_range("Index out of bounds: requested G5 function #" + std::to_string(index));
    }
    return { G5_eta[index], G5_zeta[index], G5_lambda[index] };
}

/**
* @brief Rectified half-maximum Heaviside step function H(|x|, -1).
*
* Used for preprocessing the REQUEST file input.
*
* @param x Input value.
* @return -1 if x < 0, otherwise 1.
*/
floaty RequestFile::halfMaxAbsHeavyside(floaty x) {
   return (x < 0.0) ? -1.0 : 1.0;
}

/**
* @brief Overloaded Heaviside step function for integer input.
*
* Ensures lambda values are constrained to +1 or -1.
*
* @param x Input integer.
* @return -1 if x < 0, otherwise 1.
*/
int RequestFile::halfMaxAbsHeavyside(int x) {
   return (x < 0) ? -1 : 1;
}

/**
 * @brief Splits a string into a vector of tokens based on whitespace.
 *
 * @param line The input string.
 * @return A vector of tokens.
 */
std::vector<std::string_view> RequestFile::split_line(std::string_view line) {
    std::vector<std::string_view> tokens;
    size_t start = 0, end;

    while ((end = line.find_first_of(" \t\n", start)) != std::string_view::npos) {
        if (end > start) { 
            tokens.emplace_back(line.substr(start, end - start));
        }
        start = end + 1;
    }

    if (start < line.size()) { 
        tokens.emplace_back(line.substr(start));
    }

    return tokens;
}