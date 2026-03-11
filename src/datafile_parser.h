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

#include "datafile.h"
#include "data_entry.h"

class DataFileParser {

public:
    /**
     * @brief Construct a parser utility instance.
     */
    DataFileParser() {}

    /**
     * @brief Read multiple serialized data entries from a package file.
     *
     * @param input_file Path to the binary package (`.pkg`) file.
     * @return Parsed list of entries in the same order as stored in the file.
     */
    static std::vector<DataEntry> read_multiple_from_binary(const std::string& input_file);
};
