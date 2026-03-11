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

#include <iostream>
#include <string>
#include <chrono>
#include <filesystem>
#include <tclap/CmdLine.h>

#include "config.h"

#include "progress.h"
#include "requestfile.h"
#include "datafile.h"
#include "datafile_parser.h"
#include "npzwriter.h"

int main(int argc, char* argv[]){

    try {
        // Create a TCLAP command line parser
        TCLAP::CmdLine cmd("Symmetry function generator", ' ', BPSFP_VERSION);

        // Define the required arguments
        TCLAP::ValueArg<std::string> requestFileArg("r", "request", 
            "Path to the REQUEST file", true, "", "REQUEST file path", cmd);

        TCLAP::ValueArg<std::string> dataFileArg("d", "data", 
            "Path to the PKG file", true, "", "DATA folder path", cmd);

        TCLAP::ValueArg<std::string> outputFileArg("o", "output", 
            "Path to the output file", true, "", "Output file path", cmd);

        // Parse command-line arguments
        cmd.parse(argc, argv);

        // read request file
        const auto request = RequestFile(requestFileArg.getValue());

        // create container
        std::vector<std::unique_ptr<DataFile>> data_files;

        DataFileParser dp;
        auto res = dp.read_multiple_from_binary(dataFileArg.getValue());
        for(const auto& entry : res) {
            data_files.emplace_back(DataFile::from_entry(entry));
        }

        // create progression bar
        std::atomic<int> progress(0); // Thread-safe counter
        std::thread progress_thread(show_progress, std::ref(progress), data_files.size());

        #pragma omp parallel for
        for(const auto& df : data_files) {
            df->calculate_symmetryfunctions(request);
            progress++;
        }

        // rejoin progress
        progress_thread.join();

        std::vector<float> energies;
        std::vector<EigenMatrixXff> netcoefficients;
        for(const auto& df : data_files) {
            energies.push_back(df->get_energy());
            netcoefficients.push_back(df->get_symmetry_functions());
        }

        NPZWriter npzwriter;
        npzwriter.add_float_array("energies", energies);
        npzwriter.add_matrix_array("netcoefficients", netcoefficients);
        npzwriter.add_uint8_array("elements", data_files[0]->get_atomlist());
        npzwriter.save(outputFileArg.getValue());

        std::cout << "\033[32mAll done!\033[0m" << std::endl;

        return EXIT_SUCCESS;

    } catch (TCLAP::ArgException& e) {
        std::cerr << "Error: " << e.error() << " for argument " << e.argId() << std::endl;

        return EXIT_FAILURE;
        
    }
}