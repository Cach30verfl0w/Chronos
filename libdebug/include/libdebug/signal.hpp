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
#include <utility>

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

    /**
     * This class represents a signal returned by the wait_for_signal function in the debugger. This allows the user of
     * the API to handle signals.
     *
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    class Signal {
        SignalInfo _signal_info;

        public:
        /**
         * This constructor constructs the signal by the platform-dependent signal info.
         *
         * @param signal_info The signal info from the signal
         * @author            Cedric Hammes
         * @since             09/03/2024
         */
        explicit Signal(SignalInfo signal_info) noexcept;

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
