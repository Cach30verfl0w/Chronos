#include "chronos/platform/file.hpp"
#include "chronos/utils.hpp"
#include <cxxopts.hpp>
#include <filesystem>
#include <iostream>
#include <kstd/libc.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

#ifdef BUILD_DEBUG
#define VERBOSE_LEVEL spdlog::level::trace
#else
#define VERBOSE_LEVEL spdlog::level::debug
#endif

auto split_string(const std::string& str, char delimiter) -> std::vector<std::string_view> {
    std::vector<std::string_view> views;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while(end != std::string::npos) {
        views.emplace_back(str.data() + start, end - start);
        start = end + 1;
        end = str.find(delimiter, start);
    }

    views.emplace_back(str.data() + start, str.size() - start);
    return views;
}

auto are_magic_bytes_valid(std::array<char, 4> magic_bytes) -> bool {
#ifdef PLATFORM_WINDOWS
    if (magic_bytes[0] != 'M' || magic_bytes[1] != 'Z') {
        return false;
    }

    // TODO: Add PE Header validation
    return true;
#else
    constexpr auto elf_magic_bytes = std::array<char, 4> { 0x7F, 'E', 'L', 'F' };
    return elf_magic_bytes == magic_bytes;
#endif
}

auto main(chronos::i32 argc, char** argv) -> chronos::i32 {
    // clang-format off
    auto options = cxxopts::Options {"Chronos", "Multi-platform debugger"};
    options.add_options()
            ("f,file", "Target file for debug", cxxopts::value<std::string>())
            ("p,port", "Port for debug server", cxxopts::value<chronos::u16>())
            ("v,verbose", "Enable verbose printing")
            ("h,help", "Print program usage");
    const auto result = options.parse(argc, argv);
    spdlog::set_level(result.count("verbose") ? VERBOSE_LEVEL : spdlog::level::info);
    // clang-format on

    // Set level and print header
    SPDLOG_INFO("Chronos v" CHRONOS_VERSION ", developed by Cach30verfl0w (Cedric Hammes)");
    SPDLOG_INFO("This project is licensed under Apache License 2.0");
    SPDLOG_INFO("Enter 'help' in terminal for help");
    kstd::libc::printf("%s", "\n");

    // Command Prompt
    std::string line;
    std::fputs("(Chronos)> ", stdout);
    while(std::getline(std::cin, line)) {
        std::string command;
        std::istringstream(line) >> command;
        auto arguments = split_string(line, ' ');
        arguments.erase(arguments.cbegin());

        // Handle commands
        if(std::equal(command.begin(), command.end(), "quit")) {
            break;
        }
        else if(std::equal(command.begin(), command.end(), "file")) {
            if(arguments.size() != 1) {
                SPDLOG_ERROR("Invalid usage, please use: file <path to file>");
                goto end;
            }

            const auto executable_path = arguments.at(0);
            if(!std::filesystem::exists(executable_path) || !std::filesystem::is_regular_file(executable_path)) {
                SPDLOG_ERROR("File '{}' isn't a file or doesn't exists", executable_path);
                goto end;
            }

            // Open file and map file content into memory
            const auto file = chronos::platform::File {executable_path, chronos::platform::FileFlags::READ};
            const auto map_memory_result = file.map_into_memory();
            if(map_memory_result.is_error()) {
                SPDLOG_ERROR("Unable to map a file into memory: {}", map_memory_result.get_error());
                goto end;
            }

            if(map_memory_result.get().get_size() < 4) {
                SPDLOG_ERROR("Unable to a map file into memory: The file content is too tiny");
                goto end;
            }

            // Read magic bytes
            const auto mapped_memory_ptr = *map_memory_result.get();
            std::array<char, 4> magic_bytes {};
            std::copy(mapped_memory_ptr, mapped_memory_ptr + 4, magic_bytes.begin());
            if (!are_magic_bytes_valid(magic_bytes)) {
                SPDLOG_ERROR("Unable to change file: The provided file isn't a debuggable executable");
                goto end;
            }

            // Finish
            // TODO: Read debug symbols
            SPDLOG_INFO("Successfully changed file to '{}'", executable_path);
        }
        else {
            SPDLOG_INFO("Command '{}' not found, please use 'help' for help", command);
        }

    end:
        kstd::libc::printf("%s", "\n");
        std::fputs("(Chronos)> ", stdout);
    }
    return EXIT_SUCCESS;
}
