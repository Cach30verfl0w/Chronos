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
#include "libdebug/platform/platform.hpp"

namespace libdebug::platform {
    /**
     * This method returns the last thrown error in this program. This is being used to print the error thrown by the
     * System API to the user.
     *
     * @return The last thrown error as a message
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto get_last_error() noexcept -> std::string {
        return ::strerror(errno);
    }

    /**
     * This function checks whether the process bound with the debug context is still running or has been
     * terminated.
     *
     * @return Whether the process is still running
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto is_process_running(TaskId task_id) noexcept -> kstd::Result<bool> {
        return ::kill(task_id, 0) != -1 || errno != ESRCH;
    }
}// namespace libdebug::platform
#endif