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
#include "libdebug/signal.hpp"

namespace libdebug {
    /**
     * This function checks whether the signal is a breakpoint. If yes, the return type is true, otherwise the
     * return type is false.
     *
     * @return Whether the signal is a breakpoint
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto Signal::is_breakpoint() const noexcept -> bool {
        if(_signal_info.si_signo != SIGTRAP)
            return false;

        const auto sig_code = _signal_info.si_code;
        return sig_code == TRAP_BRKPT || sig_code == TRAP_TRACE;
    }
}// namespace libdebug
#endif