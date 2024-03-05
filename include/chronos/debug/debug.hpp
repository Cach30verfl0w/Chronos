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
 * @since  03/03/2024
 */

#pragma once
#include "chronos/utils.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <kstd/defaults.hpp>
#include <kstd/option.hpp>
#include <kstd/result.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef PLATFORM_LINUX
#include <sys/personality.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace chronos::debug {
#ifdef PLATFORM_LINUX
    using ProcessId = pid_t;
#endif

    class ChronosDebugger final {
        kstd::Option<ProcessId> _running_process_id;

        public:
        ChronosDebugger() noexcept = default;
        ~ChronosDebugger() noexcept = default;
        KSTD_DEFAULT_MOVE_COPY(ChronosDebugger, ChronosDebugger);

        [[nodiscard]] auto run(const std::filesystem::path& file, const std::vector<std::string>& args) noexcept
                -> kstd::Result<void>;

        [[nodiscard]] inline auto is_running() const noexcept -> bool {
            return _running_process_id.has_value();
        }
    };
}// namespace chronos::debug