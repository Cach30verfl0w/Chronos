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
#include "libdebug/thread.hpp"

namespace libdebug {
    ThreadContext::ThreadContext(platform::TaskId process_id, platform::TaskId thread_id) :
            _saved_breakpoint_data {},
            _process_id {process_id},
            _thread_id {thread_id} {
    }

    auto ThreadContext::wait_for_signal() const noexcept -> kstd::Result<void> {
        using namespace std::string_literals;
        if(!platform::is_process_running(_process_id)) {
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
        printf("Error %i %i %i\n", signal_info.si_code, signal_info.si_signo, signal_info.si_errno);
        return {};
    }
}// namespace libdebug
#endif