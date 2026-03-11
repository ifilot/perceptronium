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

#include "npzwriter.h"

#include <cstdint>

/**
 * @brief Writes a NumPy-compatible .npy file into a buffer.
 */
template<typename T>
std::vector<uint8_t> NPZWriter::write_npy_to_buffer(const std::vector<T>& data, const std::vector<size_t>& shape) {
    std::ostringstream header_stream;
    std::string dtype = (std::is_same<T, float>::value) ? "<f4" :
                        (std::is_same<T, double>::value) ? "<f8" :
                        (std::is_same<T, int>::value) ? "<i4" :
                        (std::is_same<T, uint8_t>::value) ? "|u1" : "";

    if (dtype.empty()) {
        throw std::runtime_error("Unsupported data type");
    }

    // Ensure shape is correctly formatted for NumPy
    header_stream << "{'descr': '" << dtype << "', 'fortran_order': False, 'shape': (";
    for (size_t i = 0; i < shape.size(); ++i) {
        header_stream << shape[i];
        if (i < shape.size() - 1) {
            header_stream << ", "; // Multiple dimensions
        }
    }
    if (shape.size() == 1) {
        header_stream << ","; // Single-element tuple fix
    }
    header_stream << "), }";

    std::string header = header_stream.str();

    // Ensure header is a multiple of 16 bytes
    size_t header_size = header.size() + 10; // 10 bytes for magic number, version, and header length
    size_t padding = 16 - (header_size % 16);
    header += std::string(padding, ' ');

    // Compute total buffer size
    size_t total_size = 6 + 2 + 2 + header.size() + (data.size() * sizeof(T));

    // Allocate buffer with exact size
    std::vector<uint8_t> buffer;
    buffer.reserve(total_size);

    // Append magic number
    buffer.push_back(0x93);
    buffer.insert(buffer.end(), {'N', 'U', 'M', 'P', 'Y'});

    // Set NumPy format version to 1.0
    buffer.push_back(1); // Major version
    buffer.push_back(0); // Minor version

    // Store header length as little-endian uint16
    uint16_t header_len = static_cast<uint16_t>(header.size());
    buffer.push_back(static_cast<uint8_t>(header_len & 0xFF));
    buffer.push_back(static_cast<uint8_t>((header_len >> 8) & 0xFF));

    // Append header content
    buffer.insert(buffer.end(), header.begin(), header.end());

    // Append raw data
    const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(data.data());
    buffer.insert(buffer.end(), data_ptr, data_ptr + data.size() * sizeof(T));

    return buffer;
}

/**
 * @brief Adds a 1D NumPy array (vector) of floats to the .npz archive.
 */
void NPZWriter::add_float_array(const std::string& name, const std::vector<float>& data) {
    npz_content[name + ".npy"] = write_npy_to_buffer(data, { data.size() });
}

/**
 * @brief Adds a 1D NumPy array (vector) of uint8_t to the .npz archive.
 */
void NPZWriter::add_uint8_array(const std::string& name, const std::vector<uint8_t>& data) {
    npz_content[name + ".npy"] = write_npy_to_buffer(data, { data.size() });
}

/**
 * @brief Adds a 3D NumPy array (vector of matrices) to the .npz archive.
 */
void NPZWriter::add_matrix_array(const std::string& name, const std::vector<EigenMatrixXff>& matrices) {
    if (matrices.empty()) {
        throw std::runtime_error("Matrix vector is empty");
    }

    size_t num_matrices = matrices.size();
    size_t rows = matrices[0].rows();
    size_t cols = matrices[0].cols();

    std::vector<float> flattened_data;
    for (const auto& matrix : matrices) {
        for (int i = 0; i < matrix.rows(); ++i) {
            for (int j = 0; j < matrix.cols(); ++j) {
                flattened_data.push_back(matrix(i, j));
            }
        }
    }

    npz_content[name + ".npy"] = write_npy_to_buffer(flattened_data, { num_matrices, rows, cols });
}

void NPZWriter::save(const std::string& npz_filename) {
    std::ofstream zip_file(npz_filename, std::ios::binary);
    if (!zip_file) {
        throw std::runtime_error("Could not open .npz file for writing: " + npz_filename);
    }

    size_t offset = 0;  // Offset tracking
    std::ostringstream central_directory;  // Buffer for central directory

    for (const auto& [filename, data] : npz_content) {
        uLong crc = crc32(0L, Z_NULL, 0);
        crc = crc32(crc, reinterpret_cast<const Bytef*>(data.data()), data.size());

        uint16_t name_length = filename.size();
        uint32_t size = data.size();

        // Step 1: Write Local File Header
        zip_file.write("PK\x03\x04", 4);  // Local file header signature
        zip_file.put(20); zip_file.put(0);  // Version needed to extract
        zip_file.put(0); zip_file.put(0);   // General purpose bit flag
        zip_file.put(0); zip_file.put(0);   // Compression method (0 = no compression)
        zip_file.put(0); zip_file.put(0);   // Modification time
        zip_file.put(0); zip_file.put(0);   // Modification date
        zip_file.write(reinterpret_cast<char*>(&crc), 4);  // CRC-32 checksum
        zip_file.write(reinterpret_cast<char*>(&size), 4); // Compressed size
        zip_file.write(reinterpret_cast<char*>(&size), 4); // Uncompressed size
        zip_file.write(reinterpret_cast<char*>(&name_length), 2);
        zip_file.write("\x00\x00", 2);  // Extra field length (0)
        zip_file.write(filename.c_str(), name_length);
        zip_file.write(reinterpret_cast<const char*>(data.data()), size);

        // Step 2: Write Central Directory Entry
        central_directory.write("PK\x01\x02", 4);  // Central file header signature
        central_directory.put(20); central_directory.put(0);  // Version made by
        central_directory.put(20); central_directory.put(0);  // Version needed to extract
        central_directory.put(0); central_directory.put(0);   // General purpose bit flag
        central_directory.put(0); central_directory.put(0);   // Compression method
        central_directory.put(0); central_directory.put(0);   // Modification time
        central_directory.put(0); central_directory.put(0);   // Modification date
        central_directory.write(reinterpret_cast<char*>(&crc), 4);
        central_directory.write(reinterpret_cast<char*>(&size), 4);
        central_directory.write(reinterpret_cast<char*>(&size), 4);
        central_directory.write(reinterpret_cast<char*>(&name_length), 2);
        central_directory.write("\x00\x00", 2);  // Extra field length (0)
        central_directory.write("\x00\x00", 2);  // File comment length (0)
        central_directory.write("\x00\x00", 2);  // Disk number start (0)
        central_directory.write("\x00\x00", 2);  // Internal file attributes
        central_directory.write("\x00\x00\x00\x00", 4); // External file attributes
        central_directory.write(reinterpret_cast<char*>(&offset), 4);
        central_directory.write(filename.c_str(), name_length);

        offset += 30 + name_length + size;
    }

    // Step 3: Write Central Directory
    size_t central_dir_offset = offset;
    zip_file.write(central_directory.str().c_str(), central_directory.str().size());

    // Step 4: Write End of Central Directory Record
    size_t central_dir_size = central_directory.str().size();
    zip_file.write("PK\x05\x06", 4);
    zip_file.write("\x00\x00", 2);
    zip_file.write("\x00\x00", 2);
    uint16_t num_entries = npz_content.size();
    zip_file.write(reinterpret_cast<char*>(&num_entries), 2);
    zip_file.write(reinterpret_cast<char*>(&num_entries), 2);
    zip_file.write(reinterpret_cast<char*>(&central_dir_size), 4);
    zip_file.write(reinterpret_cast<char*>(&central_dir_offset), 4);
    zip_file.write("\x00\x00", 2);

    zip_file.close();
}