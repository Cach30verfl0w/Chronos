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
#include <cxxopts.hpp>
#include <kstd/libc.hpp>

auto main(int argc, char* argv[]) -> int {
    cxxopts::Options options {"Chronos Debugger", "Debugger for Windows and Linux"};
    // clang-format off
    options.add_options("general")
            ("v,verbose", "Enable verbose printing", cxxopts::value<bool>()->default_value("false"))
            ("f,file", "Debug target file", cxxopts::value<std::string>()->default_value(""))
            ("h,help", "Print help");
    // clang-format on

    const auto result = options.parse(argc, argv);
    if(result.count("help")) {
        kstd::libc::printf("%s\n", options.help().c_str());
        return EXIT_SUCCESS;
    }

    // TODO: Redo command stuff
    return EXIT_SUCCESS;
}