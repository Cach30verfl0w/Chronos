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

#ifdef PLATFORM_LINUX
#include "chronos/debug/debug.hpp"

namespace chronos::debug {
    Breakpoint::Breakpoint(chronos::debug::ProcessId process_id, std::intptr_t address) noexcept ://NOLINT
            _process_id {process_id},
            _address {address},
            _enabled {false},
            _saved_data {} {
    }

    auto Breakpoint::enable() noexcept -> kstd::Result<void> {
        // Read the data at the specified address and save instruction data
        errno = 0;
        const auto data = ::ptrace(PTRACE_PEEKDATA, _process_id, _address, nullptr); // TODO: Fail
        if(data < 0 && errno != 0) {
            return kstd::Error {fmt::format("Unable to enable breakpoint: {}", platform::get_last_error())};
        }
        _saved_data = static_cast<u8>(data & 0xFF);

        // Replace instruction at address with interrupt instruction
        const u64 software_interrupt_instruction = 0xCC;
        if(::ptrace(PTRACE_POKEDATA, _process_id, _address, (data & ~0xFF) | software_interrupt_instruction) < 0) {
            return kstd::Error {fmt::format("Unable to enable breakpoint: {}", platform::get_last_error())};
        }

        // Set breakpoint enabled
        _enabled = true;
        return {};
    }

    auto Breakpoint::disable() noexcept -> kstd::Result<void> {
        // Read the data at the specified address

        errno = 0;
        const auto data = ::ptrace(PTRACE_PEEKDATA, _process_id, _address, nullptr);
        if(data < 0 && errno != 0) {
            return kstd::Error {fmt::format("Unable to disable breakpoint: {}", platform::get_last_error())};
        }

        // Remove interrupt instruction and insert restored data
        if(::ptrace(PTRACE_POKEDATA, _process_id, _address, (data & ~0xFF) | _saved_data) < 0) {
            return kstd::Error {fmt::format("Unable to disable breakpoint: {}", platform::get_last_error())};
        }

        // Set breakpoint disabled
        _enabled = false;
        return {};
    }

    // TODO: Add ability to select file and set breakpoint, then run it
    auto ChronosDebugger::run(const std::filesystem::path& file, const std::vector<std::string>& args) noexcept
            -> kstd::Result<void> {
        using namespace std::string_literals;
        if(is_running()) {
            return kstd::Error {"Unable to run debugger: Another process is already running"s};
        }

        const auto child_process_id = ::fork();
        if(child_process_id == 0) {
            // Disable ASLR
            ::personality(ADDR_NO_RANDOMIZE);

            // Tell parent process that this process can be debugged
            if(::ptrace(PT_TRACE_ME, 0, nullptr, nullptr) < 0) {
                SPDLOG_ERROR("Unable to trace process: {}", platform::get_last_error());
                exit(-1);
            }

            // Run process
            std::string arguments {};
            arguments.append(file);
            for(const auto& arg : args) {
                arguments.append(" ");
                arguments.append(arg);
            }

            execl(file.c_str(), arguments.c_str(), nullptr);
        } else {
            _running_process_id = {child_process_id};
        }
        return {};
    }

    auto ChronosDebugger::continue_execution() const noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        if(!is_running()) {
            return kstd::Error {"Unable to continue execution: No process is running"s};
        }

        if(::ptrace(PTRACE_CONT, *_running_process_id, nullptr, nullptr) < 0) {
            return kstd::Error {fmt::format("Unable to continue execution: {}", platform::get_last_error())};
        }

        if (const auto wait_result = wait_for_signal(); wait_result.is_error()) {
            return kstd::Error {wait_result.get_error()};
        }

        return {};
    }

    auto ChronosDebugger::add_breakpoint(std::intptr_t address) noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        if(!is_running()) {
            return kstd::Error {"Unable to continue execution: No process is running"s};
        }

        if(_breakpoints.contains(address)) {
            return kstd::Error {"Unable to set breakpoint: Breakpoint is already set"s};
        }

        // Create and set breakpoint
        Breakpoint breakpoint {*_running_process_id, address};
        if (const auto enable_result = breakpoint.enable(); enable_result.is_error()) {
            return kstd::Error {enable_result.get_error()};
        }
        _breakpoints.insert(std::make_pair(address, breakpoint));
        return {};
    }

    auto ChronosDebugger::remove_breakpoint(std::intptr_t address) noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        if(!is_running()) {
            return kstd::Error {"Unable to continue execution: No process is running"s};
        }

        const auto breakpoint = _breakpoints.find(address);
        if(breakpoint == _breakpoints.cend()) {
            return kstd::Error {"Unable to set breakpoint: Breakpoint is not set"s};
        }

        if (const auto disable_result = breakpoint->second.disable(); disable_result.is_error()) {
            return kstd::Error {disable_result.get_error()};
        }
        _breakpoints.erase(address);
        return {};
    }

    auto ChronosDebugger::wait_for_signal() const noexcept -> kstd::Result<void> {
        // Wait for pause execution (to acquire signals like SIGSEGV)
        int wait_status;
        if (::waitpid(*_running_process_id, &wait_status, 0) < 0) {
            return kstd::Error {fmt::format("Unable to wait for signal: {}", platform::get_last_error())};
        }

        // Acquire info about signal
        siginfo_t info;
        if (::ptrace(PTRACE_GETSIGINFO, *_running_process_id, nullptr, &info) < 0) {
            return kstd::Error {fmt::format("Unable to wait signal: {}", platform::get_last_error())};
        }

        switch (info.si_signo) {
            case SIGTRAP:
                // TODO: Handle different trap types
                SPDLOG_INFO("Hit breakpoint"); // TODO: Print breakpoint address (source file etc. when available)
                break;
            case SIGSEGV:
                SPDLOG_INFO("Got SIGSEGV signal. Reason: {}", info.si_code);
                break;
            default:
                SPDLOG_INFO("Got signal {} by application", strsignal(info.si_signo));
                break;
        }
        return {};
    }

    auto ChronosDebugger::get_breakpoints() const noexcept -> const std::unordered_map<std::intptr_t, Breakpoint>& {
        return _breakpoints;
    }
}// namespace chronos::debug
#endif