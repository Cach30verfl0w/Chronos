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

#ifdef PLATFORM_WINDOWS
#include "libdebug/platform/platform.hpp"
#include <kstd/utils.hpp>

namespace chronos::platform {
    /**
     * This method returns the last thrown error in this program. This is being used to print the error thrown by the
     * System API to the user.
     *
     * @return The last thrown error as a message
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto get_last_error() noexcept -> std::string {
        const auto last_error_code = ::GetLastError();
        if(last_error_code == NO_ERROR) {
            return "No error occurred";
        }

        LPWSTR error_buffer = nullptr;
        constexpr auto language_id = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
        const auto new_length = ::FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                last_error_code, language_id, reinterpret_cast<LPWSTR>(&error_buffer), 0, nullptr);
        const auto message = kstd::utils::to_mbs({error_buffer, new_length});
        LocalFree(error_buffer);
        return message;
    }
}// namespace chronos::platform
#endif
