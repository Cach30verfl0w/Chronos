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
    auto ChronosDebugger::run(const std::filesystem::path& file, const std::vector<std::string>& args) noexcept
            -> kstd::Result<void> {
        using namespace std::string_literals;
        if(is_running()) {
            return kstd::Error {"Unable to run debugger: Another process is already running"s};
        }

        const auto child_process_id = ::fork();
        if(child_process_id == 0) {
            // Disable ASLR
            //::personality(ADDR_NO_RANDOMIZE);

            // Tell parent process that this process can be debugged
            if(::ptrace(PT_TRACE_ME, 0, nullptr, nullptr) < 0) {
                // TODO: Print error
                exit(-1);
            }

            // Run process
            std::string arguments {};
            arguments.append(file);
            for (const auto& arg : args) {
                arguments.append(" ");
                arguments.append(arg);
            }

            execl(file.c_str(), arguments.c_str(), nullptr);
        }
        _running_process_id = {child_process_id};

        // TODO: Temporary
        int wait_status;
        ::waitpid(child_process_id, &wait_status, 0);
        ::ptrace(PTRACE_CONT, child_process_id, nullptr, nullptr);
        ::waitpid(child_process_id, &wait_status, 0);
        return {};
    }
}// namespace chronos::debug
#endif