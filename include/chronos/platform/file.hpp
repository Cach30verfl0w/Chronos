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
#include "chronos/platform/platform.hpp"
#include "chronos/utils.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <kstd/bitflags.hpp>
#include <kstd/defaults.hpp>
#include <kstd/result.hpp>
#include <stdexcept>

#if defined(PLATFORM_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#elif defined(PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace chronos::platform {
#if defined(PLATFORM_LINUX)
    using FileHandle = int;
    static inline const FileHandle invalid_file_handle = -1;
#elif defined(PLATFORM_WINDOWS)
    using FileHandle = HANDLE;
    static inline const FileHandle invalid_file_handle = INVALID_HANDLE_VALUE;
#endif

    KSTD_BITFLAGS(u8, FileFlags, READ = 0b001, WRITE = 0b010, EXECUTE = 0b100);

    class File final {
        std::filesystem::path _path;
        FileHandle _file_handle;

        public:
        File(std::filesystem::path file_path, FileFlags flags);
        File(File&& other) noexcept;
        ~File() noexcept;
        KSTD_NO_COPY(File, File);

        [[nodiscard]] auto get_file_size() const noexcept -> kstd::Result<usize>;

        auto operator=(File&& other) noexcept -> File&;
    };
}// namespace chronos::platform
