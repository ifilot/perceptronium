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

#include "progress.h"

/**
 * @brief Displays a real-time progress bar in the console.
 *
 * This function continuously updates a progress bar based on the value of 
 * the atomic `progress` variable, relative to the total expected count.
 * It runs in a loop until progress reaches or exceeds `total`.
 *
 * @param progress A reference to an `std::atomic<int>` tracking the current progress.
 * @param total The total count to reach 100% completion.
 *
 * @note This function is designed to be run in a separate thread.
 *       The progress bar updates every 100 milliseconds.
 */
void show_progress(std::atomic<int>& progress, int total) {
    while (progress < total) {
        int percent = (100 * progress) / total;
        std::cout << "\rProgress: [";
        int bar_width = 50;
        int pos = (bar_width * percent) / 100;
        for (int i = 0; i < bar_width; i++) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << percent << "%";
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "\rProgress: [==================================================] 100%" << std::endl;
}