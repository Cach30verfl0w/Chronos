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
#include "libdebug/platform/platform.hpp"
#include "libdebug/utils.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <kstd/bitflags.hpp>
#include <kstd/defaults.hpp>
#include <kstd/result.hpp>
#include <stdexcept>

#if defined(PLATFORM_LINUX)
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#elif defined(PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace libdebug::platform {
    namespace {
        template<typename T, T... FLAGS>
        constexpr auto are_flags_set(const T value) noexcept -> bool {
            return (value & (FLAGS | ...)) != T::NONE;
        }
    }// namespace

#if defined(PLATFORM_LINUX)
    using FileHandle = int;
    static inline const FileHandle invalid_file_handle = -1;
#elif defined(PLATFORM_WINDOWS)
    using FileHandle = HANDLE;
    static inline const FileHandle invalid_file_handle = INVALID_HANDLE_VALUE;
#endif

    KSTD_BITFLAGS(u8, FileFlags, READ = 0b001, WRITE = 0b010, EXECUTE = 0b100);

    class FileMapping final {
#ifdef PLATFORM_WINDOWS
        HANDLE _memory_map_handle;
#endif
        u8* _pointer;
        usize _size;

        friend struct File;
#ifdef PLATFORM_WINDOWS
        FileMapping(u8* file_ptr, HANDLE memory_map_handle,  usize size) noexcept;
#else
        FileMapping(u8* file_ptr, usize size) noexcept;
#endif

        public:
        FileMapping(FileMapping&& other) noexcept;
        ~FileMapping() noexcept;
        KSTD_NO_COPY(FileMapping, FileMapping);

        [[nodiscard]] inline auto get_size() const noexcept -> usize {
            return _size;
        }

        auto operator=(FileMapping&& other) noexcept -> FileMapping&;
        [[nodiscard]] auto operator*() const noexcept -> const u8*;
    };

    class File final {
        std::filesystem::path _path;
        FileHandle _file_handle;
        FileFlags _flags;

        public:
        File(std::filesystem::path file_path, FileFlags flags);
        File(File&& other) noexcept;
        ~File() noexcept;
        KSTD_NO_COPY(File, File);

        [[nodiscard]] auto map_into_memory() const noexcept -> kstd::Result<FileMapping>;
        [[nodiscard]] auto get_file_size() const noexcept -> kstd::Result<usize>;

        auto operator=(File&& other) noexcept -> File&;
    };
}// namespace chronos::platform