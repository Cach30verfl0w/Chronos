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
 * @since  13/03/2024
 */

#ifdef PLATFORM_LINUX
#include "libdebug/process.hpp"

namespace libdebug {
    /**
     * This constructor constructs an empty breakpoint with a reference to the context and the target address of the
     * breakpoint.
     *
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    Breakpoint::Breakpoint(std::intptr_t target_address) noexcept ://NOLINT
            _address {target_address},
            _enabled {false},
            _saved_data {} {
    }

    /**
     * This function enables the breakpoint by replacing the instruction at the specified address and saving the
     * original data in this class.
     *
     * @return Void or an error
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto Breakpoint::enable(const ThreadContext& thread_context) noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        const auto thread_id = thread_context.get_thread_id();
        if(!platform::is_process_running(thread_id)) {
            return kstd::Error {"Unable to enable breakpoint: No process is running"s};
        }

        // Read the data at the specified address and save instruction data
        errno = 0;
        const auto data = ::ptrace(PTRACE_PEEKDATA, thread_id, _address, nullptr);
        if(data < 0 && errno != 0) {
            return kstd::Error {fmt::format("Unable to enable breakpoint: {}", platform::get_last_error())};
        }

        // Replace instruction at address with interrupt instruction TODO: Different values for different architectures
        const kstd::u64 instruction = 0xCC;
        _saved_data = static_cast<kstd::u8>(data & 0xFF);
        if(::ptrace(PTRACE_POKEDATA, thread_id, _address, (data & ~0xFF) | instruction) < 0) {
            return kstd::Error {fmt::format("Unable to enable breakpoint: {}", platform::get_last_error())};
        }

        // Set breakpoint enabled
        _enabled = true;
        return {};
    }


    /**
     * This function disables the breakpoint by replacing the inserted instruction at the specified address with the
     * original data saved.
     *
     * @return Void or an error
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto Breakpoint::disable(const ThreadContext& thread_context) noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        const auto thread_id = thread_context.get_thread_id();
        if(!platform::is_process_running(thread_id)) {
            return kstd::Error {"Unable to disable breakpoint: No process is running"s};
        }

        // Read the data at the specified address
        errno = 0;
        const auto data = ::ptrace(PTRACE_PEEKDATA, thread_id, _address, nullptr);
        if(data < 0 && errno != 0) {
            return kstd::Error {fmt::format("Unable to disable breakpoint: {}", platform::get_last_error())};
        }

        // Remove interrupt instruction and insert restored data
        if(::ptrace(PTRACE_POKEDATA, thread_id, _address, (data & ~0xFF) | _saved_data) < 0) {
            return kstd::Error {fmt::format("Unable to disable breakpoint: {}", platform::get_last_error())};
        }

        // Set breakpoint disabled
        _enabled = false;
        return {};
    }

    /**
     * This constructor starts the specified path to the executable with the specified arguments in subprocess
     * and attaches the debugger context to it.
     *
     * @param executable The path to the executable to debug
     * @param arguments  The command-line arguments
     * @author           Cedric Hammes
     * @since            13/03/2024
     */
    ProcessContext::ProcessContext(const std::filesystem::path& executable_path,
                                   const std::vector<std::string>& arguments) ://NOLINT
            _event_callbacks {},
            _breakpoints {},
            _threads {} {
        const auto child_process_id = ::fork();
        if(child_process_id == 0) {
            ::personality(ADDR_NO_RANDOMIZE);
            if(::ptrace(PT_TRACE_ME, 0, nullptr, nullptr) < 0) {
                exit(-1);
            }

            std::ostringstream joined_args {};
            joined_args << executable_path;
            std::copy(arguments.cbegin(), arguments.cend(), std::ostream_iterator<std::string>(joined_args, " "));
            ::execl(executable_path.c_str(), joined_args.str().c_str(), nullptr);
        }
        else if(child_process_id > 0) {
            _process_id = child_process_id;
            _threads.insert(std::pair(_process_id, ThreadContext {_process_id, _process_id}));
        }
        else {
            throw std::runtime_error {fmt::format("Unable to create debugged process: {}", platform::get_last_error())};
        }
    }

    /**
     * This constructor attaches the debugger to the specified process, identified by the specified process
     * id.
     *
     * @param process_id The pid of the target process
     * @author           Cedric Hammes
     * @since            13/03/2024
     */
    ProcessContext::ProcessContext(platform::TaskId process_id) ://NOLINT
            _event_callbacks {},
            _breakpoints {},
            _process_id {process_id},
            _threads {} {
        if(!std::filesystem::exists(fmt::format("/proc/{}", _process_id))) {
            throw std::runtime_error {fmt::format("Failed to attach to process: {} doesn't exists", _process_id)};
        }

        for(const auto& task_dir : std::filesystem::directory_iterator {fmt::format("/proc/{}/task", process_id)}) {
            const auto task_id = std::stoi(task_dir.path().filename().c_str());
            if(::ptrace(PTRACE_ATTACH, task_id, nullptr, nullptr) < 0) {
                throw std::runtime_error {fmt::format("Unable to attach to thread {} of {}: {}", task_id, _process_id,
                                                      platform::get_last_error())};
            }
            _threads.insert(std::pair(task_id, ThreadContext {_process_id, task_id}));
        }
    }

    auto ProcessContext::wait_for_signal() noexcept -> kstd::Result<Signal> {
        using namespace std::chrono;

        while(true) {
            // Enumerate threads
            for(auto& [thread_id, thread_context] : _threads) {
                int status = 0;

                // Wait for signal in single-thread timeout
                const auto begin_timestamp = steady_clock::now();
                while(true) {
                    // Acquire signal
                    if(::waitpid(thread_id, &status, WNOHANG) == -1) {
                        return kstd::Error {fmt::format("Failed signal wait on thread {}: {}", thread_id,
                                                        platform::get_last_error())};
                    }

                    // Handle signal
                    if(status != 0) {
                        return {Signal {&thread_context}};
                    }

                    // Break thread wait because timeout is elapsed // TODO: Configurable timeout
                    if(duration_cast<milliseconds>(steady_clock::now() - begin_timestamp).count() >= 500) {
                        break;
                    }
                }
            }
        }
    }

    /**
     * This function adds a breakpoint at the specified address when no breakpoint was added before
     *
     * @param address The breakpoint address
     * @return        Void or an error
     * @author        Cedric Hammes
     * @since         09/03/2024
     */
    auto ProcessContext::add_breakpoint(std::intptr_t address) noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        if(!is_process_running()) {
            return kstd::Error {"Unable to add breakpoint: No process is running"s};
        }

        if(_breakpoints.contains(address)) {
            return kstd::Error {"Unable to set breakpoint: Breakpoint is already set"s};
        }

        Breakpoint breakpoint {address};
        for(const auto& [_, thread] : _threads) {
            if(const auto enable_result = breakpoint.enable(thread); enable_result.is_error()) {
                return kstd::Error {enable_result.get_error()};
            }
        }
        _breakpoints.insert(std::make_pair(address, breakpoint));
        return {};
    }

    /**
     * This function removes the breakpoint from the specified address when no breakpoint was added before
     *
     * @param address The breakpoint address
     * @return        Void or an error
     * @author        Cedric Hammes
     * @since         09/03/2024
     */
    auto ProcessContext::remove_breakpoint(std::intptr_t address) noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        if(!is_process_running()) {
            return kstd::Error {"Unable to remove breakpoint: No process is running"s};
        }

        const auto breakpoint = _breakpoints.find(address);
        if(breakpoint == _breakpoints.cend()) {
            return kstd::Error {"Unable to set breakpoint: Breakpoint is not set"s};
        }

        for(const auto& [_, thread] : _threads) {
            if(const auto disable_result = breakpoint->second.disable(thread); disable_result.is_error()) {
                return kstd::Error {disable_result.get_error()};
            }
        }
        _breakpoints.erase(address);
        return {};
    }

    /**
     * This function checks whether the process bound with the debug context is still running or has been
     * terminated.
     *
     * @return Whether the process is still running
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto ProcessContext::is_process_running() const noexcept -> kstd::Result<bool> {
        return platform::is_process_running(_process_id);
    }
}// namespace libdebug
#endif