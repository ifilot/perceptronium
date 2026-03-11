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

#include "atomlist.h"

/**
 * @brief Returns the atomic number for a given chemical symbol.
 *
 * This function maps an element's symbol (as used in VASP POSCAR) to its atomic
 * number.
 *
 * @param chemicalSymbol The element symbol to look up.
 * @return The atomic number corresponding to the given symbol.
 */
unsigned int get_atomnumber(const std::string& chemicalSymbol){

    if(chemicalSymbol.empty()){
        std::cerr << "Atomlist: No input supplied to get_atomnumber." << std::endl;
        exit(EXIT_FAILURE);
    }
    else{

        // go through our map containing all elements and search for the specified chemical symbol
        std::unordered_map<std::string,unsigned int>::const_iterator it = periodicTable.find(chemicalSymbol);

        if(it == periodicTable.end()){
            std::cerr << "Atomlist: Unknown chemical symbol [" << chemicalSymbol << "]." << std::endl;
            exit(EXIT_FAILURE);
        }
        else{
            return it->second; // if found, return atomnumber
        }
    }
}