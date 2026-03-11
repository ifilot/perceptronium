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
#include <string>
#include <unordered_map>
#include "floats.h" // floaty

/**
 * @brief Get the atomic number corresponding to the chemical symbol.
 *
 * @param chemicalSymbol The symbol for the element you want the atomic number
 * for (symbols as in VASP POSCAR).
 * @return unsigned int The atomic number corresponding to the element specified
 * by chemical symbol.
 */
static std::unordered_map<std::string,unsigned int> periodicTable = {

    // row 1
    {"H", 1},   // Hydrogen
    {"He",2},   // Helium

    // row 2
    {"Li",3},   // Lithium
    {"Be",4},   // Beryllium
    {"B", 5},   // Boron
    {"C", 6},   // Carbon
    {"N", 7},   // Nitrogen
    {"O", 8},   // Oxygen
    {"F", 9},   // Fluorine
    {"Ne",10},  // Neon

    // row 3
    {"Na",11},  // Natrium
    {"Mg",12},  // Magnesium
    {"Al",13},  // Aluminium
    {"Si",14},  // Silicon
    {"P", 15},  // Phosphorus
    {"S", 16},  // Sulfur
    {"Cl",17},  // Chlorine
    {"Ar",18},  // Argon

    // row 4
    {"K", 19},  // Kalium
    {"Ca",20},  // Calcium
    {"Sc",21},  // Scandium
    {"Ti",22},  // Titanium
    {"V", 23},  // Vanadium
    {"Cr",24},  // Chromium
    {"Mn",25},  // Manganese
    {"Fe",26},  // Ferrum
    {"Co",27},  // Cobalt
    {"Ni",28},  // Nickel
    {"Cu",29},  // Cuprum
    {"Zn",30},  // Zinc
    {"Ga",31},  // Gallium
    {"Ge",32},  // Germanium
    {"As",33},  // Arsenicum
    {"Se",34},  // Selenium
    {"Br",35},  // Bromine
    {"Kr",36},  // Krypton

    // row 5
    {"Rb",37},  // Rubidium
    {"Sr",38},  // Strontium
    {"Y", 39},  // Yttrium
    {"Zr",40},  // Zirconium
    {"Nb",41},  // Niobium
    {"Mo",42},  // Molybdenum
    {"Tc",43},  // Technecium
    {"Ru",44},  // Rubidium
    {"Rh",45},  // Rhodium
    {"Pd",46},  // Palladium
    {"Ag",47},  // Argentum
    {"Cd",48},  // Cadmium
    {"In",49},  // Indium
    {"Sn",50},  // Stannum
    {"Sb",51},  // Stibium
    {"Te",52},  // Tellurium
    {"I", 53},  // Iodine
    {"Xe",54},  // Xenon

    // row 6
    {"Cs",55},  // Caesium
    {"Ba",56},  // Barium
    {"La",57},  // Lanthanum
    {"Ce",58},  // Cerium
    {"Pr",59},  // Praseodynium
    {"Nd",60},  // Neodynium
    {"Pm",61},  // Promethium
    {"Sm",62},  // Samarium
    {"Eu",63},  // Europium
    {"Gd",64},  // Gadolinium
    {"Tb",65},  // Terbium
    {"Dy",66},  // Dysprosium
    {"Ho",67},  // Holmium
    {"Er",68},  // Erbium
    {"Tm",69},  // Thulium
    {"Yb",70},  // Ytterbium
    {"Lu",71},  // Lutetium
    {"Hf",72},  // Hafnium
    {"Ta",73},  // Tantalum
    {"W", 74},  // Wolfram
    {"Re",75},  // Rhenium
    {"Os",76},  // Osmium
    {"Ir",77},  // Iridium
    {"Pt",78},  // Platinum
    {"Au",79},  // Aurum
    {"Hg",80},  // Heggrum
    {"Tl",81},  // Thallium
    {"Pb",82},  // Plumbum
    {"Bi",83},  // Bismuth
    {"Po",84},  // Polonium
    {"At",85},  // Astatine
    {"Rn",86},  // Radon

    // row 7
    {"Fr",87},  // Francium
    {"Ra",88},  // Radium
    {"Ac",89},  // Actinium
    {"Th",90},  // Thorium
    {"Pa",91},  // Protactinium
    {"U", 92},  // Uranium
    {"Np",93},  // Neptunium
    {"Pu",94},  // Plutonium
    {"Am",95},  // Americium
    {"Cm",96},  // Curium
    {"Bk",97},  // Berkelium
    {"Cf",98},  // Californium
    {"Es",99},  // Einsteinium
    {"Fm",100}, // Fermium
    {"Md",101}, // Mendelevium
    {"No",102}, // Nobelium
    {"Lr",103}, // Lawrencium
    {"Rf",104}, // Rutherfordium
    {"Db",105}, // Dubnium
    {"Sg",106}, // Seaborgium
    {"Bh",107}, // Bohrium
    {"Hs",108}, // Hassium
    {"Mt",109}, // Meitnerium
    {"Ds",110}, // Darmstadtium
    {"Rg",111}, // Roentgenium
    {"Cn",112}, // Copernicium
    {"Nh",113}, // Nihonium
    {"Fl",114}, // Flerovium
    {"Mc",115}, // Moscovium
    {"Lv",116}, // Livermorium
    {"Ts",117}, // Tennessine
    {"Og",118}, // Oganesson

};

/**
 * @brief Vector containing element symbols from the periodic table.
 *
 * This vector stores the standard symbols for chemical elements, ordered by
 * atomic number. The first entry ("X") is a placeholder at index 0 to align
 * element indices with their atomic numbers (i.e., Hydrogen is at index 1,
 * Helium at 2, etc.).
 *
 * Usage:
 * - `periodicTableVector[1]` returns "H" (Hydrogen).
 * - `periodicTableVector[8]` returns "O" (Oxygen).
 * - `periodicTableVector[92]` returns "U" (Uranium).
 *
 * This vector provides a simple lookup table for element symbols in
 * computational chemistry, molecular modeling, or periodic table-related
 * applications.
 */
static const std::vector<std::string> periodicTableVector = {
       "X",  // Index 0 (Placeholder)
       "H",  "He",  "Li",  "Be",  "B",   "C",   "N",   "O",   "F",   "Ne",
       "Na", "Mg",  "Al",  "Si",  "P",   "S",   "Cl",  "Ar",  "K",   "Ca",
       "Sc", "Ti",  "V",   "Cr",  "Mn",  "Fe",  "Co",  "Ni",  "Cu",  "Zn",
       "Ga", "Ge",  "As",  "Se",  "Br",  "Kr",  "Rb",  "Sr",  "Y",   "Zr",
       "Nb", "Mo",  "Tc",  "Ru",  "Rh",  "Pd",  "Ag",  "Cd",  "In",  "Sn",
       "Sb", "Te",  "I",   "Xe",  "Cs",  "Ba",  "La",  "Ce",  "Pr",  "Nd",
       "Pm", "Sm",  "Eu",  "Gd",  "Tb",  "Dy",  "Ho",  "Er",  "Tm",  "Yb",
       "Lu", "Hf",  "Ta",  "W",   "Re",  "Os",  "Ir",  "Pt",  "Au",  "Hg",
       "Tl", "Pb",  "Bi",  "Po",  "At",  "Rn",  "Fr",  "Ra",  "Ac",  "Th",
       "Pa", "U",   "Np",  "Pu",  "Am",  "Cm",  "Bk",  "Cf",  "Es",  "Fm",
       "Md", "No",  "Lr",  "Rf",  "Db",  "Sg",  "Bh",  "Hs",  "Mt",  "Ds",
       "Rg", "Cn",  "Nh",  "Fl",  "Mc",  "Lv",  "Ts",  "Og"
};

/**
 * @brief Returns the atomic number for a given chemical symbol.
 *
 * This function maps an element's symbol (as used in VASP POSCAR) to its atomic
 * number.
 *
 * @param chemicalSymbol The element symbol to look up.
 * @return The atomic number corresponding to the given symbol.
 */
unsigned int get_atomnumber(const std::string& chemicalSymbol);

/**
 * @brief Returns the 1-body normalization constant
 *
 * @warning Ensure `max_atomnumber` is >= the largest value in `periodicTable`
 *          and `Isotopes` to maintain proper normalization of 1-body symmetry
 *          functions.
 *
 * @param max_atomnumber Normalization constant ensuring `atomnumber /
 * max_atomnumber ≤ 1`.
 * @return The same `max_atomnumber`, allowing accessibility across files and
 * overrides.
 */
inline double get_element_normalisation() {
    return get_atomnumber("Og");
}
