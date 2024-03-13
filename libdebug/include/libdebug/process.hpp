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

#pragma once
#include "libdebug/platform/platform.hpp"
#include "libdebug/thread.hpp"
#include <filesystem>
#include <kstd/types.hpp>
#include <unordered_map>
#include <vector>

#ifdef PLATFORM_LINUX
#include <csignal>
#include <sys/personality.h>
#include <sys/ptrace.h>
#endif

// TODO: Ausprobieren ob multiple Threads mit einem ptrace gehandelt werden können.

namespace libdebug {
    /**
     * This class is representing a single breakpoint on some address. This is used by the debug context to handle
     * breakpoints.
     *
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    class Breakpoint final {
        std::intptr_t _address;
        bool _enabled;
        kstd::u8 _saved_data;

    public:
        /**
         * This constructor constructs an empty breakpoint
         *
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        Breakpoint(std::intptr_t target_address) noexcept;
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
        [[nodiscard]] auto enable(const ThreadContext& target_thread_context) noexcept -> kstd::Result<void>;

        /**
         * This function disables the breakpoint by replacing the inserted instruction at the specified address with the
         * original data saved.
         *
         * @return Void or an error
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] auto disable(const ThreadContext& target_thread_context) noexcept -> kstd::Result<void>;

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
     * @since  13/03/2024
     */
    class ProcessContext final {
        platform::TaskId _process_id;
        std::unordered_map<std::intptr_t, Breakpoint> _breakpoints;
        std::unordered_map<platform::TaskId, ThreadContext> _threads;

    public:
        /**
         * This constructor starts the specified path to the executable with the specified arguments in subprocess
         * and attaches the debugger context to it.
         *
         * @param executable The path to the executable to debug
         * @param arguments  The command-line arguments
         * @author           Cedric Hammes
         * @since            13/03/2024
         */
        ProcessContext(const std::filesystem::path& executable_path, const std::vector<std::string>& arguments);

        /**
         * This constructor attaches the debugger to the specified process, identified by the specified process
         * id.
         *
         * @param process_id The pid of the target process
         * @author           Cedric Hammes
         * @since            13/03/2024
         */
        explicit ProcessContext(platform::TaskId process_id);

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
         * This function checks whether the process bound with the debug context is still running or has been
         * terminated.
         *
         * @return Whether the process is still running
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        inline auto is_process_running() const noexcept -> kstd::Result<bool>;

        /**
         * This method returns a const reference to all registered breakpoints in the process context
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
         * This method returns a const reference to all registered threads in the process context
         *
         * @return All registered thread
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] inline auto get_threads() const noexcept
                -> const std::unordered_map<platform::TaskId, ThreadContext>& {
            return _threads;
        }

        /**
         * This method returns a const reference to all registered threads in the process context
         *
         * @return All registered thread
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] inline auto get_threads() noexcept -> std::unordered_map<platform::TaskId, ThreadContext>& {
            return _threads;
        }

        /**
         * This method returns the process id of the running process.
         *
         * @return The running process id
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] inline auto get_process_id() const noexcept -> platform::TaskId {
            return _process_id;
        }
    };
}// namespace libdebug