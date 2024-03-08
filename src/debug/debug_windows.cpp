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
 * @since  08/03/2024
 */

#ifdef PLATFORM_WINDOWS
#include "chronos/debug/debug.hpp"

namespace chronos::debug {
    Breakpoint::Breakpoint(chronos::debug::ProcessId process_id, std::intptr_t address) noexcept ://NOLINT
            _process_id {process_id},
            _address {address},
            _enabled {false},
            _saved_data {} {
    }

    auto Breakpoint::enable() noexcept -> kstd::Result<void> {
        // TODO: Enable breakpoint
        return {};
    }

    auto Breakpoint::disable() noexcept -> kstd::Result<void> {
        // TODO: Disable breakpoint
        return {};
    }

    auto ChronosDebugger::run(const std::filesystem::path& file, const std::vector<std::string>& args) noexcept
            -> kstd::Result<void> {
        STARTUPINFO startup_info {};
        PROCESS_INFORMATION proc_info {};
        // TODO: Add support for arguments
        if(FAILED(CreateProcess(file.string().c_str(), nullptr, nullptr, nullptr, false, DEBUG_ONLY_THIS_PROCESS,
                                nullptr, nullptr, &startup_info, &proc_info))) {
            return kstd::Error {fmt::format("Unable to run process: {}", platform::get_last_error())};
        }
        _running_process_id = proc_info.dwProcessId;
        _thread_id = proc_info.dwThreadId;

        if(const auto result = wait_for_signal(); result.is_error()) {
            return kstd::Error {result.get_error()};
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
        if(const auto enable_result = breakpoint.enable(); enable_result.is_error()) {
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

        if(const auto disable_result = breakpoint->second.disable(); disable_result.is_error()) {
            return kstd::Error {disable_result.get_error()};
        }
        _breakpoints.erase(address);
        return {};
    }

    auto ChronosDebugger::continue_execution() noexcept -> kstd::Result<void> {
        if(FAILED(ContinueDebugEvent(*_running_process_id, *_thread_id, DBG_CONTINUE))) {
            return kstd::Error {fmt::format("Unable to run process: {}", platform::get_last_error())};
        }

        if(const auto result = wait_for_signal(); result.is_error()) {
            return kstd::Error {result.get_error()};
        }
        return {};
    }

    auto ChronosDebugger::wait_for_signal() noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        if(!is_running()) {
            return kstd::Error {"Unable to continue execution: No process is running"s};
        }

        DEBUG_EVENT event {};
        while(true) {
            if(!::WaitForDebugEvent(&event, INFINITE) && event.dwProcessId == *_running_process_id) {
                continue;
            }

            if(event.dwDebugEventCode == 6 || event.dwDebugEventCode == 2 || event.dwDebugEventCode == 3 ||
               event.dwDebugEventCode == 1 || event.dwDebugEventCode == 7) {
                if(FAILED(::ContinueDebugEvent(event.dwProcessId, event.dwThreadId,
                                               event.dwDebugEventCode == 1 ? DBG_EXCEPTION_NOT_HANDLED
                                                                           : DBG_CONTINUE))) {
                    return kstd::Error {fmt::format("Unable wait for signal: {}", platform::get_last_error())};
                }
                continue;
            }

            break;
        }
        _thread_id = event.dwThreadId;

        return {};
    }
}// namespace chronos::debug
#endif