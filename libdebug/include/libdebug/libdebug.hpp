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

#pragma once
#include "libdebug/platform/platform.hpp"
#include "libdebug/signal.hpp"
#include "libdebug/utils.hpp"
#include <filesystem>
#include <kstd/defaults.hpp>
#include <kstd/option.hpp>
#include <kstd/result.hpp>

#ifdef PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <csignal>
#include <sys/personality.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#endif

namespace libdebug {
#ifdef PLATFORM_LINUX
    using ProcessId = pid_t;
#elif defined(PLATFORM_WINDOWS)
    using ProcessId = DWORD;
#endif
    
    /**
     * This class is representing a single process being debugged by this application. This context can be initialized
     * by starting a subprocess that is being debugged or attach to an existing process.
     *
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    class DebugContext;

    /**
     * This class is representing a single breakpoint on some address. This is used by the debug context to handle
     * breakpoints.
     *
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    class Breakpoint final {
        const DebugContext* _debug_context;
        std::intptr_t _address;
        bool _enabled;
        u8 _saved_data;

        public:
        /**
         * This constructor constructs an empty breakpoint
         *
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        Breakpoint(const DebugContext* debug_context, std::intptr_t target_address) noexcept;
        ~Breakpoint() noexcept = default;
        KSTD_DEFAULT_MOVE_COPY(Breakpoint, Breakpoint);

        /**
         * This function enables the breakpoint by replacing the instruction at the specified address and saving the
         * original data in this class.
         *
         * @return Void or an error
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] auto enable() noexcept -> kstd::Result<void>;

        /**
         * This function disables the breakpoint by replacing the inserted instruction at the specified address with the
         * original data saved.
         *
         * @return Void or an error
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] auto disable() noexcept -> kstd::Result<void>;

        /**
         * This method returns the address to the breakpoint
         *
         * @return The breakpoint address
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] inline auto get_address() const noexcept -> std::intptr_t {
            return _address;
        }

        /**
         * This method returns whether the breakpoint was enabled or not
         *
         * @return Whether the breakpoint was enabled or not
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] inline auto is_enabled() const noexcept -> bool {
            return _enabled;
        }
    };

    /**
     * This class is representing a single process being debugged by this application. This context can be initialized
     * by starting a subprocess that is being debugged or attach to an existing process.
     *
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    class DebugContext final {
        std::unordered_map<std::intptr_t, Breakpoint> _breakpoints;
        ProcessId _process_id;

        public:
        /**
         * This constructor starts the specified path to the executable with the specified arguments in subprocess and
         * attaches the debugger context to it.
         *
         * @param executable The path to the executable to debug
         * @param arguments  The command-line arguments
         * @author           Cedric Hammes
         * @since            09/03/2024
         */
        DebugContext(const std::filesystem::path& executable, const std::vector<std::string>& arguments);
        ~DebugContext() noexcept = default;
        KSTD_DEFAULT_MOVE(DebugContext, DebugContext);
        KSTD_NO_COPY(DebugContext, DebugContext);

        /**
         * This function continues the execution of the program when the program is running.
         *
         * @param await_signal Wait for signal after continue
         * @return             Optional signal after continue or an error
         * @author             Cedric Hammes
         * @since              09/03/2024
         */
        [[nodiscard]] auto continue_execution(bool await_signal) const noexcept -> kstd::Result<kstd::Option<Signal>>;

        /**
         * This function adds a breakpoint at the specified address when no breakpoint was added before
         *
         * @param address The breakpoint address
         * @return        Void or an error
         * @author        Cedric Hammes
         * @since         09/03/2024
         */
        [[nodiscard]] auto add_breakpoint(std::intptr_t address) noexcept -> kstd::Result<void>;

        /**
         * This function removes the breakpoint from the specified address when no breakpoint was added before
         *
         * @param address The breakpoint address
         * @return        Void or an error
         * @author        Cedric Hammes
         * @since         09/03/2024
         */
        [[nodiscard]] auto remove_breakpoint(std::intptr_t address) noexcept -> kstd::Result<void>;

        /**
         * This function waits for the next debug signal by the debug target process. While the signal is being awaited,
         * the thread where this function is invoked is blocked by this function.
         *
         * @return Void or an error
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] auto wait_for_signal() const noexcept -> kstd::Result<Signal>;

        /**
         * This function checks whether the process bound with the debug context is still running or has been
         * terminated.
         *
         * @return Whether the process is still running
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] auto is_process_running() const noexcept -> kstd::Result<bool>;

        /**
         * This method returns a const reference to all registered breakpoints in the debug context
         *
         * @return All active breakpoints
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] inline auto get_breakpoints() const noexcept
                -> const std::unordered_map<std::intptr_t, Breakpoint>& {
            return _breakpoints;
        }

        /**
         * This method returns the process id of the running process.
         *
         * @return The running process id
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] inline auto get_process_id() const noexcept -> ProcessId {
            return _process_id;
        }
    };
}// namespace libdebug
