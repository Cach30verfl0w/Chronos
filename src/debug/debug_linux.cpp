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

#ifdef PLATFORM_LINUX
#include "chronos/debug/debug.hpp"

namespace chronos::debug {
    ChronosDebugger::ChronosDebugger(const std::string& executable_path, std::vector<std::string>& arguments) {
        const auto subprocess = ::fork();
        if (subprocess == 0) {
            // Disable ASLR
            ::personality(ADDR_NO_RANDOMIZE);

            // Tell kernel that this process can be traced by the parent process
            if (::ptrace(PT_TRACE_ME, 0, nullptr, nullptr) < 0) {
                throw std::runtime_error {fmt::format("Unable to debug '{}': {}", executable_path, strerror(errno))};
            }

            // Execute program
            arguments.insert(arguments.begin(), executable_path);
            ::execl(executable_path.c_str(), executable_path.c_str(), nullptr);
            return;
        }

        _process_id = subprocess;
    }

    ChronosDebugger::~ChronosDebugger() noexcept {
        i32 status = 0;
        ::waitpid(_process_id, &status, WAIT_MYPGRP);
    }
}// namespace chronos::debug
#endif