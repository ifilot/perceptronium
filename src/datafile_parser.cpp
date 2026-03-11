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

#include "datafile_parser.h"

std::vector<DataEntry> DataFileParser::read_multiple_from_binary(const std::string& input_file) {
    std::vector<DataEntry> data_list;

    std::ifstream bin_file(input_file, std::ios::binary);
    if (!bin_file) {
        std::cerr << "Error: Unable to open file " << input_file << std::endl;
        return data_list;
    }

    // Read number of entries
    uint64_t num_entries;
    bin_file.read(reinterpret_cast<char*>(&num_entries), sizeof(num_entries));

    for (uint64_t i = 0; i < num_entries; ++i) {
        DataEntry data;

        // Read Energy (double)
        bin_file.read(reinterpret_cast<char*>(&data.Energy), sizeof(data.Energy));

        // Read Periodicity (uint64_t)
        bin_file.read(reinterpret_cast<char*>(&data.Periodicity), sizeof(data.Periodicity));

        // Read Number of Bulk Atoms (uint64_t)
        bin_file.read(reinterpret_cast<char*>(&data.NumBulkAtoms), sizeof(data.NumBulkAtoms));

        // Read Lattice (3x3 float matrix)
        for (int row = 0; row < 3; ++row) {
            bin_file.read(reinterpret_cast<char*>(data.Lattice[row].data()), sizeof(float) * 3);
        }

        // Read Atomlist Length and Atomlist
        uint64_t atomlist_length;
        bin_file.read(reinterpret_cast<char*>(&atomlist_length), sizeof(atomlist_length));
        data.Atomlist.resize(atomlist_length);
        bin_file.read(reinterpret_cast<char*>(data.Atomlist.data()), atomlist_length);

        // Read Chemical Symbols Length and Chemical Symbols
        uint64_t chem_symbols_length;
        bin_file.read(reinterpret_cast<char*>(&chem_symbols_length), sizeof(chem_symbols_length));
        data.ChemicalSymbols.resize(chem_symbols_length);
        bin_file.read(reinterpret_cast<char*>(data.ChemicalSymbols.data()), chem_symbols_length);

        // Read Coordinates
        uint64_t num_coordinates;
        bin_file.read(reinterpret_cast<char*>(&num_coordinates), sizeof(num_coordinates));
        data.Coordinates.resize(num_coordinates);
        for (uint64_t j = 0; j < num_coordinates; ++j) {
            bin_file.read(reinterpret_cast<char*>(data.Coordinates[j].data()), sizeof(float) * 3);
        }

        data_list.push_back(std::move(data));
    }

    return data_list;
}