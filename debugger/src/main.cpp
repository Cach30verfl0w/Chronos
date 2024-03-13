//  Copyright 2024 Cach30verfl0w
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

/**
 * @author Cedric Hammes
 * @since  09/03/2024
 */
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

void readMemoryMaps(int pid) {
    std::ifstream file("/proc/" + std::to_string(pid) + "/status");
    std::string line;
    while(std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string name, value;

        lineStream >> name >> value;
        name = name.substr(0, name.length() - 1);
        std::cout << "Name: " << name << std::endl;
        std::cout << "Value: " << value << std::endl;
        std::cout << std::endl;
    }

    file.close();
}

auto main(int argc, char* argv[]) -> int {
    int pid = getpid();
    readMemoryMaps(pid);
    return EXIT_SUCCESS;
}