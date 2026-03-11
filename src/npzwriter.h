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

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <cstdint>
#include <cstddef>
#include <zlib.h>  // Required for ZIP compression

#include "floats.h"

/**
 * @brief Class for writing NumPy-compatible .npz files.
 */
class NPZWriter {
private:
    std::map<std::string, std::vector<uint8_t>> npz_content;  // Stores .npy data before saving

    /**
     * @brief Writes a NumPy-compatible .npy file into a buffer.
     * 
     * @tparam T Data type (float, double, int, etc.).
     * @param data Vector containing the array data.
     * @param shape Vector representing the shape of the array (e.g., {3, 3} for a 3x3 matrix).
     * @return std::vector<uint8_t> The binary contents of the .npy file.
     */
    template<typename T>
    std::vector<uint8_t> write_npy_to_buffer(const std::vector<T>& data, const std::vector<size_t>& shape);

public:
    /**
     * @brief Adds a 1D NumPy array (vector) to the .npz archive.
     * 
     * @param name The variable name for NumPy.
     * @param data Vector of float values.
     */
    void add_float_array(const std::string& name, const std::vector<float>& data);

    /**
     * @brief Adds a 1D NumPy array (vector) of uint8_t to the .npz archive.
     *
     * @param name The variable name for NumPy.
     * @param data Vector of uint8_t values.
     */
    void add_uint8_array(const std::string& name, const std::vector<uint8_t>& data);

    /**
     * @brief Adds a 3D NumPy array (vector of matrices) to the .npz archive.
     * 
     * @param name The variable name for NumPy.
     * @param matrices Vector of Eigen matrices.
     */
    void add_matrix_array(const std::string& name, const std::vector<EigenMatrixXff>& matrices);

    /**
     * @brief Saves the stored arrays as a .npz file.
     * 
     * @param npz_filename Name of the output .npz file.
     */
    void save(const std::string& npz_filename);
};
