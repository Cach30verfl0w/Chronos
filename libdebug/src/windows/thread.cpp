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
            _process_id {process_id},
            _thread_id {thread_id} {
        if(::ptrace(PTRACE_ATTACH, thread_id, nullptr, nullptr) < 0) {
            throw std::runtime_error {fmt::format("Unable to attach to thread {} of {}: {}", _thread_id, _process_id,
                                                  platform::get_last_error())};
        }
    }
}// namespace libdebug
#endif