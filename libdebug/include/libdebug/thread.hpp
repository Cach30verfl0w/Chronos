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
#include <chrono>
#include <kstd/defaults.hpp>

#ifdef PLATFORM_LINUX
#include <sys/ptrace.h>
#include <sys/wait.h>
#endif

namespace libdebug {
    class ThreadContext {
        platform::TaskId _process_id;
        platform::TaskId _thread_id;

        friend struct ProcessContext;

    public:
        ThreadContext(platform::TaskId process_id, platform::TaskId thread_id) noexcept ://NOLINT
                _process_id {process_id},
                _thread_id {thread_id} {
        }

        ~ThreadContext() noexcept = default;
        KSTD_DEFAULT_MOVE(ThreadContext, ThreadContext);
        KSTD_NO_COPY(ThreadContext, ThreadContext);

        /**
         * This function returns the id of the process in which this thread/task is
         * running.
         *
         * @return The process id
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        [[nodiscard]] inline auto get_process_id() const noexcept -> platform::TaskId {
            return _process_id;
        }

        /**
         * This function returns a unique id referenced to this
         * thread/task.
         *
         * @return The thread id
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        [[nodiscard]] inline auto get_thread_id() const noexcept -> platform::TaskId {
            return _thread_id;
        }

        /**
         * This function returns whether this thread is the main thread of the parent
         * process.
         *
         * @return Whether this thread is the main thread
         * @author Cedric Hammes
         * @since  13/03/2024
         */
        [[nodiscard]] inline auto is_main_thread() const noexcept -> bool {
            return _process_id == _thread_id;
        }
    };
}// namespace libdebug