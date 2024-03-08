#include "chronos/debug/debug.hpp"
#include "chronos/platform/file.hpp"
#include <cxxopts.hpp>
#include <filesystem>
#include <iostream>
#include <kstd/libc.hpp>
#include <kstd/option.hpp>
#include <kstd/safe_alloc.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

#ifdef BUILD_DEBUG
#define VERBOSE_LEVEL spdlog::level::trace
#else
#define VERBOSE_LEVEL spdlog::level::debug
#endif

// TODO: Read debug symbols

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
    if(magic_bytes[0] != 'M' || magic_bytes[1] != 'Z') {
        return false;
    }

    // TODO: Add PE Header validation
    return true;
#else
    constexpr auto elf_magic_bytes = std::array<char, 4> {0x7F, 'E', 'L', 'F'};
    return elf_magic_bytes == magic_bytes;
#endif
}

auto open_file(const std::filesystem::path& file_path) noexcept -> kstd::Result<void> {
    using namespace std::string_literals;
    if(!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) {
        return kstd::Error {fmt::format("File '{}' isn't a file or doesn't exists", file_path.string())};
    }

    auto file_result = kstd::try_construct<chronos::platform::File>(file_path, chronos::platform::FileFlags::READ);
    if(file_result.is_error()) {
        return kstd::Error {fmt::format("Unable to openfile: {}", file_result.get_error())};
    }

    const auto map_memory_result = file_result->map_into_memory();
    if(map_memory_result.is_error()) {
        return kstd::Error {fmt::format("{}", map_memory_result.get_error())};
    }

    if(map_memory_result->get_size() < 4) {
        return kstd::Error {"Unable to a map file into memory: The file content is too tiny"s};
    }

    const auto mapped_memory_ptr = *map_memory_result.get();
    std::array<char, 4> magic_bytes {};
    std::copy(mapped_memory_ptr, mapped_memory_ptr + 4, magic_bytes.begin());
    if(!are_magic_bytes_valid(magic_bytes)) {
        return kstd::Error {"Unable to change file: The provided file isn't a debuggable executable"s};
    }

    return {};
}

auto main(chronos::i32 argc, char** argv) -> chronos::i32 {
    // clang-format off
    auto options = cxxopts::Options {"Chronos", "Multi-platform debugger"};
    options.add_options()
            ("f,file", "Target file for debug", cxxopts::value<std::string>()->default_value(""))
            ("p,port", "Port for debug server", cxxopts::value<chronos::u16>()->default_value("0"))
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

    if(result.count("help")) {
        kstd::libc::printf("%s\n", options.help().c_str());
        return EXIT_SUCCESS;
    }

    // Set current file path when argument was set
    // clang-format off
    kstd::Option<std::filesystem::path> current_file_path =
            result.count("file") > 0 ? kstd::Option {result["file"].as<std::string>()}.map([](auto value) {
                return std::filesystem::path {value};
            }) : kstd::Option<std::filesystem::path> {};
    // clang-format on
    if(current_file_path) {
        if(const auto file_result = open_file(*current_file_path); file_result.is_error()) {
            SPDLOG_ERROR("{}", file_result.get_error());
            return EXIT_FAILURE;
        }
    }

    // Create debugger instance
    auto debugger = chronos::debug::ChronosDebugger {};

    // Command Prompt
    std::string line;
    std::fputs("(Chronos)> ", stdout);
    while(std::getline(std::cin, line)) {
        std::string command;
        std::istringstream(line) >> command;
        auto args = split_string(line, ' ');

        // TODO: Support for PIE
        // TODO: Commands: continue, breakpoint [address, name+index (Address)], read registers,
        //  read stack/memory, read function assembly (C code?)

        // Handle commands
        const auto command_name = args[0];
        if(std::equal(command_name.cbegin(), command_name.cend(), "quit")) {
            break;
        }
        else if(std::equal(command_name.cbegin(), command_name.cend(), "run")) {
            if(args.size() != 1) {
                SPDLOG_ERROR("Invalid usage, please use: run");
                goto end;
            }

            if(current_file_path.is_empty()) {
                SPDLOG_ERROR("Please set debugee file!");
                goto end;
            }

            SPDLOG_INFO("Starting debugger...");
            const auto a = std::vector<std::string> {};
            if(const auto run_result = debugger.run(current_file_path.get(), a); run_result.is_error()) {
                SPDLOG_ERROR("{}", run_result.get_error());
                goto end;
            }
        }
        else if(std::equal(command_name.cbegin(), command_name.cend(), "continue")) {
            if(const auto continue_result = debugger.continue_execution(); continue_result.is_error()) {
                SPDLOG_ERROR("{}", continue_result.get_error());
                goto end;
            }
        }
        else if(std::equal(command_name.cbegin(), command_name.cend(), "break")) {
            const intptr_t addr_value = std::stol(std::string {args[1]}, nullptr, 16);
            if(const auto break_result = debugger.add_breakpoint(addr_value); break_result.is_error()) {
                SPDLOG_ERROR("{}", break_result.get_error());
                goto end;
            }

#ifdef CPU_64_BIT
            SPDLOG_INFO("Set breakpoint at 0x{:016X}", addr_value);
#else
            SPDLOG_INFO("Set breakpoint at 0x{:08X}", addr_value);
#endif
        }
        else if(std::equal(command_name.cbegin(), command_name.cend(), "unbreak")) {
            const intptr_t addr_value = std::stol(std::string {args[1]}, nullptr, 16);
            if(const auto unbreak_result = debugger.remove_breakpoint(addr_value); unbreak_result.is_error()) {
                SPDLOG_ERROR("{}", unbreak_result.get_error());
                goto end;
            }

#ifdef CPU_64_BIT
            SPDLOG_INFO("Removed breakpoint from 0x{:016X}", addr_value);
#else
            SPDLOG_INFO("Removed breakpoint from 0x{:08X}", addr_value);
#endif
        }
        else if(std::equal(command_name.cbegin(), command_name.cend(), "breakpoints")) {
            const auto& breakpoints = debugger.get_breakpoints();
            if (breakpoints.empty()) {
                SPDLOG_INFO("No breakpoints are set");
                goto end;
            }

            SPDLOG_INFO("List of current breakpoints");
            for (auto i = 0; i < breakpoints.size(); i++) {
                auto current_value = breakpoints.begin();
                std::advance(current_value, i);

#ifdef CPU_64_BIT
                SPDLOG_INFO("{}: 0x{:016X}", i, current_value->first);
#else
                SPDLOG_INFO("{}: 0x{:08X}", i, current_value->first);
#endif
            }
        }
        else if(std::equal(command_name.cbegin(), command_name.cend(), "file")) {
            if(args.size() != 2) {
                SPDLOG_ERROR("Invalid usage, please use: file <path to file>");
                goto end;
            }

            const auto executable_path = args.at(1);
            if(!std::filesystem::exists(executable_path) || !std::filesystem::is_regular_file(executable_path)) {
                SPDLOG_ERROR("File '{}' isn't a file or doesn't exists", executable_path);
                goto end;
            }

            // Open file and map file content into memory
            if(const auto file_result = open_file(executable_path); file_result.is_error()) {
                SPDLOG_ERROR("{}", file_result.get_error());
                goto end;
            }

            // Finish
            current_file_path = {executable_path};
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
