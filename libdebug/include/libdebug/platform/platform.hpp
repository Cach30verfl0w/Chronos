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

#pragma once
#include <kstd/result.hpp>
#ifdef COMPILER_MSVC
#include <intrin.h>
#else
#include <cpuid.h>
#endif

#ifdef PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace libdebug::platform {
    /**
     * This function returns the last thrown error in this program. This is being used to print the error thrown by the
     * System API to the user.
     *
     * @return The last thrown error as a message
     * @author Cedric Hammes
     * @since  09/03/2024
     */
    auto get_last_error() noexcept -> std::string;

    /**
     * This function returns whether the current system uses a FPU. This is being used to configure Capstone for the
     * target system.
     *
     * @return Whether the current system uses a FPU
     * @author Cedric Hammes
     * @since  10/03/2024
     */
    inline auto is_fpu_present() noexcept -> bool {
#if defined(ARCH_X86)
#ifdef COMPILER_MSVC
        int values[4];
        __cpuid(values, 1);
        return (values[3] & 0b1) == 0b1;
#else
        auto eax = 1;
        auto edx = 0;
        auto ecx = 0;
        auto ebx = 0;
        __cpuid(eax, eax, ebx, ecx, edx);
        return (edx & 0b1) == 0b1;
#endif
#else
        return true;
#endif
    }
}// namespace libdebug::platform
