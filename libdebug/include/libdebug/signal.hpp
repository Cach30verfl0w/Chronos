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
 * @since  14/03/2024
 */

#pragma once
#include "libdebug/thread.hpp"

#ifdef PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <csignal>
#endif

namespace libdebug {
#ifdef PLATFORM_WINDOWS
    using SignalInfo = DEBUG_EVENT;
#else
    using SignalInfo = siginfo_t;
#endif

    class Signal final {
        ThreadContext* _thread_context;
        SignalInfo _signal_info;

    public:
        explicit Signal(ThreadContext* thread_context, SignalInfo signal_info) noexcept ://NOLINT
                _thread_context {thread_context},
                _signal_info {signal_info} {
        }

        /**
         * This method returns a pointer to the context of the signaled thread
         *
         * @return The context of the thread
         * @author Cedric Hammes
         * @since  14/03/2024
         */
        [[nodiscard]] inline auto get_thread() const noexcept -> const ThreadContext* {
            return _thread_context;
        }

        /**
         * This function returns the info of the signal
         *
         * @return The signal's info
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] inline auto get_signal_info() const noexcept -> const SignalInfo& {
            return _signal_info;
        }


        /**
         * This function checks whether the signal is a breakpoint. If yes, the return type is true, otherwise the
         * return type is false.
         *
         * @return Whether the signal is a breakpoint
         * @author Cedric Hammes
         * @since  09/03/2024
         */
        [[nodiscard]] auto is_breakpoint() const noexcept -> bool;
    };
}// namespace libdebug