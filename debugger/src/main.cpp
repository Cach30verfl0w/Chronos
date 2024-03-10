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
#include <iostream>
#include <kstd/libc.hpp>
#include <spdlog/spdlog.h>
#include <libdebug/platform/platform.hpp>

#ifdef PLATFORM_UNIX
constexpr auto file_name = "chronos-debugger";
#else
constexpr auto file_name = "chronos-debugger.exe";
#endif

#ifdef BUILD_DEBUG
constexpr auto verbose_level = spdlog::level::trace;
#else
constexpr auto verbose_level = spdlog::level::debug;
#endif

auto main(int argc, char* argv[]) -> int {
    kstd::libc::printf("%u\n", libdebug::platform::is_fpu_present());
    cxxopts::Options options {file_name};
    // clang-format off
    options.add_options("general")
            ("v,verbose", "Enable verbose printing", cxxopts::value<bool>()->default_value("false"))
            ("f,file", "Debug target file", cxxopts::value<std::string>()->default_value(""))
            ("h,help", "Print help");
    // clang-format on

    // Parse and print header messages
    const auto result = options.parse(argc, argv);
    spdlog::set_level(result.count("verbose") ? verbose_level : spdlog::level::info);
    SPDLOG_INFO("Chronos Debugger v1.0.0 by Cach30verfl0w (Cedric Hammes)");
    SPDLOG_INFO("Source Code: https://github.com/Cach30verfl0w/Chronos");

    // Print help message
    if(result.count("help")) {
        std::string line {};
        std::stringstream help_message {options.help()};
        while(std::getline(help_message, line, '\n')) {
            SPDLOG_INFO("{}", line);
        }
        return EXIT_SUCCESS;
    }

    // TODO: Move file.hpp into libdebug

    std::fputs("(Chronos)> ", stdout);
    std::string command_line {};
    while(std::getline(std::cin, command_line)) {
        std::vector<std::string> tokens;
        std::stringstream string_stream {command_line};
        for (std::string token; std::getline(string_stream, token, ' ');) {
            tokens.push_back(token);
        }

        kstd::libc::printf("%s", "\n");
        std::fputs("(Chronos)> ", stdout);
    }
    return EXIT_SUCCESS;
}