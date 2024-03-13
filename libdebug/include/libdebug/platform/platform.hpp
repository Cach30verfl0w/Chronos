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
#include <kstd/result.hpp>

#ifdef PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <unistd.h>
#include <csignal>
#endif

namespace libdebug::platform {
#ifdef PLATFORM_WINDOWS
    using FileHandle = HANDLE;
    using TaskId = DWORD;
#else
    using FileHandle = int;
    using TaskId = pid_t;
#endif

    /**
     * This function returns the last thrown error in this program. This is being used to print the error thrown by the
     * System API to the user.
     *
     * @return The last thrown error as a message
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto get_last_error() noexcept -> std::string;

    /**
     * This function checks whether the process bound with the debug context is still running or has been
     * terminated.
     *
     * @return Whether the process is still running
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto is_process_running(TaskId task_id) noexcept -> kstd::Result<bool>;
}// namespace libdebug::platform