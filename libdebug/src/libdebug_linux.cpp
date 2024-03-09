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

#ifdef PLATFORM_LINUX
#include "libdebug/libdebug.hpp"

namespace libdebug {
    /**
     * This constructor constructs an empty breakpoint with a reference to the context and the target address of the
     * breakpoint.
     *
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    Breakpoint::Breakpoint(const libdebug::DebugContext* debug_context, std::intptr_t target_address) noexcept ://NOLINT
            _debug_context {debug_context},
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
    auto Breakpoint::enable() noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        if(!_debug_context->is_process_running()) {
            return kstd::Error {"Unable to enable breakpoint: No process is running"s};
        }

        // Read the data at the specified address and save instruction data
        errno = 0;
        const auto data = ::ptrace(PTRACE_PEEKDATA, _debug_context->get_process_id(), _address, nullptr);
        if(data < 0 && errno != 0) {
            return kstd::Error {fmt::format("Unable to enable breakpoint: {}", platform::get_last_error())};
        }
        _saved_data = static_cast<u8>(data & 0xFF);

        // Replace instruction at address with interrupt instruction
        const u64 software_interrupt_instruction = 0xCC;
        if(::ptrace(PTRACE_POKEDATA, _debug_context->get_process_id(), _address,
                    (data & ~0xFF) | software_interrupt_instruction) < 0) {
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
    auto Breakpoint::disable() noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        if(!_debug_context->is_process_running()) {
            return kstd::Error {"Unable to disable breakpoint: No process is running"s};
        }

        // Read the data at the specified address
        errno = 0;
        const auto data = ::ptrace(PTRACE_PEEKDATA, _debug_context->get_process_id(), _address, nullptr);
        if(data < 0 && errno != 0) {
            return kstd::Error {fmt::format("Unable to disable breakpoint: {}", platform::get_last_error())};
        }

        // Remove interrupt instruction and insert restored data
        if(::ptrace(PTRACE_POKEDATA, _debug_context->get_process_id(), _address, (data & ~0xFF) | _saved_data) < 0) {
            return kstd::Error {fmt::format("Unable to disable breakpoint: {}", platform::get_last_error())};
        }

        // Set breakpoint disabled
        _enabled = false;
        return {};
    }

    /**
     * This constructor starts the specified path to the executable with the specified arguments in subprocess and
     * attaches the debugger context to it.
     *
     * @param executable The path to the executable to debug
     * @param arguments  The command-line arguments
     * @author           Cedric Hammes
     * @since            09/03/2024
     */
    DebugContext::DebugContext(const std::filesystem::path& executable,
                               const std::vector<std::string>& arguments) ://NOLINT
            _breakpoints {} {
        const auto child_process_id = ::fork();
        if(child_process_id == 0) {
            ::personality(ADDR_NO_RANDOMIZE);
            if(::ptrace(PT_TRACE_ME, 0, nullptr, nullptr) < 0) {
                exit(-1);
            }

            std::ostringstream joined_args {};
            joined_args << executable;
            std::copy(arguments.cbegin(), arguments.cend(), std::ostream_iterator<std::string>(joined_args, " "));
            ::printf("%s\n", joined_args.str().c_str());
            ::execl(executable.c_str(), joined_args.str().c_str(), nullptr);
        }
        else if(child_process_id > 0) {
            _process_id = child_process_id;
        }
        else {
            throw std::runtime_error {fmt::format("Unable to create debugged process: {}", platform::get_last_error())};
        }
    }

    /**
     * This function continues the execution of the program when the program is running.
     *
     * @param await_signal Wait for signal after continue
     * @return             Void or an error
     * @author             Cedric Hammes
     * @since              09/03/2024
     */
    auto DebugContext::continue_execution(bool await_signal) const noexcept -> kstd::Result<kstd::Option<Signal>> {
        using namespace std::string_literals;
        if(!is_process_running()) {
            return kstd::Error {"Unable to wait for signal: The process is not running"s};
        }
        
        // TODO: Jump before breakpoint, remove breakpoint and then resume execution when instruction counter is set
        //  to some breakpoint

        if(::ptrace(PTRACE_CONT, _process_id, nullptr, nullptr) < 0) {
            return kstd::Error {fmt::format("Unable to wait for signal: {}", platform::get_last_error())};
        }

        // Await signal when enabled
        if(await_signal) {
            const auto result = wait_for_signal();
            if(result.is_error()) {
                return kstd::Error {result.get_error()};
            }

            return {{*result}};
        }
        return {};
    }

    /**
     * This function adds a breakpoint at the specified address when no breakpoint was added before
     *
     * @param address The breakpoint address
     * @return        Void or an error
     * @author        Cedric Hammes
     * @since         09/03/2024
     */
    auto DebugContext::add_breakpoint(std::intptr_t address) noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        if(!is_process_running()) {
            return kstd::Error {"Unable to add breakpoint: No process is running"s};
        }

        if(_breakpoints.contains(address)) {
            return kstd::Error {"Unable to set breakpoint: Breakpoint is already set"s};
        }

        // Create and set breakpoint
        Breakpoint breakpoint {this, address};
        if(const auto enable_result = breakpoint.enable(); enable_result.is_error()) {
            return kstd::Error {enable_result.get_error()};
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
    auto DebugContext::remove_breakpoint(std::intptr_t address) noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        if(!is_process_running()) {
            return kstd::Error {"Unable to remove breakpoint: No process is running"s};
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

    /**
     * This function waits for the next debug signal by the debug target process. While the signal is being awaited,
     * the thread where this function is invoked is blocked by this function.
     *
     * @return Void or an error
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto DebugContext::wait_for_signal() const noexcept -> kstd::Result<Signal> {
        using namespace std::string_literals;
        if(!is_process_running()) {
            return kstd::Error {"Unable to wait for signal: No process is running"s};
        }

        // Wait for signal and acquire info
        int wait_status;
        if(::waitpid(_process_id, &wait_status, 0) < 0) {
            return kstd::Error {fmt::format("Unable to wait for signal: {}", platform::get_last_error())};
        }

        siginfo_t signal_info {};
        if(::ptrace(PTRACE_GETSIGINFO, _process_id, nullptr, &signal_info) < 0) {
            return kstd::Error {fmt::format("Unable to wait for signal: {}", platform::get_last_error())};
        }

        // Return signal built by info
        return {Signal {signal_info}};
    }

    /**
     * This function checks whether the process bound with the debug context is still running or has been
     * terminated.
     *
     * @return Whether the process is still running
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto DebugContext::is_process_running() const noexcept -> kstd::Result<bool> {
        return ::kill(_process_id, 0) != -1 || errno != ESRCH;
    }
}// namespace libdebug
#endif